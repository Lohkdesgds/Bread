#pragma once

#include <dpp/dpp.h>
#include <dpp/nlohmann/json.hpp>
#include <dpp/fmt/format.h>
#include <Lunaris-Console/console.h>
#include <Lunaris-Bomb/bomb.h>
#include <Lunaris-Mutex/mutex.h>

#include <defaults.hpp>
#include <process_info.hpp>
#include <general_config.hpp>
#include <safe_template.hpp>
#include <slashing.hpp>
#include <timed_factory.hpp>
#include <general_functions.hpp>
#include <user_info.hpp>
#include <guild_info.hpp>
#include <global_stuff_oh_no.hpp>

#include <functional>

using namespace Lunaris;

enum class commands{ NONE = -1, HELP, SAVECONF, EXIT, SETSTATUSSTR, SETSTATUSMODE, SETSTATUSCODE, SETSTATUSLINK, REQUESTGUILDLIST, REQUESTGUILDSNOWFLAKE, REQUESTUSERSNOWFLAKE, RESETSLASHCOMMANDS, DELETEGLOBALSLASH, MEMSTATUS };
enum class discord_slashcommands{ UNKNOWN = -1, BOTSTATUS, CONFIGURATION, PASTE, PING, POLL, ROLES, SELF, USERINFO, RC_SHOWINFO, RC_COPY };
enum class roleguild_tasks{GROUP_ADD, GROUP_REMOVE, ROLE_ADD, ROLE_REMOVE, UPDATE, SELECTUPDATE};

const std::string confirm_emojis[] = {"⛔", "✅"};
//const std::string navigation_emojis[] = {"⏮️", "🔎", "⏭️"};


/// = = = = = timed/functional functions = = = = = ///
void g_on_react(const dpp::message_reaction_add_t&);
void g_on_log(const dpp::log_t&);
void g_on_new_guild(dpp::cluster&, const dpp::guild_create_t&);
void g_on_left_guild(dpp::cluster&, const dpp::guild_delete_t&);
void g_on_message(const dpp::message_create_t&);
void g_on_ready(const dpp::ready_t&, safe_data<slash_global>&);
void g_on_modal(const dpp::form_submit_t&);
void g_on_button_click(const dpp::button_click_t&);
void g_on_select(const dpp::select_click_t&);
void g_on_interaction(const dpp::interaction_create_t&);
void g_tick_presence(const safe_data<general_config>&, dpp::cluster&);
//void g_apply_guild_local_commands(dpp::cluster&, const safe_data<std::vector<slash_local>>&);
void input_handler_cmd(dpp::cluster&, bool&, safe_data<general_config>&, /*safe_data<std::vector<slash_local>>&,*/ const safe_data<slash_global>&, const std::string&);

/// = = = = = useful user-like functions = = = = = ///
// command raw, the arguments only are set on second arg
commands g_interp_cmd(const std::string&, std::string&);
std::string g_transl_nsfw_code(const dpp::guild_nsfw_level_t&);
std::string g_smash_guild_info(const dpp::guild&);

/// = = = = = setup bot functions = = = = = ///
void setup_bot(dpp::cluster&, safe_data<slash_global>&);

/// = = = = = assist funcs = = = = = ///
void error_autoprint(const dpp::confirmation_callback_t&);
std::unique_ptr<dpp::cluster> build_bot_from(safe_data<general_config>&);
bool change_component(std::vector<dpp::component>&, const std::string&, std::function<void(dpp::component&)>);
std::string get_label(const std::vector<dpp::component>&, const std::string&);
// set emoji and type
dpp::component make_boolean_button(const bool);
dpp::component& set_boolean_button(const bool, dpp::component&);
dpp::message make_ephemeral_message(const std::string& = {});
std::string generate_guild_listing_role(const std::shared_ptr<guild_info>& guil, std::string& back_select, size_t& back_vec_len, size_t& selected, const std::string& search_for);
// roleguild_auto_do can generate a new message if dpp::message& is empty-ish. It updates whatever is there.
//dpp::message roleguild_auto_do(const std::shared_ptr<guild_info>&, const dpp::message&, const roleguild_tasks, std::variant<std::monostate, guild_info::pair_id_name, std::string>);
//dpp::component make_selectable_list(const std::string&, const size_t, std::vector<dpp::select_option>);
//dpp::component& update_selectable_list(dpp::component&, const size_t, std::vector<dpp::select_option>);
bool auto_handle_button_switch(const dpp::interaction_create_t&, const std::string&, std::function<void(dpp::component&)>);
int64_t interpret_color(const std::string&);
std::string print_hex_color_auto(const int64_t);
discord_slashcommands slash_to_discord_command(const std::string&);
bool is_member_admin(const dpp::guild_member&);
// user pts, current level, points needed for next
void calc_user_level(const unsigned long long, unsigned long long&, unsigned long long&);

/// = = = = = specific commands (returns true if any response is given, not success!) = = = = = ///
bool run_botstatus(const dpp::interaction_create_t&, const dpp::command_interaction&);
bool run_self(const dpp::interaction_create_t&);
bool run_poll(const dpp::interaction_create_t&, const dpp::command_interaction&);
bool run_ping(const dpp::interaction_create_t&);
bool run_config_server(const dpp::interaction_create_t&, const dpp::command_interaction&);
bool run_copy(const dpp::interaction_create_t&, const dpp::command_interaction&);
bool run_paste(const dpp::interaction_create_t&, const dpp::command_interaction&);
bool run_showinfo(const dpp::interaction_create_t&);
bool run_roles(const dpp::interaction_create_t&);