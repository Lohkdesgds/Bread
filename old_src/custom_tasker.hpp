#pragma once

#include <thread>
#include <mutex>
#include <functional>
#include <vector>
#include <memory>

#include "tools.hpp"

const std::chrono::seconds default_tasker_time = std::chrono::seconds(12);
constexpr double threshold_reached_tasker_time_prop = 0.75;
constexpr double threshold_reached_double_tasker_time_prop = 0.4;
constexpr size_t threshold_tasker_amount = 100;
constexpr size_t threshold_tasker_amount_double = 200;

class DelayedTasker {
    std::thread work;
    std::vector<std::function<bool()>> tasks;
    std::mutex tasks_m;
    const std::chrono::seconds delay_each;
    bool keep_working = false;
    bool indeed_running = false;

    void keep_tasking_so();
public:
    DelayedTasker(const std::chrono::seconds);
    ~DelayedTasker();

    // this function MUST return TRUE when done (keeps being called if false). Exceptions clear too
    void push_back(const std::function<bool()>&);
    // automatic build call from A to B. Exception cancel run
    void push_back(const std::function<void(const size_t)>&, const size_t, const size_t);

    size_t remaining() const;

    // only when closing app
    void destroy(const bool = true);
};

inline DelayedTasker __generic_tasker(default_tasker_time);

DelayedTasker& get_default_tasker();