#pragma once

#include <dpp/dpp.h>
#include <dpp/nlohmann/json.hpp>
#include <dpp/fmt/format.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <sstream>
#include <string>

#include "../tools.hpp"
#include "../user_specific.hpp"
#include "../guild_specific.hpp"
#include "../language.hpp"


void __handle_guild_member_update(dpp::cluster&, const dpp::guild_member_update_t&);
void __handle_guild_member_update_message(dpp::cluster&, const dpp::message_create_t&);
void __handle_command_tags(dpp::cluster&, const dpp::interaction_create_t&, dpp::command_interaction&, std::shared_ptr<EachLang>);

// common
void __handle_guild_member_tags(dpp::cluster&, dpp::guild_member&);