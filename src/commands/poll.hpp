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

const std::string poll_image_url = "https://media.discordapp.net/attachments/739704685505544363/740437613755760650/POLL_img.png";
const std::string poll_emoji_delete_reactions = u8"♻️";
const std::string poll_emoji_delete_message = u8"❌";

void __handle_reaction_poll(dpp::cluster&, const dpp::message_reaction_add_t&, std::shared_ptr<EachLang>);
void __handle_command_poll(dpp::cluster&, const dpp::interaction_create_t&, dpp::command_interaction&, std::shared_ptr<EachLang>);