#pragma once

#include <dpp/dpp.h>
#include <dpp/nlohmann/json.hpp>
#include <dpp/fmt/format.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../tools.hpp"
#include "../language.hpp"
#include "../user_specific.hpp"
#include "../guild_specific.hpp"
#include "../custom_tasker.hpp"
#include "../jsoner.hpp"

const std::string botstatus_image_url = "https://media.discordapp.net/attachments/739704685505544363/740803216626679808/personal_computer.png?width=468&height=468";
const std::string botstatus_image_dpp_url = "https://cdn.discordapp.com/attachments/739704685505544363/886048618711244851/dpp_image.png?size=256";

void __handle_command_botstatus(dpp::cluster&, const dpp::interaction_create_t&, dpp::command_interaction&, std::shared_ptr<EachLang>);