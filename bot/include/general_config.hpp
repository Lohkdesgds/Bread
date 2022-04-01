#pragma once

#include <dpp/dpp.h>
#include <dpp/nlohmann/json.hpp>
#include <dpp/fmt/format.h>
#include <Lunaris-Console/console.h>
#include <Lunaris-Bomb/bomb.h>
#include <Lunaris-Mutex/mutex.h>

struct general_config {
    std::string token;
    std::string status_text = "Bread 2.0BWW Beta";
    std::string status_link;
    uint32_t shard_count = 0;
    uint32_t intents = 0;
    uint32_t status_code = static_cast<uint32_t>(dpp::at_game); // streaming, listening
    uint32_t status_mode = static_cast<uint32_t>(dpp::ps_idle); // dnd, online, afk, offline

    bool load_from(const std::string&);
    bool save_as(const std::string&);
};