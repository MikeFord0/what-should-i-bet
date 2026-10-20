#include "prompt.h"

#include <ctime>

std::string todaysDate() {
    const std::time_t now = std::time(nullptr);
    char buffer[11];  // "YYYY-MM-DD" plus the terminating '\0'
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d", std::localtime(&now));
    return buffer;
}

// Both prompts ask for JSON so the C++ code can read the answer reliably,
// not free-form prose that would have to be picked apart.

std::string buildPickPrompt(const std::string& today) {
    return "Today is " + today + R"(.

You are the analyst behind whatshouldibet.com, a site that shows one sports bet per day.

1. Search for today's games and current betting lines (moneylines, spreads, totals)
   across the major sports. One broad search for the day's odds is usually enough.
2. Narrow down to the most promising game, then search for news on it:
   injuries, lineups, form, weather, rest days.
3. Choose the single bet you think offers the best value today. Be honest about
   uncertainty. No bet is a sure thing.

You have a small search budget, so make each search targeted and don't repeat one.

Reply with ONLY a JSON object, with no other text before or after it, in exactly this shape:
{
  "sport": "e.g. NFL, NBA, Premier League",
  "event": "Away Team at Home Team",
  "start_time": "local start time with time zone, e.g. 7:30 PM ET",
  "bet": "the bet itself, e.g. Lakers -3.5 or Over 47.5 points",
  "odds": "American odds, e.g. -110 or +145",
  "confidence": "low, medium, or high",
  "reasoning": "2-4 plain-English sentences explaining why"
})";
}

std::string buildGradePrompt(const nlohmann::json& pick, const std::string& today) {
    return "Today is " + today + ". On " + pick.value("date", "") +
           " this sports bet was recommended:\n"
           "  Bet:   " + pick.value("bet", "") + "\n"
           "  Event: " + pick.value("event", "") + " (" + pick.value("sport", "") + ")\n"
           "  Odds:  " + pick.value("odds", "") + "\n" + R"(
Use web search to find the final score (one search is usually enough) and decide how the bet did.
- "win" or "loss" as usual.
- "push" if the bet is refunded (e.g. the margin lands exactly on the spread).
- "pending" only if the game has not finished yet or was postponed.

Reply with ONLY a JSON object, with no other text before or after it:
{
  "result": "win, loss, push, or pending",
  "final_score": "e.g. Seahawks 24, Broncos 20"
})";
}
