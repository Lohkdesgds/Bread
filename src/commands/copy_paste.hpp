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

const std::string url_author_icon_clipboard = "https://media.discordapp.net/attachments/739704685505544363/872539023057489920/folder_icon.png";

void __handle_command_clipboard_copy(dpp::cluster&, const dpp::interaction_create_t&, dpp::command_interaction&, std::shared_ptr<EachLang>);
void __handle_command_clipboard_paste(dpp::cluster&, const dpp::interaction_create_t&, dpp::command_interaction&, std::shared_ptr<EachLang>);
