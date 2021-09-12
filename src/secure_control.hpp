#pragma once

#include <stdexcept>
#include "tools.hpp"

class SafeFileControl {
    atomic_ull& ref;
    bool count = true; // if moved, false
public:
    SafeFileControl(atomic_ull&);
    ~SafeFileControl();

    SafeFileControl(SafeFileControl&&);

    unsigned long long current_amount() const;

    SafeFileControl(const SafeFileControl&) = delete;
    void operator=(const SafeFileControl&) = delete;
    void operator=(SafeFileControl&&) = delete;
};

class TotalControl {
    atomic_ull files_open;
    bool fail_throw_always = false;
public:
    SafeFileControl get_lock();

    void lock_new(); // turning off
    void unlock_new(); // back on?
};

inline TotalControl gtotctrl;

SafeFileControl get_lock_file();