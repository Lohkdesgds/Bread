#pragma once

#include <dpp/dpp.h>
#include <dpp/nlohmann/json.hpp>
#include <dpp/fmt/format.h>
#include <console.h>
#include <bomb.h>

#include <general_config.hpp>
#include <safe_template.hpp>
#include <slashing.hpp>

using namespace Lunaris;

enum class commands{ NONE = -1, HELP, EXIT, SETSTATUSSTR, SETSTATUSMODE, SETSTATUSCODE, SETSTATUSLINK, REQUESTGUILDLIST, REQUESTGUILDSNOWFLAKE, REQUESTUSERSNOWFLAKE, RESETSLASHCOMMANDS };

/// = = = = = timed/functional functions = = = = = ///
void g_on_log(const dpp::log_t&);
void g_on_ready(const dpp::ready_t&, slash_global&);
void g_tick_presence(const general_config&, dpp::cluster&);
void g_apply_guild_local_commands(dpp::cluster&, const safe_of<std::vector<slash_local>>&);
void input_handler_cmd(dpp::cluster&, bool&, safe_of<general_config>&, safe_of<std::vector<slash_local>>&, const slash_global&, const std::string&);

/// = = = = = useful user-like functions = = = = = ///
void lock_indefinitely();
// command raw, the arguments only are set on second arg
commands g_interp_cmd(const std::string&, std::string&);
std::string g_transl_nsfw_code(const dpp::guild_nsfw_level_t&);
std::string g_smash_guild_info(const dpp::guild&);

/// = = = = = setup bot functions = = = = = ///
void setup_bot(dpp::cluster&, slash_global&);