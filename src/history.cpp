#include "history.h"

#include <fstream>
#include <stdexcept>

using nlohmann::json;

json loadPicks(const std::string& path) {
    std::ifstream file(path);
    if (!file) return json::array();  // first run: no history yet
    return json::parse(file).value("picks", json::array());
}

void savePicks(const std::string& path, const json& picks, const json& record) {
    std::ofstream file(path);
    if (!file) throw std::runtime_error("Could not open " + path + " for writing");
    const json everything = {{"record", record}, {"picks", picks}};
    file << everything.dump(2) << '\n';
}
