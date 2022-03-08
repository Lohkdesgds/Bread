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

const unsigned commands_version_val = 5;

const size_t commands_amount = 13;
const int max_err = 5;

struct _dummy_commands{
    struct commands_rules {
        dpp::slashcommand cmd;
        bool enabled = true;
    };
    std::pair<std::string, commands_rules> cmds[commands_amount];
};

// deprecated
std::string language_based_on_location(const dpp::guild&);

void __apply_commands_at_guild(dpp::cluster&, const mull, dpp::guild*);