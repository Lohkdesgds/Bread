#include "user_specific.hpp"

void user_data::clipboard_data::clear()
{
    guild_id = message_id = channel_id = 0;
}

bool user_data::clipboard_data::has_data() const
{
    return guild_id != 0 && channel_id != 0 && message_id != 0;
}

std::string user_data::clipboard_data::generate_url() const
{
    return "https://discord.com/channels/" + std::to_string(guild_id) + "/" + std::to_string(channel_id) + "/" + std::to_string(message_id);
}

nlohmann::json user_data::clipboard_data::to_json() const
{
    nlohmann::json j;
    j["guild_id"] = guild_id;
    j["channel_id"] = channel_id;
    j["message_id"] = message_id;
    return j;
}

void user_data::clipboard_data::from_json(const nlohmann::json& j)
{
    safe_json(j, "guild_id", guild_id);
    safe_json(j, "channel_id", channel_id);
    safe_json(j, "message_id", message_id);
}

nlohmann::json user_data::to_json() const
{
    nlohmann::json j;

    j["points"] = points;
    j["last_points_earned"] = last_points_earned;

    j["messages_sent"] = messages_sent;
    for(const auto& i : messages_sent_per_guild) {
        if (i.second > 0) j["messages_sent_per_guild"].push_back(i);
    }
    j["attachments_sent"] = attachments_sent;
    for(const auto& i : attachments_sent_per_guild) {
        if (i.second > 0) j["attachments_sent_per_guild"].push_back(i);
    }
    j["commands_used"] = commands_used;
    j["pref_color"] = pref_color;
    j["times_they_got_positive_points"] = times_they_got_positive_points;
    j["times_they_got_negative_points"] = times_they_got_negative_points;

    j["show_level_up_messages"] = show_level_up_messages;

    if (clipboard.has_data()) j["clipboard"] = clipboard.to_json();

    for(const auto& i : points_per_guild) {
        if (i.second > 0) j["points_per_guild"].push_back(i);
    }
    return j;
}

void user_data::from_json(const nlohmann::json& j)
{
    if (j.empty()) return;
    safe_json(j, "points", points, false);
    safe_json(j, "last_points_earned", last_points_earned, false);
    safe_json(j, "messages_sent", messages_sent, false);
    safe_json_array(j, "messages_sent_per_guild", messages_sent_per_guild, false);
    safe_json(j, "attachments_sent", attachments_sent, false);
    safe_json_array(j, "attachments_sent_per_guild", attachments_sent_per_guild, false);
    safe_json(j, "commands_used", commands_used, false);
    safe_json(j, "pref_color", pref_color, false);
    safe_json(j, "times_they_got_positive_points", times_they_got_positive_points, false);
    safe_json(j, "times_they_got_negative_points", times_they_got_negative_points, false);
    safe_json(j, "clipboard", clipboard, false);
    safe_json(j, "show_level_up_messages", show_level_up_messages, false);
    safe_json_array(j, "points_per_guild", points_per_guild, false);
}

std::unique_lock<std::shared_mutex> UserSelf::luck() const
{
    return std::unique_lock<std::shared_mutex>(secure);
}

std::shared_lock<std::shared_mutex> UserSelf::luck_shr() const
{
    return std::shared_lock<std::shared_mutex>(secure);
}

UserSelf::UserSelf(dpp::cluster& cor, const std::string& p)
    : path(p), core(cor)
{
    nlohmann::json j = get_from_file(user_config_path_default, path, ".json");
    if (j.is_discarded() || j.empty()) return;
    data.from_json(j);
}

UserSelf::~UserSelf()
{
    if (_had_update) save();
}

UserSelf::UserSelf(UserSelf&& oth) noexcept
    : data(oth.data), path(oth.path), _had_update(oth._had_update), core(oth.core)
{
    auto l2 = oth.luck();

    oth.path.clear();
    oth._had_update = false;
}

void UserSelf::operator=(UserSelf&& oth) noexcept
{
    auto l1 = luck();
    auto l2 = oth.luck();

    data = std::move(oth.data);
    path = std::move(oth.path);
    _had_update = oth._had_update;
    oth.path.clear();
    oth._had_update = false;
}

bool UserSelf::save()
{
    auto l1 = luck();

    if (path.empty()) return false;
    const auto mj = data.to_json();
    if (!save_file(core, mj, user_config_path_default, path, ".json")) {
        Lunaris::cout << Lunaris::console::color::RED << "[UserSelf] Could not flush " << path;
    }
    else _had_update = false;
    return true;
}

