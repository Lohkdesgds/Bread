#pragma once

#include <dpp/dpp.h>
#include <dpp/nlohmann/json.hpp>
#include <dpp/fmt/format.h>
#include <console.h>
#include <bomb.h>

using namespace Lunaris;

class slash_global {
    std::vector<dpp::slashcommand> valid_commands;
    dpp::snowflake default_bot_id = 0;
public:
    bool load_from(const std::string&);

    size_t size() const;

    dpp::slashcommand& index(size_t);
    const dpp::slashcommand& index(size_t) const;

    void update_bot_id(const dpp::cluster&);

    void remove_global(dpp::cluster&) const;
    void apply_bulk(dpp::cluster&) const;
};

// todo: custom slash command
class slash_local {
    dpp::snowflake to_guild = 0;

};