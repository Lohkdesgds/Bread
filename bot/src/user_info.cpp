#include <user_info.hpp>
#include <general_functions.hpp>

void user_info::clipboard_data::clear()
{
    guild_id = message_id = channel_id = 0;
}

bool user_info::clipboard_data::has_data() const
{
    return guild_id != 0 && channel_id != 0 && message_id != 0;
}

std::string user_info::clipboard_data::generate_url() const
{
    return "https://discord.com/channels/" + std::to_string(guild_id) + "/" + std::to_string(channel_id) + "/" + std::to_string(message_id);
}

nlohmann::json user_info::clipboard_data::to_json() const
{
    nlohmann::json j;
    j["guild_id"] = guild_id;
    j["channel_id"] = channel_id;
    j["message_id"] = message_id;
    return j;
}

void user_info::clipboard_data::from_json(const nlohmann::json& j)
{
    find_json_autoabort(j, "guild_id", guild_id);
    find_json_autoabort(j, "channel_id", channel_id);
    find_json_autoabort(j, "message_id", message_id);
}

nlohmann::json user_info::to_json() const
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

void user_info::from_json(const nlohmann::json& j)
{
    if (j.empty()) return;
    find_json_autoabort(j, "points", points);
    find_json_autoabort(j, "last_points_earned", last_points_earned);
    find_json_autoabort(j, "messages_sent", messages_sent);
    find_json_array_autoabort<std::pair<mull,mull>>(j, "messages_sent_per_guild", messages_sent_per_guild);
    find_json_autoabort(j, "attachments_sent", attachments_sent);
    find_json_array_autoabort<std::pair<mull,mull>>(j, "attachments_sent_per_guild", attachments_sent_per_guild);
    find_json_autoabort(j, "commands_used", commands_used);
    find_json_autoabort(j, "pref_color", pref_color);
    find_json_autoabort(j, "times_they_got_positive_points", times_they_got_positive_points);
    find_json_autoabort(j, "times_they_got_negative_points", times_they_got_negative_points);
    if (j.contains("clipboard")) clipboard.from_json(j["clipboard"]);
    find_json_autoabort(j, "show_level_up_messages", show_level_up_messages);
    find_json_array_autoabort<std::pair<mull,mull>>(j, "points_per_guild", points_per_guild);
}

unsigned long long user_info::get_points_on_guild(const unsigned long long& i) const
{
    if (auto it = points_per_guild.find(i); it != points_per_guild.end()) return it->second;
    return 0;
}

unsigned long long user_info::get_messages_on_guild(const unsigned long long& i) const
{
    if (auto it = messages_sent_per_guild.find(i); it != messages_sent_per_guild.end()) return it->second;
    return 0;
}

unsigned long long user_info::get_attachments_on_guild(const unsigned long long& i) const
{
    if (auto it = attachments_sent_per_guild.find(i); it != attachments_sent_per_guild.end()) return it->second;
    return 0;
}

user_info::user_info(const dpp::snowflake& id)
    : __user_id(id)
{
    auto js = get_from_file(needed_paths[user_props::user_path_off], std::to_string(__user_id), ".json");
    from_json(js);
    Lunaris::cout << Lunaris::console::color::DARK_BLUE << "[MEMU] User #" << id << " was loaded.";
}

user_info::~user_info()
{
    if (save_file(to_json(), needed_paths[user_props::user_path_off], std::to_string(__user_id), ".json")) {
        Lunaris::cout << Lunaris::console::color::DARK_GRAY << "[MEMU] User #" << __user_id << " was unloaded.";
    }
    else {
        Lunaris::cout << Lunaris::console::color::RED << "[MEMU] User #" << __user_id << " failed to unload.";
    }
}