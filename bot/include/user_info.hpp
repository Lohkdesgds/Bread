#pragma once

#include <dpp/dpp.h>
#include <dpp/nlohmann/json.hpp>
#include <dpp/fmt/format.h>
#include <console.h>
#include <bomb.h>

#include <defaults.hpp>
#include <general_functions.hpp>

struct user_info {
    using mull = unsigned long long;

    struct clipboard_data {
        mull guild_id = 0, channel_id = 0, message_id = 0;

        void clear();
        bool has_data() const;
        std::string generate_url() const;

        nlohmann::json to_json() const;
        void from_json(const nlohmann::json&);
    };

    // points stuff
    mull points = 0; // total
    std::unordered_map<mull, mull> points_per_guild; // [guild] = pts
    mull last_points_earned = 0; // related to time_to_earn_points_sameuser

    // statistics stuff
    mull messages_sent = 0;
    std::unordered_map<mull, mull> messages_sent_per_guild; // [guild] = msgs
    mull attachments_sent = 0;
    std::unordered_map<mull, mull> attachments_sent_per_guild; // [guild] = atts
    mull commands_used = 0;
    int64_t pref_color = -1;
    mull times_they_got_positive_points = 0;
    mull times_they_got_negative_points = 0;

    // clipboard stuff
    clipboard_data clipboard;

    // user configurable:
    bool show_level_up_messages = true;

    nlohmann::json to_json() const;
    void from_json(const nlohmann::json&);

    // ======== automatic things ======== //
    const dpp::snowflake __user_id;
    user_info(const dpp::snowflake&);
    ~user_info();
};