#pragma once
// pick: turns Claude's reply into the pick.json file the website reads.

#include <string>
#include <nlohmann/json.hpp>

// Finds Claude's JSON answer inside its reply blocks, checks every expected
// field is present, and attaches the web pages Claude cited as "sources".
// Throws std::runtime_error if the answer is missing or malformed.
nlohmann::json extractPick(const nlohmann::json& replyContent);

// Writes the pick to disk as nicely indented JSON.
void writePickFile(const nlohmann::json& pick, const std::string& path);
