#pragma once
// prompt: what we ask Claude.
//
// To change the website's "personality", this is the file to edit.

#include <string>
#include <nlohmann/json.hpp>

// Today's date as YYYY-MM-DD in the machine's local time zone
// (the daily job sets TZ=America/New_York).
std::string todaysDate();

// Asks for today's best bet. Claude doesn't know the date unless we tell it.
std::string buildPickPrompt(const std::string& today);

// Asks whether an earlier pick won, lost or pushed.
std::string buildGradePrompt(const nlohmann::json& pick, const std::string& today);
