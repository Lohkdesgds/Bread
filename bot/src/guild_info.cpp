#include <guild_info.hpp>

guild_info::pair_id_name::pair_id_name(const mull a, const std::string& b)
    : id(a), name(b)
{
}

nlohmann::json guild_info::pair_id_name::to_json() const
{
    nlohmann::json j;
    j["name"] = name;
    j["id"] = id;
    return j;
}

void guild_info::pair_id_name::from_json(const nlohmann::json& j)
{
    find_json_autoabort(j, "name", name);
    find_json_autoabort(j, "id", id);
}

guild_info::pair_id_name::pair_id_name(const nlohmann::json& j)
{
    from_json(j);
}

guild_info::pair_id_level::pair_id_level(const mull a, const mull b)
    : id(a), level(b)
{    
}

nlohmann::json guild_info::pair_id_level::to_json() const
{
    nlohmann::json j;
    j["id"] = id;
    j["level"] = level;
    return j;
}

void guild_info::pair_id_level::from_json(const nlohmann::json& j)
{
    find_json_autoabort(j, "id", id);
    find_json_autoabort(j, "level", level);
}

guild_info::pair_id_level::pair_id_level(const nlohmann::json& j)
{
    from_json(j);
}

nlohmann::json guild_info::category::to_json() const
{
    nlohmann::json j;
    j["name"] = name;
    j["can_combine"] = can_combine;
    for(const auto& _field : list) j["list"].push_back(_field.to_json());
    //for(const auto& _field : roles_whitelist) j["roles_whitelist"].push_back(_field);
    return j;
}

void guild_info::category::from_json(const nlohmann::json& j)
{
    find_json_autoabort(j, "name", name);
    find_json_autoabort(j, "can_combine", can_combine);
    find_json_array_autoabort<guild_info::pair_id_name>(j, "list", list);
}

guild_info::category::category(const nlohmann::json& j)
{
    from_json(j);
}

nlohmann::json guild_info::to_json() const
{
    if (role_per_level.size() > 0) std::sort(role_per_level.begin(), role_per_level.end(), [](const guild_info::pair_id_level& a,const guild_info::pair_id_level& b){return a.level < b.level;});

    nlohmann::json j;
    j["last_user_earn_points"] = last_user_earn_points;
    j["fallback_levelup_message_channel"] = fallback_levelup_message_channel;
    j["guild_was_deleted"] = guild_was_deleted;
    j["block_levelup_user_event"] = block_levelup_user_event;
    j["allow_external_paste"] = allow_external_paste;
    j["commands_public"] = commands_public;
    for(const auto& _field : roles_available) j["roles_available"].push_back(_field.to_json());
    for(const auto& _field : roles_when_join) j["roles_when_join"].push_back(_field);
    for(const auto& _field : role_per_level) j["role_per_level"].push_back(_field.to_json());
    return j;
}

void guild_info::from_json(const nlohmann::json& j)
{
    if (j.empty()) return;
    find_json_autoabort(j, "last_user_earn_points", last_user_earn_points);
    find_json_autoabort(j, "fallback_levelup_message_channel", fallback_levelup_message_channel);
    find_json_autoabort(j, "guild_was_deleted", guild_was_deleted);
    find_json_autoabort(j, "block_levelup_user_event", block_levelup_user_event);
    find_json_autoabort(j, "allow_external_paste", allow_external_paste);
    find_json_autoabort(j, "commands_public", commands_public);

   if (auto pj = j.find("roles_available");         pj != j.end()) for(const auto& it : *pj) { roles_available.push_back(it); }//, roles_available);
   if (auto pj = j.find("roles_when_join");         pj != j.end()) for(const auto& it : *pj) { roles_when_join.push_back(it); }//, roles_when_join);
   if (auto pj = j.find("role_per_level");          pj != j.end()) for(const auto& it : *pj) { role_per_level.push_back(it); }//, role_per_level);

    //find_json_array_autoabort(j, "roles_available", roles_available);
    //find_json_array_autoabort(j, "roles_considered_admin", roles_considered_admin);
    //find_json_array_autoabort(j, "roles_when_join", roles_when_join);
    //find_json_array_autoabort(j, "role_per_level", role_per_level);
    //find_json_array_autoabort(j, "temp_logs", temp_logs);
    //find_json_array_autoabort(j, "default_poll_emojis", default_poll_emojis);
}

guild_info::guild_info(const dpp::snowflake& id)
    : __guild_id(id)
{
//    std::ifstream cfile(needed_paths[guild_props::guild_path_off] + std::to_string(__guild_id));
//    if (!cfile.is_open() || !cfile.good()) return;
//
//    std::stringstream buffer;
//    buffer << cfile.rdbuf();
//    const auto js = nlohmann::json::parse(buffer.str(), nullptr);

    auto js = get_from_file(needed_paths[guild_props::guild_path_off], std::to_string(__guild_id), ".json");

    from_json(js);
}

guild_info::~guild_info()
{
    save_file(to_json(), needed_paths[guild_props::guild_path_off], std::to_string(__guild_id), ".json");
//    std::ofstream cfile(needed_paths[guild_props::guild_path_off] + std::to_string(__guild_id));
//    if (!cfile.is_open() || !cfile.good()) {
//        Lunaris::cout << Lunaris::console::color::RED << "FATAL ERROR: Can't save guild #" << __guild_id;
//        return;
//    }
//
//    cfile << to_json();
}