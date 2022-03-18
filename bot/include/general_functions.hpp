#pragma once

#include <dpp/dpp.h>
#include <dpp/nlohmann/json.hpp>
#include <dpp/fmt/format.h>
#include <console.h>
#include <bomb.h>

#include <general_config.hpp>
#include <safe_template.hpp>
#include <slashing.hpp>
#include <defaults.hpp>
#include <timed_factory.hpp>
#include <user_info.hpp>

#include <functional>

using namespace Lunaris;

enum class commands{ NONE = -1, HELP, EXIT, SETSTATUSSTR, SETSTATUSMODE, SETSTATUSCODE, SETSTATUSLINK, REQUESTGUILDLIST, REQUESTGUILDSNOWFLAKE, REQUESTUSERSNOWFLAKE, RESETSLASHCOMMANDS, MEMSTATUS };

/// = = = = = timed/functional functions = = = = = ///
void g_on_log(const dpp::log_t&);
void g_on_ready(const dpp::ready_t&, safe_data<slash_global>&);
void g_on_modal(const dpp::form_submit_t&);
void g_tick_presence(const safe_data<general_config>&, dpp::cluster&);
void g_apply_guild_local_commands(dpp::cluster&, const safe_data<std::vector<slash_local>>&);
void input_handler_cmd(dpp::cluster&, bool&, safe_data<general_config>&, safe_data<std::vector<slash_local>>&, const safe_data<slash_global>&, const std::string&, const timed_factory<dpp::snowflake, user_info>&);

/// = = = = = useful user-like functions = = = = = ///
void lock_indefinitely();
// command raw, the arguments only are set on second arg
commands g_interp_cmd(const std::string&, std::string&);
std::string g_transl_nsfw_code(const dpp::guild_nsfw_level_t&);
std::string g_smash_guild_info(const dpp::guild&);

/// = = = = = setup bot functions = = = = = ///
void setup_bot(dpp::cluster&, safe_data<slash_global>&, timed_factory<dpp::snowflake, user_info>&);

/// = = = = = assist funcs = = = = = ///
std::unique_ptr<dpp::cluster> build_bot_from(safe_data<general_config>&);
template<typename T, typename K> bool find_json_autoabort(const nlohmann::json&, const T&, K&, std::function<void(const std::exception&)> = {});
template<typename T, typename K> bool find_json_array_autoabort(const nlohmann::json&, const T&, K&, std::function<void(const std::exception&)> = {});

#include <impl/general_functions.ipp>