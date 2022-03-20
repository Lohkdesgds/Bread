#pragma once

#include <dpp/dpp.h>
#include <dpp/nlohmann/json.hpp>
#include <dpp/fmt/format.h>
#include <console.h>
#include <bomb.h>

#include <bot.hpp>

#include <functional>

using namespace Lunaris;

enum class commands{ NONE = -1, HELP, EXIT, SETSTATUSSTR, SETSTATUSMODE, SETSTATUSCODE, SETSTATUSLINK, REQUESTGUILDLIST, REQUESTGUILDSNOWFLAKE, REQUESTUSERSNOWFLAKE, RESETSLASHCOMMANDS, MEMSTATUS };
enum class discord_commands{ BOTSTATUS, CONFIGURATION, PASTE, PING, POINTS, POLL, RGBCONVERT, ROLES, SELF, STATS, TIME };

const std::string confirm_emojis[] = {"⛔", "✅"};

/// = = = = = timed/functional functions = = = = = ///
void g_on_log(const dpp::log_t&);
void g_on_ready(const dpp::ready_t&, safe_data<slash_global>&);
void g_on_modal(const dpp::form_submit_t&);
void g_on_button_click(const dpp::button_click_t&);
void g_on_select(const dpp::select_click_t&);
void g_on_interaction(const dpp::interaction_create_t&);
void g_tick_presence(const safe_data<general_config>&, dpp::cluster&);
void g_apply_guild_local_commands(dpp::cluster&, const safe_data<std::vector<slash_local>>&);
void input_handler_cmd(dpp::cluster&, bool&, safe_data<general_config>&, safe_data<std::vector<slash_local>>&, const safe_data<slash_global>&, const std::string&);

/// = = = = = useful user-like functions = = = = = ///
void lock_indefinitely();
// command raw, the arguments only are set on second arg
commands g_interp_cmd(const std::string&, std::string&);
std::string g_transl_nsfw_code(const dpp::guild_nsfw_level_t&);
std::string g_smash_guild_info(const dpp::guild&);

/// = = = = = setup bot functions = = = = = ///
void setup_bot(dpp::cluster&, safe_data<slash_global>&);

/// = = = = = assist funcs = = = = = ///
void error_autoprint(const dpp::confirmation_callback_t&);
std::unique_ptr<dpp::cluster> build_bot_from(safe_data<general_config>&);
template<typename T, typename K> bool find_json_autoabort(const nlohmann::json&, const T&, K&, std::function<void(const std::exception&)> = {});
template<typename T, typename K> bool find_json_array_autoabort(const nlohmann::json&, const T&, K&, std::function<void(const std::exception&)> = {});
bool change_component(std::vector<dpp::component>&, const std::string&, std::function<void(dpp::component&)>);
// set emoji and type
dpp::component make_boolean_button(const bool);
dpp::component& set_boolean_button(const bool, dpp::component&);
bool auto_handle_button_switch(const dpp::interaction_create_t&, const std::string&, std::function<void(dpp::component&)>);
int64_t interpret_color(const std::string&);
std::string print_hex(const int64_t);

#include <impl/general_functions.ipp>