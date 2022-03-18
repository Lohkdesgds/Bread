#pragma once 

#include <timed_factory.hpp>

template<typename Key, typename VarType>
inline timed_factory<Key, VarType>::mem::mem(const std::chrono::seconds& rf)
    : ref_delta(rf), dt(std::chrono::system_clock::now().time_since_epoch() + rf)
{
}

template<typename Key, typename VarType>
inline timed_factory<Key, VarType>::mem::mem(mem&& mm)
    : ref_delta(mm.ref_delta), dt(mm.dt), var(std::move(mm.var))
{
}

template<typename Key, typename VarType>
inline bool timed_factory<Key, VarType>::mem::should_del() const
{
    return (std::chrono::system_clock::now().time_since_epoch() - dt).count() > 0;
}

template<typename Key, typename VarType>
inline void timed_factory<Key, VarType>::mem::update_time()
{
    dt = std::chrono::system_clock::now().time_since_epoch() + ref_delta;
}


template<typename Key, typename VarType>
inline std::shared_ptr<VarType> timed_factory<Key, VarType>::operator[](const Key& k)
{
    std::unique_lock<std::shared_mutex> lu(shr);
    auto it = objs.find(k);
    if (it != objs.end()) return it->second.var;

    mem mm(delta_for_kill);
    std::shared_ptr<VarType> cpy = std::make_shared<VarType>();
    mm.var = cpy;
    objs.insert({k, std::move(mm)});

    return cpy;
}

template<typename Key, typename VarType>
inline std::shared_ptr<VarType> timed_factory<Key, VarType>::operator[](const Key& k) const
{
    std::shared_lock<std::shared_mutex> lu(shr);
    auto it = objs.find(k);
    if (it == objs.end()) return {}; // null
    return it->second.var;
}

template<typename Key, typename VarType>
inline void timed_factory<Key, VarType>::free(const Key& k)
{
    std::unique_lock<std::shared_mutex> lu(shr);
    auto it = objs.find(k);
    if (it != objs.end()) objs.erase(it);
}

template<typename Key, typename VarType>
inline void timed_factory<Key, VarType>::free_freeable()
{
    std::unique_lock<std::shared_mutex> lu(shr);
    for(auto it = objs.begin(); it != objs.end();)
    {
        if (it->second.should_del()) {
            it = objs.erase(it);
        }
        else ++it;
    }
}

template<typename Key, typename VarType>
inline void timed_factory<Key, VarType>::set_free_time(std::chrono::seconds t)
{
    if (t.count() >= 10) delta_for_kill = t;
    else delta_for_kill = std::chrono::seconds(10);
}

template<typename Key, typename VarType>
inline size_t timed_factory<Key, VarType>::size() const
{
    return objs.size();
}