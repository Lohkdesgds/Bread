#pragma once

#include <shared_mutex>
#include <functional>

template<typename T>
class safe_data {
    T obj;
    mutable std::shared_mutex mu;
public:
    template<typename J>
    J csafe(const std::function<J(const T&)>) const;
    template<typename J>
    J safe(const std::function<J(T&)>);

    T reset();
    T reset(T&&);
};

template<typename T>
class force_const {
    const std::shared_ptr<T> ptr;
public:
    force_const(const std::shared_ptr<T>&);

    const T* const operator->() const;
    const T& operator*() const;

    T& unsafe();

    operator bool() const;
};

#include <impl/safe_template.ipp>