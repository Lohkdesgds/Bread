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

#include <impl/safe_template.ipp>