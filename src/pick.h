#pragma once
// pick: checks Claude's answers have the shape we asked for.

#include <string>
#include <nlohmann/json.hpp>

#include "claude_client.h"

// Builds a new pick from Claude's answer: checks every field is present, then
// adds the date, the sources and result "pending".
nlohmann::json makePick(const ClaudeAnswer& answer, const std::string& date);

// Applies a grading answer to a pick: sets "result" (win/loss/push/pending)
// and "final_score". Throws if the answer doesn't make sense.
void applyGrade(nlohmann::json& pick, const ClaudeAnswer& answer);
