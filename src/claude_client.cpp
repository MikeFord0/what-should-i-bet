#include "claude_client.h"

#include <curl/curl.h>
#include <set>
#include <stdexcept>

using nlohmann::json;

namespace {

const char* kApiUrl = "https://api.anthropic.com/v1/messages";

// libcurl hands us the response in chunks; this appends each chunk to a string.
size_t appendToString(char* data, size_t size, size_t count, void* userData) {
    auto* out = static_cast<std::string*>(userData);
    out->append(data, size * count);
    return size * count;
}

// One HTTPS POST to the Messages API. Returns the parsed JSON reply.
json postToApi(const std::string& apiKey, const json& body) {
    CURL* curl = curl_easy_init();
    if (!curl) throw std::runtime_error("Could not start libcurl");

    const std::string payload = body.dump();
    const std::string keyHeader = "x-api-key: " + apiKey;
    std::string response;

    // The three headers every Claude API request needs.
    curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "content-type: application/json");
    headers = curl_slist_append(headers, keyHeader.c_str());
    headers = curl_slist_append(headers, "anthropic-version: 2023-06-01");

    curl_easy_setopt(curl, CURLOPT_URL, kApiUrl);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, appendToString);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 300L);  // searching can take a while

    const CURLcode result = curl_easy_perform(curl);
    long httpStatus = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpStatus);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (result != CURLE_OK) {
        throw std::runtime_error(std::string("Network error: ") + curl_easy_strerror(result));
    }
    if (httpStatus != 200) {
        // The API explains what went wrong in the body (bad key, bad model name, ...).
        throw std::runtime_error("Claude API returned HTTP " + std::to_string(httpStatus) +
                                 ": " + response);
    }
    return json::parse(response);
}

// Sends the conversation, continuing if the API pauses a long search turn.
// Returns the "content" blocks of Claude's final reply.
json sendWithWebSearch(const ClaudeRequest& request) {
    // A conversation is a list of messages. We start with one: our question.
    json messages = json::array();
    messages.push_back({{"role", "user"}, {"content", request.prompt}});

    // The web search tool runs on Anthropic's servers, so we only have to
    // switch it on. Claude decides what to search for.
    const json tools = json::array({{
        {"type", "web_search_20250305"},
        {"name", "web_search"},
        {"max_uses", request.maxSearches},
    }});

    // If Claude is still busy searching after a long turn, the API stops early
    // with stop_reason "pause_turn". We send its partial reply back so it can
    // pick up where it left off.
    for (int attempt = 0; attempt < 5; ++attempt) {
        const json body = {
            {"model", request.model},
            {"max_tokens", request.maxTokens},
            {"messages", messages},
            {"tools", tools},
        };
        const json reply = postToApi(request.apiKey, body);

        if (reply.value("stop_reason", "") != "pause_turn") {
            return reply.at("content");
        }
        messages.push_back({{"role", "assistant"}, {"content", reply.at("content")}});
    }
    throw std::runtime_error("Claude did not finish after several continuations");
}

// Claude's reply is a list of blocks, for example:
//   text                   "I'll search for today's games..."
//   server_tool_use        the search query Claude ran
//   web_search_tool_result the pages it found
//   text (with citations)  the final answer (often split into several blocks)
// The answer is the text that comes after the last search result.
ClaudeAnswer readAnswer(const json& content) {
    size_t start = 0;
    for (size_t i = 0; i < content.size(); ++i) {
        if (content[i].value("type", "") == "web_search_tool_result") start = i + 1;
    }

    std::string text;
    ClaudeAnswer answer;
    answer.sources = json::array();
    std::set<std::string> seenUrls;

    for (size_t i = start; i < content.size(); ++i) {
        const json& block = content[i];
        if (block.value("type", "") != "text") continue;
        text += block.value("text", "");

        // Citations: the pages Claude based this text on.
        const json citations = block.value("citations", json::array());
        for (const json& cite : citations) {
            const std::string url = cite.value("url", "");
            if (!url.empty() && seenUrls.insert(url).second) {
                answer.sources.push_back({{"title", cite.value("title", url)}, {"url", url}});
            }
        }
    }

    // Cut out the {...} part, in case Claude added a stray sentence around it.
    const size_t open = text.find('{');
    const size_t close = text.rfind('}');
    if (open == std::string::npos || close == std::string::npos || close < open) {
        throw std::runtime_error("No JSON found in Claude's answer:\n" + text);
    }
    try {
        answer.data = json::parse(text.substr(open, close - open + 1));
    } catch (const json::parse_error& e) {
        throw std::runtime_error(std::string("Claude's answer was not valid JSON: ") + e.what());
    }
    return answer;
}

}  // namespace

ClaudeAnswer askClaude(const ClaudeRequest& request) {
    return readAnswer(sendWithWebSearch(request));
}
