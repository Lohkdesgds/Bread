#include "secure_control.hpp"

SafeFileControl::SafeFileControl(atomic_ull& a)
    : ref(a)
{
    ref.plus();
}

SafeFileControl::~SafeFileControl()
{
    if (count) ref.minus();
}

SafeFileControl::SafeFileControl(SafeFileControl&& oth)
    : ref(oth.ref)
{
    if (!oth.count) throw std::runtime_error("invalid double std::move() of SafeFileControl");
    oth.count = false;
}

unsigned long long SafeFileControl::current_amount() const
{
    return ref.read();
}

SafeFileControl TotalControl::get_lock()
{
    if (fail_throw_always) throw std::runtime_error("Locking is blocked");
    return {files_open};
}

void TotalControl::lock_new() // turning off
{
    fail_throw_always = true;
}

void TotalControl::unlock_new() // back on?
{
    fail_throw_always = false;
}

SafeFileControl get_lock_file()
{
    return gtotctrl.get_lock();
}