#pragma once

#include <dpp/dpp.h>
#include <dpp/nlohmann/json.hpp>
#include <dpp/fmt/format.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../tools.hpp"
#include "../language.hpp"

const std::string ping_start_text_content = "||#PING|";

void __handle_message_ping(dpp::cluster&, const dpp::message_create_t&, std::shared_ptr<EachLang>);
void __handle_command_ping(dpp::cluster&, const dpp::interaction_create_t&, dpp::command_interaction&, std::shared_ptr<EachLang>);