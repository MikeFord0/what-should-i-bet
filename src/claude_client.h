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

// Sends the prompt to Claude with the web search tool turned on.
// Returns the "content" array of Claude's final reply: a list of blocks
// (text, search queries, search results, ...).
// Throws std::runtime_error if the network or the API fails.
nlohmann::json askClaude(const ClaudeRequest& request);
