#include "guild_specific.hpp"

guild_data::pair_id_name::pair_id_name(const mull a, const std::string& b)
    : id(a), name(b)
{
}

nlohmann::json guild_data::pair_id_name::to_json() const
{
    nlohmann::json j;
    j["name"] = name;
    j["id"] = id;
    return j;
}

void guild_data::pair_id_name::from_json(const nlohmann::json& j)
{
    safe_json(j, "name", name);
    safe_json(j, "id", id);
}

guild_data::pair_id_level::pair_id_level(const mull a, const mull b)
    : id(a), level(b)
{    
}

nlohmann::json guild_data::pair_id_level::to_json() const
{
    nlohmann::json j;
    j["id"] = id;
    j["level"] = level;
    return j;
}

void guild_data::pair_id_level::from_json(const nlohmann::json& j)
{
    safe_json(j, "id", id);
    safe_json(j, "level", level);
}

nlohmann::json guild_data::category::to_json() const
{
    nlohmann::json j;
    j["name"] = name;
    j["can_combine"] = can_combine;
    for(const auto& _field : list) j["list"].push_back(_field.to_json());
    //for(const auto& _field : roles_whitelist) j["roles_whitelist"].push_back(_field);
    return j;
}

void guild_data::category::from_json(const nlohmann::json& j)
{
    safe_json(j, "name", name);
    safe_json(j, "can_combine", can_combine);
    safe_json_array(j, "list", list, false);
}

nlohmann::json guild_data::to_json() const
{
    if (role_per_level.size() > 0) std::sort(role_per_level.begin(), role_per_level.end(), [](const guild_data::pair_id_level& a,const guild_data::pair_id_level& b){return a.level < b.level;});

    nlohmann::json j;
    j["last_user_earn_points"] = last_user_earn_points;
    j["last_command_version"] = last_command_version;
    j["fallback_levelup_message_channel"] = fallback_levelup_message_channel;
    j["guild_was_deleted"] = guild_was_deleted;
    j["block_levelup_user_event"] = block_levelup_user_event;
    j["language"] = language;
    j["allow_external_paste"] = allow_external_paste;
    j["temp_flag_no_config"] = temp_flag_no_config;
    for(const auto& _field : roles_available) j["roles_available"].push_back(_field.to_json());
    for(const auto& _field : roles_considered_admin) j["roles_considered_admin"].push_back(_field);
    for(const auto& _field : roles_when_join) j["roles_when_join"].push_back(_field);
    for(const auto& _field : role_per_level) j["role_per_level"].push_back(_field.to_json());
    for(const auto& _field : temp_logs) j["temp_logs"].push_back(_field);
    for(const auto& _field : default_poll_emojis) j["default_poll_emojis"].push_back(_field);
    return j;
}

void guild_data::from_json(const nlohmann::json& j)
{
    if (j.empty()) return;
    safe_json(j, "last_user_earn_points", last_user_earn_points, false);
    safe_json(j, "last_command_version", last_command_version, false);
    safe_json(j, "fallback_levelup_message_channel", fallback_levelup_message_channel, false);
    safe_json(j, "guild_was_deleted", guild_was_deleted, false);
    safe_json(j, "block_levelup_user_event", block_levelup_user_event, false);
    safe_json(j, "language", language, false);
    safe_json(j, "allow_external_paste", allow_external_paste, false);
    safe_json(j, "temp_flag_no_config", temp_flag_no_config, false);
    safe_json_array(j, "roles_available", roles_available, false);
    safe_json_array(j, "roles_considered_admin", roles_considered_admin, false);
    safe_json_array(j, "roles_when_join", roles_when_join, false);
    safe_json_array(j, "role_per_level", role_per_level, false);
    safe_json_array(j, "temp_logs", temp_logs, false);
    safe_json_array(j, "default_poll_emojis", default_poll_emojis, false);
}

std::unique_lock<std::shared_mutex> GuildSelf::luck() const
{
    return std::unique_lock<std::shared_mutex>(secure);
}

std::shared_lock<std::shared_mutex> GuildSelf::luck_shr() const
{
    return std::shared_lock<std::shared_mutex>(secure);
}

void GuildSelf::_sort_role_per_level_nolock()
{
    if (data.role_per_level.size() > 0)
        std::sort(data.role_per_level.begin(), data.role_per_level.end(), [](const guild_data::pair_id_level& a,const guild_data::pair_id_level& b){return a.level < b.level;});
}

GuildSelf::GuildSelf(dpp::cluster& cor, const std::string& p)
    : path(p), core(cor)
{
    nlohmann::json j = get_from_file(guild_config_path_default, path, ".json");
    if (j.is_discarded() || j.empty()) return;
    data.from_json(j);
}

GuildSelf::~GuildSelf()
{
    if (_had_update) save();
}

GuildSelf::GuildSelf(GuildSelf&& oth) noexcept
    : data(oth.data), path(oth.path), _had_update(oth._had_update), core(oth.core)
{
    auto l2 = oth.luck();

    oth.path.clear();
    oth._had_update = false;
}

