#pragma once

#include <stdexcept>
#include <atomic>
#include "tools.hpp"

class SafeFileControl {
    std::atomic<mull>& ref;
    bool count = true; // if moved, false
public:
    SafeFileControl(std::atomic<mull>&);
    ~SafeFileControl();

    SafeFileControl(SafeFileControl&&);

    unsigned long long current_amount() const;

    SafeFileControl(const SafeFileControl&) = delete;
    void operator=(const SafeFileControl&) = delete;
    void operator=(SafeFileControl&&) = delete;
};

class TotalControl {
    std::atomic<mull> files_open;
    bool fail_throw_always = false;
public:
    SafeFileControl get_lock();

    void lock_new(); // turning off
    void unlock_new(); // back on?
};

inline TotalControl gtotctrl;

SafeFileControl get_lock_file();