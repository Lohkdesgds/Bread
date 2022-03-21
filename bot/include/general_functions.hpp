#pragma once

#include <dpp/dpp.h>
#include <dpp/nlohmann/json.hpp>
#include <dpp/fmt/format.h>
#include <console.h>
#include <bomb.h>

void lock_indefinitely();
template<typename T, typename K> 
bool find_json_autoabort(const nlohmann::json&, const T&, K&, std::function<void(const std::exception&)> = {});
template<typename J, typename T, typename K> 
bool find_json_array_autoabort(const nlohmann::json&, const T&, K&, std::function<void(const std::exception&)> = {});

std::vector<dpp::snowflake> slice_string_auto_snowflake(const std::string&);

#include <impl/general_functions.ipp>