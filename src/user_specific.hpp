#pragma once

#include <dpp/dpp.h>
#include <dpp/nlohmann/json.hpp>
#include <dpp/fmt/format.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <iostream>
#include <mutex>

#include "tools.hpp"
#include "secure_control.hpp"

#include "bot_reference_so_bad.hpp"
#include "memory_reference_manager.hpp"
#include "jsoner.hpp"

const std::string user_config_path_default = "./users/";
constexpr mull time_to_earn_points_sameuser_ms = 2 * 60 * 1000;

constexpr int range_low = -10;
constexpr int range_high = 35;
constexpr int range_total = range_high - range_low; // easier
constexpr int range_boost_low = 40;
constexpr int range_boost_high = 200;
constexpr int range_boost_total = range_boost_high - range_boost_low; // easier
constexpr unsigned range_boost_chances = 40; // one in 40 (it was 42)


struct user_data {
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
    int32_t pref_color = -1;
    mull times_they_got_positive_points = 0;
    mull times_they_got_negative_points = 0;

    // clipboard stuff
    struct clipboard_data : public jsonable {
        mull guild_id = 0, channel_id = 0, message_id = 0;

        void clear();
        bool has_data() const;
        std::string generate_url() const;

        nlohmann::json to_json() const;
        void from_json(const nlohmann::json&);
    };
    clipboard_data clipboard;

    // user configurable:
    bool show_level_up_messages = true;

    nlohmann::json to_json() const;
    void from_json(const nlohmann::json&);
};

class UserSelf {
    user_data data;
    std::string path;
    bool _had_update = false;
    dpp::cluster& core;
public:
    UserSelf(dpp::cluster&, const std::string&);
    ~UserSelf();

    UserSelf(UserSelf&&) noexcept;
    void operator=(UserSelf&&) noexcept;

    UserSelf(const UserSelf&) = delete;
    void operator=(const UserSelf&) = delete;

    bool save();
    std::string export_json() const;

    mull get_points() const;
    mull get_points_at_guild(const mull) const;
    bool get_show_level_up() const;

    int32_t get_user_color() const;
    void set_user_color(const int32_t);

    mull get_times_points_positive() const;
    mull get_times_points_negative() const;
    mull get_total_messages() const;
    mull get_total_messages_at_guild(const mull) const;
    mull get_total_attachments() const;
    mull get_total_attachments_at_guild(const mull) const;
    mull get_total_commands() const;

    const user_data::clipboard_data& get_clipboard() const;

    // from what guild, how many points?
    void add_points(const mull, const int);
    bool is_earning_points_time() const;
    // guild, value
    void set_points_at_guild(const mull, const mull);

    void set_show_level_up(const bool);

    void add_message_sent_once(const mull);
    void add_attachment_sent_once(const mull);
    void add_command_sent_once();

    // guild, channel, message
    void set_clipboard(const mull, const mull, const mull);
    void reset_clipboard();
};

inline MemoryReferenceManager<UserSelf> __user_memory_control([](const mull id){return UserSelf{*__global_cluster_sad_times, std::to_string(id)};}, "UserSelf");

ComplexSharedPtr<UserSelf> get_user_config(const mull);