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
#include "../apply_command_guild.hpp"
#include "tags.hpp"

//void __handle_guildcreate_configurar(dpp::cluster&, const dpp::guild_create_t&);
void __handle_command_configurar(dpp::cluster&, const dpp::interaction_create_t&, dpp::command_interaction&, std::shared_ptr<EachLang>);