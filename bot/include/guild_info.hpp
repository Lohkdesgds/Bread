#pragma once

#include <dpp/dpp.h>
#include <dpp/nlohmann/json.hpp>
#include <dpp/fmt/format.h>
#include <console.h>
#include <bomb.h>

#include <defaults.hpp>
#include <general_functions.hpp>

struct guild_info {
    using mull = unsigned long long;

    struct pair_id_name {
        mull id = 0;
        std::string name;

        pair_id_name() = default;
        pair_id_name(const mull, const std::string&);

        nlohmann::json to_json() const;
        void from_json(const nlohmann::json&);
        pair_id_name(const nlohmann::json&);
    };
    struct pair_id_level {
        mull id = 0;
        mull level = 0;

        pair_id_level() = default;
        pair_id_level(const mull, const mull);

        nlohmann::json to_json() const;
        void from_json(const nlohmann::json&);
        pair_id_level(const nlohmann::json&);
    };
    struct category {
        std::string name;
        bool can_combine = true;
        std::vector<guild_info::pair_id_name> list;

        nlohmann::json to_json() const;
        void from_json(const nlohmann::json&);
        category(const nlohmann::json&);
        category() = default;
    };

    std::vector<category> roles_available; // /roles
    std::vector<mull> roles_when_join;
    mutable std::vector<pair_id_level> role_per_level;
    bool block_levelup_user_event = false; 
    mull last_user_earn_points = 0; // time last user earned.
    mull fallback_levelup_message_channel = 0;
    bool guild_was_deleted = false;
    bool allow_external_paste = true;

    nlohmann::json to_json() const;
    void from_json(const nlohmann::json&);

    // ======== automatic things ======== //
    const dpp::snowflake __guild_id;
    guild_info(const dpp::snowflake&);
    ~guild_info();
};