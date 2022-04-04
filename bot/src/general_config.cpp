#include <general_config.hpp>

bool general_config::load_from(const std::string& s)
{
    const auto buf = dpp::utility::read_file(s);
    if (buf.empty()) return false;
    nlohmann::json j = nlohmann::json::parse(buf);

    if (auto _t = dpp::string_not_null(&j, "token"); !_t.empty()) token = _t;
    if (auto _t = dpp::string_not_null(&j, "status_text"); !_t.empty()) status_text = _t;
    if (auto _t = dpp::string_not_null(&j, "status_link"); !_t.empty()) status_link = _t;
    shard_count = dpp::int32_not_null(&j, "shard_count");
    intents = dpp::int32_not_null(&j, "intents");
    status_code = dpp::int32_not_null(&j, "status_code");
    status_mode = dpp::int32_not_null(&j, "status_mode");
    
    return true;
}

bool general_config::save_as(const std::string& s)
{
    std::ofstream fout(s, std::ios::binary | std::ios::out);
    if (fout.bad() || !fout.is_open()) return false;

    nlohmann::json j;

    j["token"] = token;
    j["status_text"] = status_text;
    j["status_link"] = status_link;
    j["shard_count"] = shard_count;
    j["intents"] = intents;
    j["status_code"] = status_code;
    j["status_mode"] = status_mode;

    const auto dump = j.dump();

    fout.write(dump.data(), dump.size());

    return true;
}