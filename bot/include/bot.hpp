#pragma once

#include <dpp/dpp.h>
#include <dpp/nlohmann/json.hpp>
#include <dpp/fmt/format.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>

#include <console.h>
#include <bomb.h>

#include <defaults.hpp>
#include <general_config.hpp>
#include <safe_template.hpp>
#include <slashing.hpp>
#include <defaults.hpp>
#include <timed_factory.hpp>
#include <user_info.hpp>
#include <general_functions.hpp>

inline timed_factory<dpp::snowflake, user_info> tf_user_info;