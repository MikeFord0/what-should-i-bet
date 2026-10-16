// daily_pick: asks Claude for today's best bet and saves it for the website.
//
// Usage:   daily_pick [output-path]          (default: site/pick.json)
// Needs:   ANTHROPIC_API_KEY environment variable
// Optional: CLAUDE_MODEL environment variable (default: claude-sonnet-5-5)

#include <cstdlib>
#include <iostream>
#include <string>

#include <curl/curl.h>

#include "claude_client.h"
#include "pick.h"
#include "prompt.h"

namespace {

std::string envOr(const char* name, const std::string& fallback) {
    const char* value = std::getenv(name);
    return (value && *value) ? value : fallback;
}

}  // namespace

int main(int argc, char* argv[]) {
    const std::string apiKey = envOr("ANTHROPIC_API_KEY", "");
    if (apiKey.empty()) {
        std::cerr << "Set the ANTHROPIC_API_KEY environment variable first.\n";
        return 1;
    }
    const std::string outputPath = argc > 1 ? argv[1] : "site/pick.json";

    ClaudeRequest request;
    request.apiKey = apiKey;
    request.model = envOr("CLAUDE_MODEL", "claude-sonnet-5-5");
    const std::string date = todaysDate();
    request.prompt = buildPrompt(date);

    curl_global_init(CURL_GLOBAL_DEFAULT);
    int exitCode = 0;
    try {
        std::cout << "Asking " << request.model << " for the pick of " << date << "...\n";

        const nlohmann::json reply = askClaude(request);  // 1. ask
        nlohmann::json pick = extractPick(reply);         // 2. understand the answer
        pick["date"] = date;
        pick["model"] = request.model;
        writePickFile(pick, outputPath);                  // 3. save it for the site

        std::cout << pick.dump(2) << "\nSaved to " << outputPath << '\n';
    } catch (const std::exception& error) {
        // On failure we leave yesterday's pick.json untouched, so the site keeps working.
        std::cerr << "Error: " << error.what() << '\n';
        exitCode = 1;
    }
    curl_global_cleanup();
    return exitCode;
}
