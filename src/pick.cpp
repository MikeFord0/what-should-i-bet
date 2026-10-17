#include "pick.h"

#include <stdexcept>

using nlohmann::json;

namespace {

const char* kPickFields[] = {"sport", "event", "start_time", "bet",
                             "odds",  "confidence", "reasoning"};

std::string requireString(const json& data, const char* field) {
    if (!data.contains(field) || !data[field].is_string()) {
        throw std::runtime_error(std::string("Claude's answer is missing \"") + field + "\"");
    }
    return data[field].get<std::string>();
}

}  // namespace

json makePick(const ClaudeAnswer& answer, const std::string& date) {
    json pick;
    pick["date"] = date;
    for (const char* field : kPickFields) {
        pick[field] = requireString(answer.data, field);
    }
    pick["sources"] = answer.sources;
    pick["result"] = "pending";  // graded on a later morning
    return pick;
}

void applyGrade(json& pick, const ClaudeAnswer& answer) {
    const std::string result = requireString(answer.data, "result");
    if (result != "win" && result != "loss" && result != "push" && result != "pending") {
        throw std::runtime_error("Unexpected result \"" + result + "\"");
    }
    pick["result"] = result;
    if (answer.data.contains("final_score") && answer.data["final_score"].is_string()) {
        pick["final_score"] = answer.data["final_score"];
    }
}
