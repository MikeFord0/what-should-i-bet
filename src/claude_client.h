#pragma once
// claude_client: talks to the Claude Messages API over HTTPS.
//
// This module knows about HTTP, headers and the API's JSON shape, and nothing
// about sports or betting. You could reuse it for any other Claude project.

#include <string>
#include <nlohmann/json.hpp>

struct ClaudeRequest {
    std::string apiKey;      // from the ANTHROPIC_API_KEY environment variable
    std::string model;       // e.g. "claude-sonnet-5-5"
    std::string prompt;      // the question we ask
    int maxTokens = 4096;    // upper limit on how long Claude's reply can be
    int maxSearches = 5;     // upper limit on web searches (each costs $0.01)
};

// What one question cost. Tokens are roughly "pieces of words"; every web
// page Claude reads counts as input tokens, which is where most cost comes from.
struct ClaudeUsage {
    int inputTokens = 0;
    int outputTokens = 0;
    int searches = 0;        // $0.01 each
};

struct ClaudeAnswer {
    nlohmann::json data;     // the JSON object Claude answered with
    nlohmann::json sources;  // [{title, url}] of web pages Claude cited
    ClaudeUsage usage;       // printed in the GitHub Actions log
};

// Sends the prompt to Claude with the web search tool turned on, and reads
// the JSON object out of its final answer. The prompt must ask for JSON.
// Throws std::runtime_error if the network, the API or the answer is bad.
ClaudeAnswer askClaude(const ClaudeRequest& request);
