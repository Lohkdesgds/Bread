#pragma once

#include <dpp/dpp.h>
#include <dpp/nlohmann/json.hpp>
#include <dpp/fmt/format.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <iostream>
#include <mutex>
#include <unordered_map>
#include <vector>
#include <initializer_list>

#include "tools.hpp"

const std::string language_config_path = "./lang/";
const std::string language_seek_filename = "__config.json";

enum class lang_line{ // 0 TO _MAX
    // format:
    // - *_FINAL_* = final response from COMMAND
    // - *_MESSAGE_* = common message 

    GENERIC_BOT_FAILED_DISCORD_ERROR,
    GENERIC_BOT_FAILED_DOING_TASK,
    GENERIC_BOT_SUCCESS,
    GENERIC_BOT_SUCCESS_ON_RESET,
    GENERIC_BOT_NOT_ALLOWED_BECAUSE_GUILD_SETTINGS,
    GENERIC_BOT_GOOD_SEE_DM,
    // generic from commands
    COMMAND_GENERIC_INVALID_ARGS,
    COMMAND_GENERIC_INVALID_ARGS_WITH_NAME,
    COMMAND_GENERIC_CANT_FIND,
    COMMAND_GENERIC_CANT_FIND_WITH_NAME,
    COMMAND_GENERIC_ALREADY_SET_WITH_NAME,
    COMMAND_GENERIC_ALREADY_EXISTS,
    COMMAND_GENERIC_SUCCESS_SETTING_WITH_NAME,
    COMMAND_GENERIC_SUCCESS_TAKE_TIME,
    COMMAND_GENERIC_FOUND_SWITCH_FROM_TO_NAMES,
    COMMAND_GENERIC_SUCCESS_ADD_WITH_NAME,
    COMMAND_GENERIC_SUCCESS_REMOVE_WITH_NAME,
    COMMAND_GENERIC_IN_LIST_TRUE_WITH_NAME,
    COMMAND_GENERIC_IN_LIST_FALSE_WITH_NAME,
    // botstatus.cpp
    COMMAND_BOTSTATUS_FINAL_INFOTITLE,
    COMMAND_BOTSTATUS_FINAL_TIME_ON,
    COMMAND_BOTSTATUS_FINAL_SHARD_NOW,
    COMMAND_BOTSTATUS_FINAL_SHARDS_ACTIVE,
    COMMAND_BOTSTATUS_FINAL_MAX_SHARDS,
    COMMAND_BOTSTATUS_FINAL_GUILDS_IN_SHARD,
    COMMAND_BOTSTATUS_FINAL_THREADS_BOT,
    COMMAND_BOTSTATUS_FINAL_MEMORY_USAGE_FULL,
    COMMAND_BOTSTATUS_FINAL_MEMORY_USAGE_RAM,
    COMMAND_BOTSTATUS_FINAL_USERS_IN_MEMORY,
    COMMAND_BOTSTATUS_FINAL_GUILDS_IN_MEMORY,
    COMMAND_BOTSTATUS_FINAL_DELAYED_TASKS_AMOUNT,
    COMMAND_BOTSTATUS_FINAL_DELAYED_FILES_AMOUNT,
    // configurar.cpp
    COMMAND_CONFIG_LOG_LOGS_TITLE,
    COMMAND_CONFIG_LANGUAGE_SUCCESS_TAKE_TIME_WITH_NAME,
    COMMAND_CONFIG_LEVELS_BLOCKING_TRUE,
    COMMAND_CONFIG_LEVELS_BLOCKING_FALSE,
    // copy_paste.cpp
    COMMAND_COPY_SUCCESS_SPOILER_PASTE,
    COMMAND_PASTE_CLIPBOARD_EMPTY,
    COMMAND_PASTE_FINAL_ACCESS_DIRECTLY,
    COMMAND_PASTE_FINAL_REFERENCED_BY,
    COMMAND_PASTE_FINAL_REFERENCED_SOURCE,
    COMMAND_PASTE_FINAL_REFERENCED_GUILD,
    COMMAND_PASTE_FINAL_USER_COMMENTED,
    COMMAND_PASTE_FINAL_ORIGINAL_TEXT,
    COMMAND_PASTE_FINAL_ORIGINAL_EMBED_BELOW,
    // points.cpp
    COMMAND_POINTS_MESSAGE_LEVEL_UP,
    COMMAND_POINTS_MESSAGE_LEVEL_DOWN,
    COMMAND_POINTS_MESSAGE_GLOBAL_TAG,
    COMMAND_POINTS_MESSAGE_LOCAL_TAG,
    COMMAND_POINTS_MESSAGE_LEVEL,
    COMMAND_POINTS_FINAL_GLOBAL,
    COMMAND_POINTS_FINAL_LOCAL,
    COMMAND_POINTS_FINAL_LEVEL_NOW,
    COMMAND_POINTS_FINAL_POINTS_NOW,
    COMMAND_POINTS_FINAL_NEXT_LEVEL_IN,
    // poll.cpp
    COMMAND_POLL_FINAL_TITLE_NAME,
    // statistics.cpp
    COMMAND_STATS_FINAL_TITLE_NAME,
    COMMAND_STATS_FINAL_TOTAL_MESSAGES,
    COMMAND_STATS_FINAL_MESSAGES_HERE,
    COMMAND_STATS_FINAL_MESSAGES_PERC,
    COMMAND_STATS_FINAL_TOTAL_FILES,
    COMMAND_STATS_FINAL_FILES_HERE,
    COMMAND_STATS_FINAL_FILES_PERC,
    COMMAND_STATS_FINAL_TOTAL_COMMANDS,
    // rgbcolor.cpp
    COMMAND_RGB2DECIMAL_RESULT,
    
