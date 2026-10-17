#pragma once
// record: the betting math. Win/loss record and profit.
// Pure functions with no network or files, so they're easy to test.

#include <string>
#include <nlohmann/json.hpp>

// Money won or lost on one bet of `stake` dollars.
//   American odds -110: risk $110 to win $100  ->  a $10 win pays $9.09
//   American odds +145: risk $100 to win $145  ->  a $10 win pays $14.50
//   loss: -stake   push or pending: 0
double profitFor(const std::string& odds, const std::string& result, double stake);

// Summarizes a list of picks (newest first) into:
// { "stake": 10,
//   "all_time": {"wins", "losses", "pushes", "profit"},
//   "last_10":  {...same, for the 10 most recent graded picks} }
nlohmann::json summarize(const nlohmann::json& picks, double stake);
