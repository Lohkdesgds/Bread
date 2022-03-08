#pragma once

#include <mutex>

// quick easy simple base for any thread-sensitive stuff
// changing this file and other things in the future, please don't look at this thing
template<typename T>
struct safe_of {
    T obj;
    mutable std::mutex obj_mu;
};