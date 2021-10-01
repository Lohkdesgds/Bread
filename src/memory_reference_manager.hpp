#pragma once

#include <dpp/dpp.h>
#include <dpp/nlohmann/json.hpp>
#include <dpp/fmt/format.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <memory>
#include <thread>
#include <shared_mutex>
#include <unordered_map>

#include "tools.hpp"

const std::chrono::seconds time_each = std::chrono::seconds(3);
constexpr size_t time_check_references = 30; // time_check_references * time_each = total time per possible memory flush.
constexpr mull min_living_time = 60 * 60 * 1000; // ms
constexpr mull time_flush_always = 60 * 60 * 1000; // 1 hour
constexpr size_t memory_reference_flush_max_overload_control = 30; // up to 50 files per try

template<typename T>
class ComplexSharedPtr{
    using shared_unique = std::shared_ptr<std::unique_ptr<T>>;
    shared_unique ptr;
public:
    ComplexSharedPtr() = default;
    ComplexSharedPtr(const ComplexSharedPtr&);
    ComplexSharedPtr(ComplexSharedPtr&&);
    ComplexSharedPtr(std::unique_ptr<T>&&);
    ComplexSharedPtr(const shared_unique&);
    ComplexSharedPtr(shared_unique&&);
    void operator=(const ComplexSharedPtr&);
    void operator=(ComplexSharedPtr&&);
    void operator=(std::unique_ptr<T>&&);
    void operator=(const shared_unique&);
    void operator=(shared_unique&&);

    T* const operator->() const;
    const T& operator*() const;
    T& get_reference_edit();

    operator bool() const;
    size_t use_count() const;

    void destroy();
};

template<typename T, typename... Args>
ComplexSharedPtr<T> make_complex_ptr(Args&&...);

template <typename T>
class MemoryReferenceManager {
    struct block_data {
        ComplexSharedPtr<T> ref;
        mull when_ms = 0; // when to kill (get_time_ms() + min_living_time on create/update)
        mull last_flush = 0; // if when_ms take too long, this will flush sometimes
    };
    std::thread silence_flush;
    std::shared_mutex smu;
    std::unordered_map<mull, block_data> umap;
    const std::string nameit;
    bool keep_working = false;
    bool is_running = false;

    const std::function<T(const mull&)> generator;
    const std::function<void(T&)> save_func;

    void keep_flush();

    std::unique_lock<std::shared_mutex> get_lock(const std::string& = "");
    std::shared_lock<std::shared_mutex> get_lock_shared(const std::string& = "");
    bool get_nolock(const mull, ComplexSharedPtr<T>&);

    std::string genid() const;
public:
    MemoryReferenceManager(const std::function<T(const mull&)>, const std::function<void(T&)>, const std::string&);
    ~MemoryReferenceManager();

    // lock?
    void stop(const bool = true);
    ComplexSharedPtr<T> get(const mull);

    size_t size() const;
};

#include "memory_reference_manager.ipp"