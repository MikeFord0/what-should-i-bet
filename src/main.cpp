// daily_pick: the morning job for whatshouldibet.com.
//   1. grade  - check how earlier pending picks turned out
//   2. pick   - ask for today's best bet
//   3. tally  - recompute the record and $10-per-bet profit
//   4. save   - write everything to picks.json for the website
//
// Usage:    daily_pick [path]                (default: site/picks.json)
// Needs:    ANTHROPIC_API_KEY environment variable
// Optional: CLAUDE_MODEL       model that picks  (default: claude-sonnet-5-5)
//           CLAUDE_GRADE_MODEL model that grades (default: claude-haiku-5-5)

#include <cstdlib>
#include <iostream>
#include <string>

#include <curl/curl.h>

#include "claude_client.h"
#include "history.h"
#include "pick.h"
#include "prompt.h"
#include "record.h"

namespace {

const double kStake = 10.0;  // pretend we bet $10 on every pick

// Search limits. Each search costs $0.01, and every page it returns adds input
// tokens, so these are the main cost levers.
const int kPickSearches = 4;   // schedule + odds, then news on the chosen game
const int kGradeSearches = 2;  // a final score is a simple lookup

// Prints what a question cost, so the GitHub Actions log shows where money goes.
void printUsage(const ClaudeUsage& usage, const std::string& model) {
    std::cout << "  [" << model << ": " << usage.inputTokens << " input tokens, "
              << usage.outputTokens << " output tokens, " << usage.searches << " searches]\n";
}

std::string envOr(const char* name, const std::string& fallback) {
    const char* value = std::getenv(name);
    return (value && *value) ? value : fallback;
}

// Step 1. A failed grade isn't fatal: the pick stays pending and we try again tomorrow.
void gradePendingPicks(nlohmann::json& picks, ClaudeRequest request, const std::string& today) {
    for (nlohmann::json& pick : picks) {
        const std::string date = pick.value("date", "");
        if (pick.value("result", "pending") != "pending" || date >= today) continue;

        std::cout << "Grading " << date << ": " << pick.value("bet", "") << "... ";
        try {
            request.prompt = buildGradePrompt(pick, today);
            const ClaudeAnswer answer = askClaude(request);
            applyGrade(pick, answer);
            std::cout << pick["result"].get<std::string>() << '\n';
            printUsage(answer.usage, request.model);
        } catch (const std::exception& error) {
            std::cout << "could not grade (" << error.what() << ")\n";
        }
    }
}

// Step 2. Puts today's pick at the front of the list. If the job is run twice
// in one day, the second pick replaces the first.
void addTodaysPick(nlohmann::json& picks, ClaudeRequest request, const std::string& today) {
    std::cout << "Asking for the pick of " << today << "...\n";
    request.prompt = buildPickPrompt(today);
    const ClaudeAnswer answer = askClaude(request);
    const nlohmann::json pick = makePick(answer, today);
    printUsage(answer.usage, request.model);

    if (!picks.empty() && picks[0].value("date", "") == today) picks.erase(0);
    picks.insert(picks.begin(), pick);
    std::cout << pick.dump(2) << '\n';
}

}  // namespace

int main(int argc, char* argv[]) {
    ClaudeRequest request;
    request.apiKey = envOr("ANTHROPIC_API_KEY", "");
    request.model = envOr("CLAUDE_MODEL", "claude-sonnet-5-5");
    if (request.apiKey.empty()) {
        std::cerr << "Set the ANTHROPIC_API_KEY environment variable first.\n";
        return 1;
    }
    const std::string path = argc > 1 ? argv[1] : "site/picks.json";
    const std::string today = todaysDate();

    curl_global_init(CURL_GLOBAL_DEFAULT);
    int exitCode = 0;
    try {
        nlohmann::json picks = loadPicks(path);

        // Grading is an easy lookup, so it uses a cheaper model and fewer searches.
        ClaudeRequest gradeRequest = request;
        gradeRequest.model = envOr("CLAUDE_GRADE_MODEL", "claude-haiku-5-5");
        gradeRequest.maxSearches = kGradeSearches;
        gradeRequest.maxTokens = 1024;
        gradePendingPicks(picks, gradeRequest, today);

        ClaudeRequest pickRequest = request;
        pickRequest.maxSearches = kPickSearches;
        try {
            addTodaysPick(picks, pickRequest, today);
        } catch (const std::exception& error) {
            // Still save the grades below; the site keeps showing the last pick.
            std::cerr << "Could not get today's pick: " << error.what() << '\n';
            exitCode = 1;
        }

        const nlohmann::json record = summarize(picks, kStake);
        savePicks(path, picks, record);
        std::cout << "Record: " << record.dump() << "\nSaved to " << path << '\n';
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        exitCode = 1;
    }
    curl_global_cleanup();
    return exitCode;
}
