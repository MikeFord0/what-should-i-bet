#include "claude_client.h"

#include <curl/curl.h>
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

}  // namespace

json askClaude(const ClaudeRequest& request) {
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

    // Usually one request is enough. If Claude is still busy searching after a
    // long turn, the API stops early with stop_reason "pause_turn". We then send
    // its partial reply back so it can pick up where it left off.
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