    _MAX
};
enum class lang_command{ // 0 TO _MAX
    PING, // used
    TIME, // used
    BOTSTATUS, // used
    RGB2DECIMAL, // used
    FEEDBACK, // used
    POINTS, // used
    COPY, // used
    PASTE, // used
    STATS, // used
    POLL, // used
    ROLES, // used
    CONFIG, // used
    SELFCONF, // used
    PING_DESC, // used
    TIME_DESC, // used
        TIME_DAY_OFFSET, // used
        TIME_DAY_OFFSET_DESC, // used
        TIME_HOUR_OFFSET, // used
        TIME_HOUR_OFFSET_DESC, // used
        TIME_MINUTE_OFFSET, // used
        TIME_MINUTE_OFFSET_DESC, // used
    BOTSTATUS_DESC, // used
    RGB2DECIMAL_DESC, // used
        RGB2DECIMAL_RED, // used
        RGB2DECIMAL_RED_DESC, // used
        RGB2DECIMAL_GREEN, // used
        RGB2DECIMAL_GREEN_DESC, // used
        RGB2DECIMAL_BLUE, // used
        RGB2DECIMAL_BLUE_DESC, // used
    FEEDBACK_DESC, // used
        FEEDBACK_TEXT, // used
        FEEDBACK_TEXT_DESC, // used
    POINTS_DESC, // used
    POINTS_USER, // used
    POINTS_USER_DESC, // used
    POINTS_GLOBAL, // used
    POINTS_GLOBAL_DESC, // used
    COPY_DESC, // used
    PASTE_DESC, // used
        PASTE_TEXT, // used
        PASTE_TEXT_DESC, // used
    STATS_DESC, // used
    STATS_USER, // used
        STATS_USER_DESC, // used
    POLL_DESC, // used
    POLL_TEXT, // used
    POLL_TEXT_DESC, // used
    POLL_TITLE, // used
    POLL_TITLE_DESC, // used
    POLL_MODE, // used
    POLL_MODE_DESC, // used
        POLL_MODE_OPTION_0_DESC, // used
        POLL_MODE_OPTION_1_DESC, // used
        POLL_MODE_OPTION_2_DESC, // used
        POLL_MODE_OPTION_3_DESC, // used
        POLL_MODE_OPTION_4_DESC, // used
    POLL_EMOJIS, // used
    POLL_EMOJIS_DESC, // used
    POLL_LINK, // used
    POLL_LINK_DESC, // used
    POLL_COLOR, // used
    POLL_COLOR_DESC, // used
    ROLES_DESC, // used
    ROLES_EACH, // used
    ROLES_EACH_DESC, // used
    CONFIG_DESC, // used
    CONFIG_APPLY, // used
    CONFIG_APPLY_DESC, // used
    CONFIG_LOGS, // used
    CONFIG_LOGS_DESC, // used
    CONFIG_EXPORT, // used
    CONFIG_EXPORT_DESC, // used
    CONFIG_LANGUAGE, // used
    CONFIG_LANGUAGE_DESC, // used
        CONFIG_LANGUAGE_STRING, // used
        CONFIG_LANGUAGE_STRING_DESC, // used
    CONFIG_EXTERNAL, // used
    CONFIG_EXTERNAL_DESC, // used
        CONFIG_EXTERNAL_CANPASTE, // used
        CONFIG_EXTERNAL_CANPASTE_DESC, // used
            CONFIG_EXTERNAL_CANPASTE_ALLOW, // used
            CONFIG_EXTERNAL_CANPASTE_ALLOW_DESC, // used
    CONFIG_POLLDEFAULTS, // 
    CONFIG_POLLDEFAULTS_DESC, // 
        CONFIG_POLLDEFAULTS_EMOJIS, // 
        CONFIG_POLLDEFAULTS_EMOJIS_DESC, //
    CONFIG_ADMIN, // used
    CONFIG_ADMIN_DESC, // used
        CONFIG_ADMIN_ADD, // used
        CONFIG_ADMIN_ADD_DESC, // used
        CONFIG_ADMIN_REMOVE, // used
        CONFIG_ADMIN_REMOVE_DESC, // used
        CONFIG_ADMIN_VERIFY, // used
        CONFIG_ADMIN_VERIFY_DESC, // used
            CONFIG_ADMIN_COMMON_ROLE, // used
            CONFIG_ADMIN_COMMON_ROLE_DESC, // used
    CONFIG_POINTS, // used
    CONFIG_POINTS_DESC, // used
        CONFIG_POINTS_WHO, // used
        CONFIG_POINTS_WHO_DESC, // used
        CONFIG_POINTS_VALUE, // used
        CONFIG_POINTS_VALUE_DESC, // used
    CONFIG_ROLES, // used
    CONFIG_ROLES_DESC, // used
        CONFIG_ROLES_ADD, // used
        CONFIG_ROLES_ADD_DESC, // used
        CONFIG_ROLES_REMOVE, // used
        CONFIG_ROLES_REMOVE_DESC, // used
        CONFIG_ROLES_CLEANUP, // used
        CONFIG_ROLES_CLEANUP_DESC, // used
        CONFIG_ROLES_COMBO, // used
        CONFIG_ROLES_COMBO_DESC, // used
            CONFIG_ROLES_ADD_CATEGORY, // used
            CONFIG_ROLES_ADD_CATEGORY_DESC, // used
            CONFIG_ROLES_ADD_ROLE, // used
            CONFIG_ROLES_ADD_ROLE_DESC, // used
            CONFIG_ROLES_ADD_NAME, // used
            CONFIG_ROLES_ADD_NAME_DESC, // used
            CONFIG_ROLES_REMOVE_CATEGORY, // used
            CONFIG_ROLES_REMOVE_CATEGORY_DESC, // used
            CONFIG_ROLES_REMOVE_ROLE, // used
            CONFIG_ROLES_REMOVE_ROLE_DESC, // used
            CONFIG_ROLES_COMBO_COMBINABLE, // used
            CONFIG_ROLES_COMBO_COMBINABLE_DESC, // used
            CONFIG_ROLES_COMBO_CATEGORY, // used
            CONFIG_ROLES_COMBO_CATEGORY_DESC, // used
    CONFIG_AUTOROLE, // used
    CONFIG_AUTOROLE_DESC, // used
        CONFIG_AUTOROLE_ADD, // used
        CONFIG_AUTOROLE_ADD_DESC, // used
        CONFIG_AUTOROLE_REMOVE, // used
        CONFIG_AUTOROLE_REMOVE_DESC, // used
        CONFIG_AUTOROLE_VERIFY, // used
        CONFIG_AUTOROLE_VERIFY_DESC, // used
            CONFIG_AUTOROLE_COMMON_ROLE, // used
            CONFIG_AUTOROLE_COMMON_ROLE_DESC, // used
    CONFIG_LEVELS, // used
    CONFIG_LEVELS_DESC, // used
        CONFIG_LEVELS_ADD, // used
        CONFIG_LEVELS_ADD_DESC, // used
        CONFIG_LEVELS_REMOVE, // used
        CONFIG_LEVELS_REMOVE_DESC, // used
        CONFIG_LEVELS_REDIRECT,
        CONFIG_LEVELS_REDIRECT_DESC,
            CONFIG_LEVELS_REDIRECT_CHANNELID,
            CONFIG_LEVELS_REDIRECT_CHANNELID_DESC,
        CONFIG_LEVELS_MESSAGES, // used
        CONFIG_LEVELS_MESSAGES_DESC, // used
            CONFIG_LEVELS_ADD_ROLE, // used
            CONFIG_LEVELS_ADD_ROLE_DESC, // used
            CONFIG_LEVELS_ADD_LEVEL, // used
            CONFIG_LEVELS_ADD_LEVEL_DESC, // used
            CONFIG_LEVELS_REMOVE_ROLE, // used
            CONFIG_LEVELS_REMOVE_ROLE_DESC, // used
            CONFIG_LEVELS_REMOVE_LEVEL, // used
            CONFIG_LEVELS_REMOVE_LEVEL_DESC, // used
            CONFIG_LEVELS_MESSAGES_BLOCK, // used
            CONFIG_LEVELS_MESSAGES_BLOCK_DESC, // used
    SELFCONF_DESC, // used
        SELFCONF_LEVELNOTIF, // used
        SELFCONF_LEVELNOTIF_DESC, // used
            SELFCONF_LEVELNOTIF_ENABLE, // used
            SELFCONF_LEVELNOTIF_ENABLE_DESC, // used
        SELFCONF_COLOR, // used
        SELFCONF_COLOR_DESC, // used
            SELFCONF_COLOR_VALUE, // used
            SELFCONF_COLOR_VALUE_DESC, // used
        SELFCONF_DATA,
        SELFCONF_DATA_DESC,
    _MAX
};

