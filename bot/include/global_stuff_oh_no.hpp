#pragma once

#include <timed_factory.hpp>
#include <user_info.hpp>
#include <guild_info.hpp>

inline timed_factory<dpp::snowflake, user_info> tf_user_info;
inline timed_factory<dpp::snowflake, guild_info> tf_guild_info;