void GuildSelf::operator=(GuildSelf&& oth) noexcept
{
    auto l1 = luck();
    auto l2 = oth.luck();

    data = std::move(oth.data);
    path = std::move(oth.path);
    _had_update = oth._had_update;
    oth.path.clear();
    oth._had_update = false;
}

bool GuildSelf::save()
{
    auto l1 = luck();

    if (path.empty()) return false;
    const auto mj = data.to_json();
    if (!save_file(core, mj, guild_config_path_default, path, ".json")) {
        Lunaris::cout << Lunaris::console::color::RED << "[UserSelf] Could not flush " << path;
    }
    else _had_update = false;
    return true;
}

std::string GuildSelf::export_json() const
{
    auto l1 = luck_shr();
    return data.to_json().dump(1);
}

bool GuildSelf::import_json(const std::string& str)
{
    auto l1 = luck();
    nlohmann::json j = nlohmann::json::parse(str, nullptr, false);
    if (!j.empty() && !j.is_discarded()) {
        data.from_json(j);
        return true;
    }
    return false;
}

//std::vector<guild_data::category>& GuildSelf::get_roles_map()
//{
//    _had_update = true;
//    return data.roles_available;
//}

void GuildSelf::interface_roles_map(std::function<void(std::vector<guild_data::category>&)> f)
{
    if (f){
        auto l1 = luck();
        _had_update = true;
        f(data.roles_available);
    }
}

const std::vector<guild_data::category>& GuildSelf::get_roles_map() const
{
    auto l1 = luck_shr();
    return data.roles_available;
}

//std::vector<guild_data::pair_id_level>& GuildSelf::get_roles_per_level_map()
//{
//    _had_update = true;
//    return data.role_per_level;
//}

void GuildSelf::interface_roles_per_level_map(std::function<void(std::vector<guild_data::pair_id_level>&)> f)
{
    if (f){
        auto l1 = luck();
        _had_update = true;
        f(data.role_per_level);
    }
}

const std::vector<guild_data::pair_id_level>& GuildSelf::get_roles_per_level_map() const
{
    auto l1 = luck_shr();
    return data.role_per_level;
}

//std::vector<mull>& GuildSelf::get_roles_admin()
//{
//    _had_update = true;
//    return data.roles_considered_admin;
//}

void GuildSelf::interface_roles_admin(std::function<void(std::vector<mull>&)> f)
{
    if (f){
        auto l1 = luck();
        _had_update = true;
        f(data.roles_considered_admin);
    }
}

const std::vector<mull>& GuildSelf::get_roles_admin() const
{
    auto l1 = luck_shr();
    return data.roles_considered_admin;
}

//std::vector<mull>& GuildSelf::get_roles_joined()
//{
//    _had_update = true;
//    return data.roles_when_join;
//}

void GuildSelf::interface_roles_joined(std::function<void(std::vector<mull>&)> f)
{
    if (f){
        auto l1 = luck();
        _had_update = true;
        f(data.roles_when_join);
    }
}

const std::vector<mull>& GuildSelf::get_roles_joined() const
{
    auto l1 = luck_shr();
    return data.roles_when_join;
}

//std::vector<std::string>& GuildSelf::get_default_poll()
//{
//    _had_update = true;
//    return data.default_poll_emojis;
//}

void GuildSelf::interface_default_poll(std::function<void(std::vector<std::string>&)> f)
{
    if (f){
        auto l1 = luck();
        _had_update = true;
        f(data.default_poll_emojis);
    }
}

const std::vector<std::string>& GuildSelf::get_default_poll() const
{
    auto l1 = luck_shr();
    return data.default_poll_emojis;
}

void GuildSelf::sort_role_per_level()
{
    auto l1 = luck();
    _sort_role_per_level_nolock();
}

void GuildSelf::remove_role_admin(const mull var)
{
    auto l1 = luck();
    if (auto it = std::find(data.roles_considered_admin.begin(), data.roles_considered_admin.end(), var); it != data.roles_considered_admin.end()){
        data.roles_considered_admin.erase(it);
        _had_update = true;
    }
}

void GuildSelf::add_role_admin(const mull var)
{
    auto l1 = luck();
    if (auto it = std::find(data.roles_considered_admin.begin(), data.roles_considered_admin.end(), var); it == data.roles_considered_admin.end()){
        data.roles_considered_admin.push_back(var);
        _had_update = true;
    }
}

bool GuildSelf::check_role_admin(const mull var) const
{
    auto l1 = luck_shr();
    return std::find(data.roles_considered_admin.begin(), data.roles_considered_admin.end(), var) != data.roles_considered_admin.end();
}

void GuildSelf::remove_role_joined(const mull var)
{
    auto l1 = luck();
    if (auto it = std::find(data.roles_when_join.begin(), data.roles_when_join.end(), var); it != data.roles_when_join.end()){
        data.roles_when_join.erase(it);
        _had_update = true;
    }
}

