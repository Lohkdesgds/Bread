#pragma once

#include <dpp/dpp.h>
#include <dpp/nlohmann/json.hpp>
#include <dpp/fmt/format.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <iostream>

#include "bot_reference_so_bad.hpp"
#include "tools.hpp"
#include "configurer.hpp"
#include "guild_specific.hpp"
#include "user_specific.hpp"
#include "apply_command_guild.hpp"

#include "commands/points.hpp"
#include "commands/tiem.hpp"
#include "commands/statistics.hpp"
#include "commands/copy_paste.hpp"
#include "commands/poll.hpp"
#include "commands/tags.hpp"
#include "commands/configurar.hpp"
#include "commands/eu.hpp"
#include "commands/ping.hpp"
#include "commands/botstatus.hpp"
#include "commands/feedback.hpp"
#include "commands/rgbcolor.hpp"

/// DEFAULTS
constexpr int default_app_intents = dpp::i_guilds | dpp::i_guild_messages | dpp::i_guild_members | dpp::i_guild_bans | dpp::i_guild_message_reactions;
constexpr int default_shard_count = 0;

/// FUNCTIONS

// generic constant
void set_presence(dpp::cluster&, ConfigSetting&);

// main call
void setup_bot(ConfigSetting&, bool&);

// generic hooks
void _hook_ev_ready(dpp::cluster&, ConfigSetting&, const dpp::ready_t&);
void _hook_ev_guild_member_update(dpp::cluster&, ConfigSetting&, const dpp::guild_member_update_t&);
void _hook_ev_interaction(dpp::cluster&, ConfigSetting&, const dpp::interaction_create_t&);
void _hook_ev_message_create(dpp::cluster&, ConfigSetting&, const dpp::message_create_t&);
void _hook_ev_guild_create(dpp::cluster&, ConfigSetting&, const dpp::guild_create_t&);
void _hook_ev_guild_delete(dpp::cluster&, ConfigSetting&, const dpp::guild_delete_t&);
void _hook_ev_message_reaction_add(dpp::cluster&, ConfigSetting&, const dpp::message_reaction_add_t&);

// generic things
void _fix_json_missing_vals(nlohmann::json&);
void _fix_set_slash_commands(dpp::cluster&, ConfigSetting&);