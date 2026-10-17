#pragma once
// history: reads and writes site/picks.json, the site's whole "database".
//
// File shape:
// { "record": { ...totals from record.h... },
//   "picks":  [ newest pick first, ..., oldest pick last ] }

#include <string>
#include <nlohmann/json.hpp>

// Returns the list of picks, or an empty list if the file doesn't exist yet.
nlohmann::json loadPicks(const std::string& path);

// Writes the picks and their totals back to disk.
void savePicks(const std::string& path, const nlohmann::json& picks,
               const nlohmann::json& record);
