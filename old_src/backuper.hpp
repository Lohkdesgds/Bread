#pragma once

#include <dpp/dpp.h>
#include <dpp/nlohmann/json.hpp>
#include <dpp/fmt/format.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <iostream>
#include <mutex>

#include "tools.hpp"
#include "secure_control.hpp"
#include "custom_tasker.hpp"

#include "memory_reference_manager.hpp"

const mull guild_backup_id    = 847342829285933057;//739198747069644921;
const mull channel_backup_id  = 891422972450054194;//889173078813319259;
const mull channel_logging_jl = 891424429857468517; // log join and leave channel
const size_t max_message_backup_size = 1800;
const size_t max_message_file_size = (1024 * 1024 * 8) - 1;
const std::string file_alt_way = "_alt";
const std::chrono::seconds tasker_jsoner_time = std::chrono::seconds(60);

inline DelayedTasker __flush_to_guild_tasker(tasker_jsoner_time);

nlohmann::json get_from_file(const std::string& path, const std::string& name, const std::string& extension);
// name should not be bigger than max_message_backup_size
bool save_file(dpp::cluster& core, const nlohmann::json& j, const std::string& path, const std::string& name, const std::string& extension);

void log_joined_new_guild(dpp::cluster&, const dpp::guild_create_t&);
void log_updated_guild(dpp::cluster&, const dpp::guild_create_t&);
void log_left_a_guild(dpp::cluster&, const dpp::guild_delete_t&);

DelayedTasker& get_file_tasker();
size_t get_saving_buffer();