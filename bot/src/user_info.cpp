#include <user_info.hpp>

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
    find_json_array_autoabort(j, "messages_sent_per_guild", messages_sent_per_guild);
    find_json_autoabort(j, "attachments_sent", attachments_sent);
    find_json_array_autoabort(j, "attachments_sent_per_guild", attachments_sent_per_guild);
    find_json_autoabort(j, "commands_used", commands_used);
    find_json_autoabort(j, "pref_color", pref_color);
    find_json_autoabort(j, "times_they_got_positive_points", times_they_got_positive_points);
    find_json_autoabort(j, "times_they_got_negative_points", times_they_got_negative_points);
    find_json_autoabort(j, "clipboard", clipboard);
    find_json_autoabort(j, "show_level_up_messages", show_level_up_messages);
    find_json_array_autoabort(j, "points_per_guild", points_per_guild);
}