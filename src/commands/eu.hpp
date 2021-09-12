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
#include "../language.hpp"

void __handle_command_eu(dpp::cluster&, const dpp::interaction_create_t&, dpp::command_interaction&, std::shared_ptr<EachLang>);