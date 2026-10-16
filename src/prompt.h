#pragma once
// prompt: what we ask Claude each day.
//
// To change the website's "personality", this is the file to edit.

#include <string>

// Today's date as YYYY-MM-DD in the machine's local time zone
// (the daily job sets TZ=America/New_York).
std::string todaysDate();

// The full question sent to Claude, including today's date. Claude doesn't
// know the date unless we tell it.
std::string buildPrompt(const std::string& date);
