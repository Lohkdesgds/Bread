#pragma once

#include <safe_template.hpp>

template<typename T>
template<typename J>
inline J safe_data<T>::csafe(const std::function<J(const T&)> f) const
{
    if (!f) throw std::invalid_argument("Function was null!");
    std::shared_lock<std::shared_mutex> lu(mu);
    return f(obj);
}

template<typename T>
template<typename J>
inline J safe_data<T>::safe(const std::function<J(T&)> f)
{
    if (!f) throw std::invalid_argument("Function was null!");
    std::unique_lock<std::shared_mutex> lu(mu);
    return f(obj);
}

template<typename T>
inline T safe_data<T>::reset()
{
    std::unique_lock<std::shared_mutex> lu(mu);
    T mov = std::move(obj);
    return mov;
}

template<typename T>
inline T safe_data<T>::reset(T&& v)
{
    std::unique_lock<std::shared_mutex> lu(mu);
    T mov = std::move(obj);
    obj = std::move(v);
    return mov;
}