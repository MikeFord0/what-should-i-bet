#include "pick.h"

#include <fstream>
#include <set>
#include <stdexcept>

using nlohmann::json;

namespace {

const char* kRequiredFields[] = {"sport", "event", "start_time", "bet",
                                 "odds",  "confidence", "reasoning"};

// Claude's reply is a list of blocks, for example:
//   text                   "I'll search for today's games..."
//   server_tool_use        the search query Claude ran
//   web_search_tool_result the pages it found
//   text (with citations)  the final answer (often split into several blocks)
// The answer is the text that comes after the last search result.
size_t indexAfterLastSearch(const json& content) {
    size_t start = 0;
    for (size_t i = 0; i < content.size(); ++i) {
        if (content[i].value("type", "") == "web_search_tool_result") start = i + 1;
    }
    return start;
}

}  // namespace

json extractPick(const json& content) {
    std::string answerText;
    json sources = json::array();
    std::set<std::string> seenUrls;

    for (size_t i = indexAfterLastSearch(content); i < content.size(); ++i) {
        const json& block = content[i];
        if (block.value("type", "") != "text") continue;

        answerText += block.value("text", "");

        // Citations: the pages Claude based this text on. We pass them on to the
        // website (the API terms ask you to show them when you display answers).
        for (const json& cite : block.value("citations", json::array())) {
            const std::string url = cite.value("url", "");
            if (!url.empty() && seenUrls.insert(url).second) {
                sources.push_back({{"title", cite.value("title", url)}, {"url", url}});
            }
        }
    }

    // Cut out the {...} part, in case Claude added a stray sentence around it.
    const size_t open = answerText.find('{');
    const size_t close = answerText.rfind('}');
    if (open == std::string::npos || close == std::string::npos || close < open) {
        throw std::runtime_error("No JSON found in Claude's answer:\n" + answerText);
    }

    json pick;
    try {
        pick = json::parse(answerText.substr(open, close - open + 1));
    } catch (const json::parse_error& e) {
        throw std::runtime_error(std::string("Claude's answer was not valid JSON: ") + e.what());
    }

    for (const char* field : kRequiredFields) {
        if (!pick.contains(field) || !pick[field].is_string()) {
            throw std::runtime_error(std::string("Claude's answer is missing \"") + field + "\"");
        }
    }

    pick["sources"] = sources;
    return pick;
}

void writePickFile(const json& pick, const std::string& path) {
    std::ofstream file(path);
    if (!file) throw std::runtime_error("Could not open " + path + " for writing");
    file << pick.dump(2) << '\n';
}
