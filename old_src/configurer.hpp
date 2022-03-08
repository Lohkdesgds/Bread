#pragma once

#include <dpp/dpp.h>
#include <dpp/nlohmann/json.hpp>
#include <dpp/fmt/format.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <mutex>
#include <iostream>
#include <functional>

#include "secure_control.hpp"

class ConfigSetting {
    nlohmann::json json;
    mutable std::mutex mu;
    std::string path;
public:
    bool open(const std::string&);
    bool flush() const;
    void do_safe(const std::function<void(nlohmann::json&)>&);

    std::string         qgs(const std::string&);
    unsigned long long  qgl(const std::string&);
    int64_t             qgi(const std::string&);
};