void GuildSelf::add_role_joined(const mull var)
{
    auto l1 = luck();
    if (auto it = std::find(data.roles_when_join.begin(), data.roles_when_join.end(), var); it == data.roles_when_join.end()){
        data.roles_when_join.push_back(var);
        _had_update = true;
    }
}

bool GuildSelf::check_role_joined(const mull var) const
{
    auto l1 = luck_shr();
    return std::find(data.roles_when_join.begin(), data.roles_when_join.end(), var) != data.roles_when_join.end();
}

void GuildSelf::remove_role_level_map(const mull id)
{
    auto l1 = luck();
    if (auto it = std::find_if(data.role_per_level.begin(), data.role_per_level.end(), [&](const guild_data::pair_id_level& a){return a.id == id;}); it != data.role_per_level.end()){
        data.role_per_level.erase(it);
        _sort_role_per_level_nolock();
        _had_update = true;
    }
}

void GuildSelf::add_role_level_map(const guild_data::pair_id_level par)
{
    auto l1 = luck();
    if (auto it = std::find_if(data.role_per_level.begin(), data.role_per_level.end(), [&](const guild_data::pair_id_level& a){return a.level == par.level;}); it != data.role_per_level.end()){
        it->id = par.id;
    }
    else data.role_per_level.push_back(par);
    _sort_role_per_level_nolock();
    _had_update = true;
}

bool GuildSelf::is_message_level_blocked() const
{
    auto l1 = luck_shr();
    return data.block_levelup_user_event;
}

void GuildSelf::set_message_level_blocked(const bool var)
{
    auto l1 = luck();
    data.block_levelup_user_event = var;
    _had_update = true;
}

bool GuildSelf::is_earning_points_time() const
{
    auto l1 = luck_shr();
    return get_time_ms() > data.last_user_earn_points;
}

void GuildSelf::set_earned_points()
{
    auto l1 = luck();
    data.last_user_earn_points = get_time_ms() + time_to_earn_points_diffuser_ms;
    _had_update = true;
}

const std::string& GuildSelf::get_language() const
{
    auto l1 = luck_shr();
    return data.language;
}

void GuildSelf::set_language(const std::string& var)
{
    auto l1 = luck();
    data.language = var;
    _had_update = true;
}

void GuildSelf::post_log(std::string str)
{
    if (str.empty()) return; // empty log?
    if (str.size() > guild_log_each_max_size){
        str = str.substr(0, guild_log_each_max_size - 3) + "...";
    }

    std::lock_guard<std::mutex> luck(data.temp_logs_mutex.mu);
    data.temp_logs.push_back({get_time_ms(), std::move(str)});
    if (data.temp_logs.size() >= max_logs_stored) data.temp_logs.erase(data.temp_logs.begin());
    _had_update = true;
}

std::string GuildSelf::get_log(const size_t index) const
{
    std::lock_guard<std::mutex> luck(data.temp_logs_mutex.mu);
    return ((index >= data.temp_logs.size()) ? "" : ("<t:" + std::to_string(static_cast<unsigned long long>(data.temp_logs[index].first / 1000)) + ":T> " + data.temp_logs[index].second));
}

size_t GuildSelf::get_log_size() const
{
    auto l1 = luck_shr();
    return data.temp_logs.size();
}

mull GuildSelf::get_level_channel_id() const
{
    auto l1 = luck_shr();
    return data.fallback_levelup_message_channel;
}

void GuildSelf::set_level_channel_id(const mull var)
{
    auto l1 = luck();
    data.fallback_levelup_message_channel = var;
    _had_update = true;
}

unsigned GuildSelf::get_current_command_version() const
{
    auto l1 = luck_shr();
    return data.last_command_version;
}

void GuildSelf::set_current_command_version(const unsigned var)
{
    auto l1 = luck();
    data.last_command_version = var;
    _had_update = true;
}

bool GuildSelf::get_can_paste_external_content() const
{
    auto l1 = luck_shr();
    return data.allow_external_paste;
}

void GuildSelf::set_can_paste_external_content(const bool var)
{
    auto l1 = luck();
    data.allow_external_paste = var;
    _had_update = true;
}

void GuildSelf::set_guild_deleted(const bool var)
{
    auto l1 = luck();
    data.guild_was_deleted = var;
    _had_update = true;
}

bool GuildSelf::is_config_locked() const
{
    auto l1 = luck_shr();
    return data.temp_flag_no_config;
}

void GuildSelf::set_config_locked(const bool var)
{
    auto l1 = luck();
    data.temp_flag_no_config = var;
    _had_update = true;
}

ComplexSharedPtr<GuildSelf> get_guild_config(const mull uuid)
{
    if (uuid == 0) throw std::runtime_error("Invalid ID null");
    auto shr = __guild_memory_control.get(uuid);
    return shr;
}