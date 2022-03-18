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