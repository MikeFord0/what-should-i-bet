#include "record.h"

#include <cmath>
#include <cstdlib>

using nlohmann::json;

namespace {

// "+145" -> 145, "-110" -> -110. Anything unreadable counts as -110,
// the standard price for spreads and totals.
int parseOdds(const std::string& odds) {
    const int value = std::atoi(odds.c_str());  // atoi understands "+145" and "-110"
    return (value >= 100 || value <= -100) ? value : -110;
}

double roundToCents(double dollars) {
    return std::round(dollars * 100.0) / 100.0;
}

// Adds up a range of picks. Pending picks are skipped.
json tally(const json& picks, size_t maxGraded, double stake) {
    int wins = 0, losses = 0, pushes = 0;
    double profit = 0.0;
    size_t graded = 0;

    for (const json& pick : picks) {
        if (graded == maxGraded) break;
        const std::string result = pick.value("result", "pending");
        if (result == "pending") continue;

        ++graded;
        if (result == "win") ++wins;
        if (result == "loss") ++losses;
        if (result == "push") ++pushes;
        profit += profitFor(pick.value("odds", ""), result, stake);
    }
    return {{"wins", wins}, {"losses", losses}, {"pushes", pushes},
            {"profit", roundToCents(profit)}};
}

}  // namespace

double profitFor(const std::string& odds, const std::string& result, double stake) {
    if (result == "loss") return -stake;
    if (result != "win") return 0.0;  // push or pending

    const int price = parseOdds(odds);
    return price > 0 ? stake * price / 100.0      // underdog: win more than you risk
                     : stake * 100.0 / -price;   // favorite: win less than you risk
}

json summarize(const json& picks, double stake) {
    return {
        {"stake", stake},
        {"all_time", tally(picks, picks.size(), stake)},
        {"last_10", tally(picks, 10, stake)},
    };
}
