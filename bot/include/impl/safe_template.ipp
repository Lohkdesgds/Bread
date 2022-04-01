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

template<typename T>
force_const<T>::force_const(const std::shared_ptr<T>& p)
    : ptr(p)
{
}

template<typename T>
const T* const force_const<T>::operator->() const
{
    return ptr.get();
}

template<typename T>
const T& force_const<T>::operator*() const
{
    return *ptr.get();
}

template<typename T>
T& force_const<T>::unsafe()
{
    return *ptr.get();
}

template<typename T>
force_const<T>::operator bool() const
{
    return ptr.get() != nullptr;
}