std::unordered_map<std::string, lang_command> __conversion_lang_command_gen(); // boot
std::unordered_map<std::string, lang_line> __conversion_lang_line_gen(); // boot

const inline std::unordered_map<std::string, lang_command> __i_conversion_lang_command = __conversion_lang_command_gen();
const inline std::unordered_map<std::string, lang_line> __i_conversion_lang_line = __conversion_lang_line_gen();

void replaceargformat(std::string&, std::initializer_list<std::string>); // "string {0} {1} ..."
std::string replaceargformatdirect(std::string, std::initializer_list<std::string>); // "string {0} {1} ..."
lang_command strtolangcmd(const std::string&);
lang_line strtolangline(const std::string&);
std::string langcmdtostr(const lang_command&);
std::string langlinetostr(const lang_line&);

std::string checksum_lang_line(const std::unordered_map<lang_line, std::string>&); // if string not empty, error
std::string checksum_lang_command(const std::unordered_map<lang_command, std::string>&); // if string not empty, error

class EachLang {
    const std::unordered_map<lang_line, std::string>& ref_lines;
    const std::unordered_map<lang_command, std::string>& ref_cmds;
public:
    EachLang(const std::unordered_map<lang_line, std::string>&, const std::unordered_map<lang_command, std::string>&);

    const std::string& get(const lang_line&) const;
    const std::string& get(const lang_command&) const;

    const std::unordered_map<lang_command, std::string>& get_commands() const;

    lang_line line_from(const std::string&) const;
    lang_command command_from(const std::string&, const std::initializer_list<lang_command>&) const;
};

class LanguageControl {
    std::unordered_map<std::string, std::unordered_map<lang_line, std::string>> language_lines;
    std::unordered_map<std::string, std::unordered_map<lang_command, std::string>> language_commands;
    std::string language_fallback = "en.US";
    mutable std::mutex control;
public:
    //LanguageControl();

    const std::vector<std::string> language_list() const;
    std::shared_ptr<EachLang> get_lang(const std::string&) const;

    bool try_reload();
};

inline LanguageControl langctrl;

std::shared_ptr<EachLang> get_lang(const std::string&); // direct call