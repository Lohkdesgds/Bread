#pragma once

#include <dpp/dpp.h>
#include <dpp/nlohmann/json.hpp>
#include <dpp/fmt/format.h>
#include <Lunaris-Console/console.h>
#include <Lunaris-Bomb/bomb.h>
#include <Lunaris-Mutex/mutex.h>

#include <defaults.hpp>

#include <memory>
#include <chrono>
#include <unordered_map>
#include <shared_mutex>

template<typename Key, typename VarType>
class timed_factory {
    struct mem {
        const std::chrono::seconds& ref_delta;
        decltype(std::chrono::system_clock::now().time_since_epoch()) dt;
        std::shared_ptr<VarType> var;

        mem(const std::chrono::seconds&);
        mem(mem&&);
        mem(const mem&) = delete;

        bool should_del() const;
        void update_time();
    };

    std::unordered_map<Key, mem> objs;
    mutable std::shared_mutex shr;
    std::chrono::seconds delta_for_kill = factory_default_time;
public:
    std::shared_ptr<VarType> operator[](const Key&);
    std::shared_ptr<VarType> operator[](const Key&) const;

    // force free one
    void free(const Key&);

    // free the old guys
    void free_freeable();

    // force clear all references
    void free_all();

    // inactive for how long to free stuff on free_freeable()?
    void set_free_time(std::chrono::seconds);

    size_t size() const;
};

#include <impl/timed_factory.ipp>