#pragma once

#include <dpp/dpp.h>
#include <dpp/nlohmann/json.hpp>
#include <dpp/fmt/format.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <iostream>
#include <mutex>
#include <functional>

#include "tools.hpp"
#include "secure_control.hpp"
#include "language.hpp"

#include "bot_reference_so_bad.hpp"
#include "tools.hpp"
#include "memory_reference_manager.hpp"
#include "backuper.hpp"

const std::string guild_config_path_default = "./guilds/";
constexpr mull time_to_earn_points_diffuser_ms = 45 * 1000;
constexpr size_t max_logs_stored = 30;
constexpr size_t guild_log_each_max_size = 256;

struct guild_data {
    struct pair_id_name : public jsonable {
        mull id = 0;
        std::string name;

        pair_id_name() = default;
        pair_id_name(const mull, const std::string&);

        nlohmann::json to_json() const;
        void from_json(const nlohmann::json&);
    };
    struct pair_id_level : public jsonable {
        mull id = 0;
        mull level = 0;

        pair_id_level() = default;
        pair_id_level(const mull, const mull);

        nlohmann::json to_json() const;
        void from_json(const nlohmann::json&);
    };
    struct category : public jsonable {
        std::string name;
        bool can_combine = true;
        std::vector<guild_data::pair_id_name> list;

        nlohmann::json to_json() const;
        void from_json(const nlohmann::json&);
    };
    struct dummy_mutex{
        mutable std::mutex mu;
        dummy_mutex() = default;
        dummy_mutex(const dummy_mutex&){}
        dummy_mutex(dummy_mutex&&){}
        void operator=(const dummy_mutex&){}
        void operator=(dummy_mutex&&){}
    };

    std::vector<category> roles_available; // /cargos
    std::vector<mull> roles_considered_admin;
    std::vector<mull> roles_when_join;
    mutable std::vector<pair_id_level> role_per_level;
    std::string language;
    bool block_levelup_user_event = false; 
    mull last_user_earn_points = 0; // time last user earned.
    mull fallback_levelup_message_channel = 0;
    bool guild_was_deleted = false;
    unsigned last_command_version = 0;
    bool allow_external_paste = true;
    bool temp_flag_no_config = false;
    std::vector<std::string> default_poll_emojis;

    std::vector<std::pair<mull, std::string>> temp_logs;
    dummy_mutex temp_logs_mutex;

    nlohmann::json to_json() const;
    void from_json(const nlohmann::json&);
};

class GuildSelf {
    guild_data data;
    std::string path;
    bool _had_update = false;
    mutable std::shared_mutex secure;
    dpp::cluster& core;

    std::unique_lock<std::shared_mutex> luck() const;
    std::shared_lock<std::shared_mutex> luck_shr() const;

    void _sort_role_per_level_nolock();
public:
    GuildSelf(dpp::cluster&, const std::string&);
    ~GuildSelf();
    
    GuildSelf(GuildSelf&&) noexcept;
    void operator=(GuildSelf&&) noexcept;

    GuildSelf(const GuildSelf&) = delete;
    void operator=(const GuildSelf&) = delete;

    bool save();
    std::string export_json() const;
    bool import_json(const std::string&);

    void interface_roles_map(std::function<void(std::vector<guild_data::category>&)>);
    //std::vector<guild_data::category>& get_roles_map();
    const std::vector<guild_data::category>& get_roles_map() const;

    void interface_roles_per_level_map(std::function<void(std::vector<guild_data::pair_id_level>&)>);
    //std::vector<guild_data::pair_id_level>& get_roles_per_level_map();
    const std::vector<guild_data::pair_id_level>& get_roles_per_level_map() const;

    void interface_roles_admin(std::function<void(std::vector<mull>&)>);
    //std::vector<mull>& get_roles_admin();
    const std::vector<mull>& get_roles_admin() const;

    void interface_roles_joined(std::function<void(std::vector<mull>&)>);
    //std::vector<mull>& get_roles_joined();
    const std::vector<mull>& get_roles_joined() const;

    void interface_default_poll(std::function<void(std::vector<std::string>&)>);
    //std::vector<std::string>& get_default_poll();
    const std::vector<std::string>& get_default_poll() const;    

    void sort_role_per_level();

    void remove_role_admin(const mull);
    void add_role_admin(const mull);
    bool check_role_admin(const mull) const;

    void remove_role_joined(const mull);
    void add_role_joined(const mull);
    bool check_role_joined(const mull) const;

    void remove_role_level_map(const mull);
    void add_role_level_map(const guild_data::pair_id_level); // if LEVEL exists, ID changes. if ID exists but not ID, duplicates ID with new LEVEL.

    bool is_message_level_blocked() const;
    void set_message_level_blocked(const bool);

    bool is_earning_points_time() const;
    void set_earned_points();

    const std::string& get_language() const;
    void set_language(const std::string&);

    void post_log(std::string);
    std::string get_log(const size_t) const;
    size_t get_log_size() const;

    mull get_level_channel_id() const;
    void set_level_channel_id(const mull);

    unsigned get_current_command_version() const;
    void set_current_command_version(const unsigned);

    bool get_can_paste_external_content() const;
    void set_can_paste_external_content(const bool);

    bool is_guild_deleted() const;
    void set_guild_deleted(const bool);

    bool is_config_locked() const;
    void set_config_locked(const bool);
};

inline MemoryReferenceManager<GuildSelf> __guild_memory_control(
    [](const mull id){return GuildSelf{*__global_cluster_sad_times, std::to_string(id)};},
    [](GuildSelf& gs){ gs.save(); },
    "GuildSelf");

ComplexSharedPtr<GuildSelf> get_guild_config(const mull);
//void delete_guild_config(const mull); // kick or ban from guild = delete configuration