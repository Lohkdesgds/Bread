#pragma once

#include <dpp/dpp.h>
#include <dpp/nlohmann/json.hpp>
#include <dpp/fmt/format.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../tools.hpp"
#include "../user_specific.hpp"
#include "../guild_specific.hpp"
#include "../language.hpp"

const std::string statistics_image_url = "https://media.discordapp.net/attachments/739704685505544363/740787679364382790/cadeado_key.png";

void __handle_message_statistics(dpp::cluster&, const dpp::message_create_t&);
void __handle_command_statistics_any(dpp::cluster&, const dpp::interaction_create_t&, dpp::command_interaction&);
void __handle_command_statistics(dpp::cluster&, const dpp::interaction_create_t&, dpp::command_interaction&, std::shared_ptr<EachLang>);