std::string UserSelf::export_json() const
{
    auto l1 = luck_shr();
    return data.to_json().dump(1);
}

mull UserSelf::get_points() const
{
    auto l1 = luck_shr();
    return data.points;
}

mull UserSelf::get_points_at_guild(const mull gid) const
{
    auto l1 = luck_shr();
    auto it = data.points_per_guild.find(gid);
    if (it == data.points_per_guild.end()) return 0;
    return it->second;
}

bool UserSelf::get_show_level_up() const
{
    auto l1 = luck_shr();
    return data.show_level_up_messages;
}

int64_t UserSelf::get_user_color() const
{
    auto l1 = luck_shr();
    return (data.pref_color < 0 ? random() : data.pref_color) % 0xFFFFFF;
}

void UserSelf::set_user_color(const int64_t val)
{
    auto l1 = luck();
    data.pref_color = val < 0 ? -1 : val;
    _had_update = true;
}

mull UserSelf::get_times_points_positive() const
{
    auto l1 = luck_shr();
    return data.times_they_got_positive_points;
}

mull UserSelf::get_times_points_negative() const
{
    auto l1 = luck_shr();
    return data.times_they_got_negative_points;
}

mull UserSelf::get_total_messages() const
{
    auto l1 = luck_shr();
    return data.messages_sent;
}

mull UserSelf::get_total_messages_at_guild(const mull gid) const
{
    auto l1 = luck_shr();
    auto it = data.messages_sent_per_guild.find(gid);
    if (it != data.messages_sent_per_guild.end()) return it->second;
    return 0;
}

mull UserSelf::get_total_attachments() const
{
    auto l1 = luck_shr();
    return data.attachments_sent;
}

mull UserSelf::get_total_attachments_at_guild(const mull gid) const
{
    auto l1 = luck_shr();
    auto it = data.attachments_sent_per_guild.find(gid);
    if (it != data.attachments_sent_per_guild.end()) return it->second;
    return 0;
}

mull UserSelf::get_total_commands() const
{
    auto l1 = luck_shr();
    return data.commands_used;
}

const user_data::clipboard_data& UserSelf::get_clipboard() const
{
    auto l1 = luck_shr();
    return data.clipboard;
}

void UserSelf::add_points(const mull gid, const int pts)
{
    auto l1 = luck();
    data.last_points_earned = get_time_ms() + time_to_earn_points_sameuser_ms;

    if (pts < 0 && data.points_per_guild[gid] < (-pts)) {
        data.points_per_guild[gid] = 0;
        if (data.points < (-pts)) data.points = 0;
        else data.points += pts;
    }
    else {
        data.points_per_guild[gid] += pts;
        data.points += pts;
    }

    if (pts > 0) data.times_they_got_positive_points++;
    if (pts < 0) data.times_they_got_negative_points++;

    _had_update = true;
}

bool UserSelf::is_earning_points_time() const
{
    auto l1 = luck_shr();
    return get_time_ms() > data.last_points_earned;
}

void UserSelf::set_points_at_guild(const mull gid, const mull val)
{
    auto l1 = luck();
    data.points_per_guild[gid] = val;
    _had_update = true;
}

void UserSelf::set_show_level_up(const bool shw)
{
    auto l1 = luck();
    data.show_level_up_messages = shw;
    _had_update = true;
}

void UserSelf::add_message_sent_once(const mull gid)
{
    auto l1 = luck();
    data.messages_sent_per_guild[gid]++;
    data.messages_sent++;
    _had_update = true;
}

void UserSelf::add_attachment_sent_once(const mull gid)
{
    auto l1 = luck();
    data.attachments_sent_per_guild[gid]++;
    data.attachments_sent++;
    _had_update = true;
}

void UserSelf::add_command_sent_once()
{
    auto l1 = luck();
    data.commands_used++;
    _had_update = true;
}

void UserSelf::set_clipboard(const mull g, const mull c, const mull m)
{
    auto l1 = luck();
    data.clipboard.guild_id = g;
    data.clipboard.channel_id = c;
    data.clipboard.message_id = m;
    _had_update = true;
}

void UserSelf::reset_clipboard()
{
    auto l1 = luck();
    data.clipboard.clear();
    _had_update = true;
}

ComplexSharedPtr<UserSelf> get_user_config(const mull uuid)
{
    if (uuid == 0) throw std::runtime_error("Invalid ID null");
    auto shr = __user_memory_control.get(uuid);
    return shr;
}