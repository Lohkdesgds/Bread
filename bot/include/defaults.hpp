#pragma once

#include <dpp/dpp.h>
#include <string>

const std::string needed_paths[] = {"./bot/", "./guilds/", "./users/"};
const std::string config_path = "./bot/config.json";
const std::string slash_path = "./bot/slashes.json";
const auto default_intents = dpp::i_default_intents | dpp::i_message_content;

const auto factory_default_time = std::chrono::seconds(120); // 2 min in memory

namespace user_props {
    constexpr size_t user_path_off = 2; // needed_paths[2]
    constexpr unsigned long long time_to_earn_points_sameuser_ms = 2 * 60 * 1000;

    constexpr int range_low = -10;
    constexpr int range_high = 35;
    constexpr int range_total = range_high - range_low; // easier
    constexpr int range_boost_low = 40;
    constexpr int range_boost_high = 200;
    constexpr int range_boost_total = range_boost_high - range_boost_low; // easier
    constexpr unsigned range_boost_chances = 40; // one in 40 (it was 42)
}

namespace guild_props {
    constexpr size_t guild_path_off = 1; // needed_paths[1]
    constexpr unsigned long long time_to_earn_points_diffuser_ms = 45 * 1000;
    constexpr size_t guild_log_each_max_size = 256;

    constexpr size_t max_onjoin_roles_len = 20;
    
    constexpr size_t max_role_groups = 8;
    constexpr size_t max_role_group_each = 20;
}

namespace images {
    const std::string botstatus_image_url = "https://media.discordapp.net/attachments/739704685505544363/740803216626679808/personal_computer.png?width=468&height=468";
    //const std::string botstatus_image_dpp_url = "https://cdn.discordapp.com/attachments/739704685505544363/886048618711244851/dpp_image.png?size=256";

    const std::string url_author_icon_clipboard = "https://media.discordapp.net/attachments/739704685505544363/872539023057489920/folder_icon.png";
}