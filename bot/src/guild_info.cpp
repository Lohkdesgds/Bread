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

std::string guild_info::category::render_string_block() const
{
    std::string out;
    out = "```cs\n";
    out += "Group selected: " + name + " " + std::string(can_combine ? "[combinable]" : "[single]") + "\n";

    if (list.size() > 0) {
        dpp::cache<dpp::role>* cach = dpp::get_role_cache();
        std::shared_lock<std::shared_mutex> lu(cach->get_mutex());
        const auto& rols = cach->get_container();

        for(const auto& it : list) {
            out += " " + std::to_string(it.id) + " [" + it.name + "]: #";
            auto found = std::find_if(rols.begin(), rols.end(), [&](const std::pair<dpp::snowflake, dpp::role*>& s){ return s.first == it.id;});
            if (found != rols.end()) out += found->second->name;
            out += "\n";
        }
    }
    else {
        out += " <empty list>\n";
    }

    out += "```";
    return out;
}

nlohmann::json guild_info::to_json() const
{
    if (role_per_level.size() > 0) std::sort(role_per_level.begin(), role_per_level.end(), [](const guild_info::pair_id_level& a,const guild_info::pair_id_level& b){return a.level < b.level;});

    nlohmann::json j;
    j["last_user_earn_points"] = last_user_earn_points;
    j["fallback_levelup_message_channel"] = fallback_levelup_message_channel;
    j["guild_was_deleted"] = guild_was_deleted;
    j["guild_on_outage"] = guild_on_outage;
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
    find_json_autoabort(j, "guild_on_outage", guild_on_outage);
    find_json_autoabort(j, "block_levelup_user_event", block_levelup_user_event);
    find_json_autoabort(j, "allow_external_paste", allow_external_paste);
    find_json_autoabort(j, "commands_public", commands_public);

   if (auto pj = j.find("roles_available");         pj != j.end()) for(const auto& it : *pj) { roles_available.push_back(it); }//, roles_available);
   if (auto pj = j.find("roles_when_join");         pj != j.end()) for(const auto& it : *pj) { roles_when_join.push_back(it); }//, roles_when_join);
   if (auto pj = j.find("role_per_level");          pj != j.end()) for(const auto& it : *pj) { role_per_level.push_back(it); }//, role_per_level);
}

guild_info::guild_info(const dpp::snowflake& id)
    : __guild_id(id)
{
    auto js = get_from_file(needed_paths[guild_props::guild_path_off], std::to_string(__guild_id), ".json");
    from_json(js);
    Lunaris::cout << Lunaris::console::color::DARK_BLUE << "[MEMG] Guild #" << id << " was loaded.";
}

guild_info::~guild_info()
{
    if (save_file(to_json(), needed_paths[guild_props::guild_path_off], std::to_string(__guild_id), ".json")) {
        Lunaris::cout << Lunaris::console::color::DARK_GRAY << "[MEMG] Guild #" << __guild_id << " was unloaded.";
    }
    else {
        Lunaris::cout << Lunaris::console::color::RED << "[MEMG] Guild #" << __guild_id << " failed to unload.";
    }
}