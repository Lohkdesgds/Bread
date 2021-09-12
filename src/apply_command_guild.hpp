#pragma once

#include <dpp/dpp.h>
#include <dpp/nlohmann/json.hpp>
#include <dpp/fmt/format.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <memory>

#include "tools.hpp"
#include "language.hpp"
#include "guild_specific.hpp"
#include "custom_tasker.hpp"
//#include "tags.hpp"

const unsigned commands_version_val = 1;

const size_t commands_amount = 10;
const int max_err = 5;

struct _dummy_commands{
    std::pair<std::string, dpp::slashcommand> cmds[commands_amount];
};

std::string language_based_on_location(const dpp::guild&);

void __apply_commands_at_guild(dpp::cluster&, const mull, dpp::guild*);