#pragma once

#include <dpp/dpp.h>
#include <dpp/nlohmann/json.hpp>
#include <dpp/fmt/format.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <iostream>
#include <mutex>
#include <sstream>

#include "console.hpp"

using mull = unsigned long long;

// src: https://stackoverflow.com/questions/43526647/decltype-of-function-parameter

template <std::size_t N, typename T0, typename ... Ts>
struct typeN
 { using type = typename typeN<N-1U, Ts...>::type; };

template <typename T0, typename ... Ts>
struct typeN<0U, T0, Ts...>
 { using type = T0; };

template <std::size_t, typename>
struct argN;

template <std::size_t N, typename R, typename ... As>
struct argN<N, R(As...)>
 { using type = typename typeN<N, As...>::type; };

template <typename>
struct returnType;

template <typename R, typename ... As>
struct returnType<R(As...)>
 { using type = R; };


//class atomic_ull {
//    std::mutex m;
//    mull val = 0;
//public:
//    void plus();
//    void minus();
//    mull read();
//};

template<typename T>
class nullable_ref {
    T* opt = nullptr;
    const T* ref = nullptr;
public:
    nullable_ref() = default;
    ~nullable_ref();

    nullable_ref(const nullable_ref&); // copy, even if arg is ref
    nullable_ref(nullable_ref&&); // move
    void operator=(const nullable_ref&) = delete;
    void operator=(nullable_ref&&) = delete;

    nullable_ref(const T&);
    nullable_ref(const T*);

    void operator=(const T&) = delete;
    void operator=(const T*) = delete;

    const T* operator->() const;
    const T& operator*() const;
    template<typename Q> bool operator==(const Q&) const;
    bool operator==(const T*) const;
    template<typename Q> bool operator!=(const Q&) const;
    bool operator!=(const T*) const;
    bool operator!() const;
    //operator const T&() const;

    bool is_null() const;
    bool is_self() const; // self created or ref?
    bool is_ref() const; // ref or self created?
    T& get_noncte(); // if copy, cool, else this may fail
    const T& get() const;

    T copy_zero() const; // copy if exists, else return zero equivalent value
};

class jsonable {
public:
    jsonable() = default;
    jsonable(const nlohmann::json&);
    void operator=(const nlohmann::json&);

    virtual void from_json(const nlohmann::json&) {};
};

class ProcessInfo {
public:
    enum class data{PID,COMM,STATE,PPID,PGRP,SESSION,TTY_NR,TPGID,FLAGS,MINFLT,CMINFLT,MAJFLT,
        CMAJFLT,UTIME,STIME,CUTIME,CSTIME,PRIORITY,NICE,NUM_THREADS,ITREALVALUE,STARTTIME,
        VSIZE,RSS,RSSLIM,STARTCODE,ENDCODE,STARTSTACK,KSTKESP,KSTKEIP,SIGNAL,BLOCKED,
        SIGIGNORE,SIGCATCH,WCHAN,NSWAP,CNSWAP,EXIT_SIGNAL,PROCESSOR,_SIZE};
private:
    std::string dat[static_cast<size_t>(data::_SIZE)];
public:
    void generate();

    const std::string& get(const data&);
};

mull get_time_ms();
std::string fix_name_for_cmd(std::string);
std::string get_thread_id_str();
int32_t rgb_to_decimal_color(int, int, int);

//size_t cast_to_switch(const std::string&, const std::initializer_list<std::string>);
//size_t cast_to_switch(const nullable_ref<std::string>&, const std::initializer_list<std::string>);

nullable_ref<std::string>               get_first_name(const dpp::command_interaction&);
nullable_ref<dpp::command_data_option>  get_first_option(const dpp::command_interaction&);
nullable_ref<std::string>               get_first_name(const dpp::command_data_option&);
nullable_ref<dpp::command_data_option>  get_first_option(const dpp::command_data_option&);

nullable_ref<std::string>               get_first_name(const nullable_ref<dpp::command_interaction>&);
nullable_ref<dpp::command_data_option>  get_first_option(const nullable_ref<dpp::command_interaction>&);
nullable_ref<std::string>               get_first_name(const nullable_ref<dpp::command_data_option>&);
nullable_ref<dpp::command_data_option>  get_first_option(const nullable_ref<dpp::command_data_option>&);

nullable_ref<std::string>               get_str_in_command(const dpp::command_interaction&, const std::string&);
nullable_ref<int32_t>                   get_int_in_command(const dpp::command_interaction&, const std::string&);
nullable_ref<bool>                      get_bool_in_command(const dpp::command_interaction&, const std::string&);
nullable_ref<dpp::snowflake>            get_mull_in_command(const dpp::command_interaction&, const std::string&);
nullable_ref<dpp::command_data_option>  get_cmd_in_command(const dpp::command_interaction&, const std::string&);

nullable_ref<std::string>               get_str_in_command(const dpp::command_data_option&, const std::string&);
nullable_ref<int32_t>                   get_int_in_command(const dpp::command_data_option&, const std::string&);
nullable_ref<bool>                      get_bool_in_command(const dpp::command_data_option&, const std::string&);
nullable_ref<dpp::snowflake>            get_mull_in_command(const dpp::command_data_option&, const std::string&);
nullable_ref<dpp::command_data_option>  get_cmd_in_command(const dpp::command_data_option&, const std::string&);

nullable_ref<std::string>               get_str_in_command(const nullable_ref<dpp::command_interaction>&, const std::string&);
nullable_ref<int32_t>                   get_int_in_command(const nullable_ref<dpp::command_interaction>&, const std::string&);
nullable_ref<bool>                      get_bool_in_command(const nullable_ref<dpp::command_interaction>&, const std::string&);
nullable_ref<dpp::snowflake>            get_mull_in_command(const nullable_ref<dpp::command_interaction>&, const std::string&);
nullable_ref<dpp::command_data_option>  get_cmd_in_command(const nullable_ref<dpp::command_interaction>&, const std::string&);

nullable_ref<std::string>               get_str_in_command(const nullable_ref<dpp::command_data_option>&, const std::string&);
nullable_ref<int32_t>                   get_int_in_command(const nullable_ref<dpp::command_data_option>&, const std::string&);
nullable_ref<bool>                      get_bool_in_command(const nullable_ref<dpp::command_data_option>&, const std::string&);
nullable_ref<dpp::snowflake>            get_mull_in_command(const nullable_ref<dpp::command_data_option>&, const std::string&);
nullable_ref<dpp::command_data_option>  get_cmd_in_command(const nullable_ref<dpp::command_data_option>&, const std::string&);

/// INLINE

/*class json_type_fix {
    const nlohmann::json& r;
public:
    json_type_fix(const nlohmann::json& j) : r(j) {}

    template<typename T>
    inline operator T() const {
        try {
            return r.get<T>();
        }
        catch(const std::exception& e) {
            throw std::runtime_error(std::string("Invalid JSON expected type\nJSON error: ") + e.what() + "\nJSON dump:\n" + r.dump());
        }
        catch(...){
            throw std::runtime_error(std::string("Invalid JSON expected type\nJSON error: UNCAUGHT\nJSON dump:\n") + r.dump());
        }
        return T{};
    }
};*/

// expects a jsonable child
template<typename T, std::enable_if_t<std::is_base_of_v<jsonable, T>, int> = 0>
inline void safe_json(const nlohmann::json& jj, const std::string& key, T& t, const bool must_have = true)
{
    if (auto jr = jj.find(key); jr != jj.end() && !jr->is_null()) {
        const nlohmann::json& j = *jr;
        try {
            ((jsonable&)t) = j;
        }
        catch(const std::exception& e){
            Lunaris::cout << "[EXCEPTION JSON] FAILED TO CONVERT ONE JSON! (type was '" << static_cast<int8_t>(j.type()) << " (json id)', expected '" << typeid(T).name() << "')" ;
            Lunaris::cout << "[EXCEPTION JSON] Details: " << e.what() ;
            throw std::runtime_error(std::string("JSON @ key '") + key + "' issued: " + e.what());
        }
        catch(...){
            Lunaris::cout << "[EXCEPTION JSON] FAILED TO CONVERT ONE JSON! (type was '" << static_cast<int8_t>(j.type()) << " (json id)', expected '" << typeid(T).name() << "')" ;
            throw std::runtime_error(std::string("JSON @ key '") + key + "' issued UNCAUGHT");
        }
    }
    else if (must_have) throw std::runtime_error(std::string("JSON @ key '") + key + "' had NO VALUE! (empty)"); 
}

// no jsonable child
template<typename T, std::enable_if_t<!std::is_base_of_v<jsonable, T>, int> = 0>
inline void safe_json(const nlohmann::json& jj, const std::string& key, T& t, const bool must_have = true)
{
    if (auto jr = jj.find(key); jr != jj.end() && !jr->is_null()) {
        const nlohmann::json& j = *jr;
        try {
            t = j;
        }
        catch(const std::exception& e){
            Lunaris::cout << "[EXCEPTION JSON] FAILED TO CONVERT ONE JSON! (type was '" << static_cast<int8_t>(j.type()) << " (json id)', expected '" << typeid(T).name() << "')" ;
            Lunaris::cout << "[EXCEPTION JSON] Details: " << e.what() ;
            throw std::runtime_error(std::string("JSON @ key '") + key + "' issued: " + e.what());
        }
        catch(...){
            Lunaris::cout << "[EXCEPTION JSON] FAILED TO CONVERT ONE JSON! (type was '" << static_cast<int8_t>(j.type()) << " (json id)', expected '" << typeid(T).name() << "')" ;
            throw std::runtime_error(std::string("JSON @ key '") + key + "' issued UNCAUGHT");
        }
    }
    else if (must_have) throw std::runtime_error(std::string("JSON @ key '") + key + "' had NO VALUE! (empty)"); 
}

template<typename T, std::enable_if_t<std::is_base_of_v<jsonable, T>, int> = 0>
inline void __safe_json_array(const nlohmann::json& jj, const std::string& key, const std::function<void(const T&)>& eater, const bool must_have)
{
    //Lunaris::cout << "__ reading array '" << key << "' ...\n";
    if (auto jr = jj.find(key); jr != jj.end() && !jr->is_null() && eater) {
        const nlohmann::json& j = *jr;
        int8_t currtyp = 0;
        try {
            for(const auto& e : j) {
                currtyp = static_cast<uint8_t>(e.type());
                T hee;
                (jsonable&)hee = e;
                eater(hee);
                //eater.insert(e);
            }
        }
        catch(const std::exception& e){
            Lunaris::cout << "[EXCEPTION JSON] FAILED TO CONVERT ONE JSON! (type was '" << currtyp << " (json id)', expected '" << typeid(T).name() << "')" ;
            Lunaris::cout << "[EXCEPTION JSON] Details: " << e.what() ;
            throw std::runtime_error(std::string("JSON @ key '") + key + "' issued: " + e.what());
        }
        catch(...){
            Lunaris::cout << "[EXCEPTION JSON] FAILED TO CONVERT ONE JSON! (type was '" << currtyp << " (json id)', expected '" << typeid(T).name() << "')" ;
            throw std::runtime_error(std::string("JSON @ key '") + key + "' issued UNCAUGHT");
        }
        //Lunaris::cout << "FAILED TO CONVERT ONE JSON! (type was '" << static_cast<int8_t>(j.type()) << "', expected '" << typeid(T).name() << "')" ;
        //throw std::runtime_error(std::string("JSON @ key '") + key + "' issued UNKNOWN");
        //Lunaris::cout << "__ good array '" << key << "' ...\n";
    }
    else if (must_have) throw std::runtime_error(std::string("JSON @ key '") + key + "' was EMPTY or invalid FUNCTION!");
}

template<typename T, std::enable_if_t<!std::is_base_of_v<jsonable, T>, int> = 0>
inline void __safe_json_array(const nlohmann::json& jj, const std::string& key, const std::function<void(const T&)>& eater, const bool must_have)
{
    //Lunaris::cout << "__ reading array '" << key << "' ...\n";
    if (auto jr = jj.find(key); jr != jj.end() && !jr->is_null() && eater) {
        const nlohmann::json& j = *jr;
        int8_t currtyp = 0;
        try {
            for(const auto& e : j) {
                currtyp = static_cast<uint8_t>(e.type());
                eater(e);
                //eater.insert(e);
            }
        }
        catch(const std::exception& e){
            Lunaris::cout << "[EXCEPTION JSON] FAILED TO CONVERT ONE JSON! (type was '" << currtyp << " (json id)', expected '" << typeid(T).name() << "')" ;
            Lunaris::cout << "[EXCEPTION JSON] Details: " << e.what() ;
            throw std::runtime_error(std::string("JSON @ key '") + key + "' issued: " + e.what());
        }
        catch(...){
            Lunaris::cout << "[EXCEPTION JSON] FAILED TO CONVERT ONE JSON! (type was '" << currtyp << " (json id)', expected '" << typeid(T).name() << "')" ;
            throw std::runtime_error(std::string("JSON @ key '") + key + "' issued UNCAUGHT");
        }
        //Lunaris::cout << "FAILED TO CONVERT ONE JSON! (type was '" << static_cast<int8_t>(j.type()) << "', expected '" << typeid(T).name() << "')" ;
        //throw std::runtime_error(std::string("JSON @ key '") + key + "' issued UNKNOWN");
        //Lunaris::cout << "__ good array '" << key << "' ...\n";
    }
    else if (must_have) throw std::runtime_error(std::string("JSON @ key '") + key + "' was EMPTY or invalid FUNCTION!");
}

template<typename K, template <typename> typename T, std::enable_if_t<(std::is_same<std::vector<K>, T<K>>::value/* || std::is_same<std::list<K>, T<K>>::value*/), int> = 0>
inline void safe_json_array(const nlohmann::json& jj, const std::string& key, T<K>& yoo, const bool must_have = true)
{
    __safe_json_array<K>(jj, key, [&](const K& t){yoo.push_back(t);}, must_have);
}

template<typename K, typename J, template <typename, typename> typename T, std::enable_if_t<(std::is_same<std::unordered_map<K, J>, T<K, J>>::value || std::is_same<std::map<K, J>, T<K, J>>::value), int> = 0>
inline void safe_json_array(const nlohmann::json& jj, const std::string& key, T<K, J>& yoo, const bool must_have = true)
{
    __safe_json_array<std::pair<K, J>>(jj, key, [&](const std::pair<K, J>& t){yoo.insert(t);}, must_have);
}

template<typename VEC, typename FINDF, typename DOF> // bool(const each&), void (each&)
inline void find_and_set_or_replace(VEC& v, FINDF f, DOF replc)
{
    for(auto it = v.begin(); it != v.end();)
    {
        if (f(*it)) {
            replc(*it);
            return;
        }
        ++it;
    }
    v.push_back({});
    replc(*(--v.end()));
}

template<typename VEC, typename FINDF, typename DOF> // // bool(const each&), void (each_iterator)
inline bool find_and_do(VEC& v, FINDF f, DOF d)
{
    for(auto it = v.begin(); it != v.end();)
    {
        if (f(*it)){
            return d(it);
        }
        else ++it;
    }
    return false;
}


template<typename T>
inline nullable_ref<T>::~nullable_ref()
{
    if (opt) {
        delete opt;
        opt = nullptr;
    }
}

template<typename T>
inline nullable_ref<T>::nullable_ref(const nullable_ref& w)
{
    if (w.opt) opt = new T(*w.opt);
    else if (w.ref) opt = new T(*w.ref);
}

template<typename T>
inline nullable_ref<T>::nullable_ref(nullable_ref&& w)
    : opt(w.opt), ref(w.ref)
{
    w.opt = nullptr;
}

template<typename T>
inline nullable_ref<T>::nullable_ref(const T& w)
    : opt(new T(w))
{
}

template<typename T>
inline nullable_ref<T>::nullable_ref(const T* w)
    : ref(w)
{
}

template<typename T>
const T* nullable_ref<T>::operator->() const
{
    return ref ? ref : opt;
}

template<typename T>
const T& nullable_ref<T>::operator*() const
{
    return ref ? *ref : *opt;
}

template<typename T>
template<typename Q>
bool nullable_ref<T>::operator==(const Q& w) const
{
    return is_null() ? false : ((ref ? *ref : *opt) == w);
}

template<typename T>
bool nullable_ref<T>::operator==(const T* w) const
{
    return (ref ? ref : opt) == w;
}

template<typename T>
template<typename Q>
bool nullable_ref<T>::operator!=(const Q& w) const
{
    return is_null() ? true : ((ref ? *ref : *opt) != w);
}

template<typename T>
bool nullable_ref<T>::operator!=(const T* w) const
{
    return (ref ? ref : opt) != w;
}

template<typename T>
bool nullable_ref<T>::operator!() const
{
    return is_null();
}

/*template<typename T>
nullable_ref<T>::operator const T&() const
{
    return ref ? *ref : *opt;
}*/

template<typename T>
inline bool nullable_ref<T>::is_null() const
{
    return this == nullptr || (opt == nullptr && ref == nullptr);
}

template<typename T>
inline bool nullable_ref<T>::is_self() const
{
    return ref == nullptr && opt != nullptr;
}

template<typename T>
inline bool nullable_ref<T>::is_ref() const
{
    return ref != nullptr && opt == nullptr;
}

template<typename T>
inline T& nullable_ref<T>::get_noncte()
{
    return opt ? *opt : *const_cast<T*>(ref);
}

template<typename T>
inline const T& nullable_ref<T>::get() const
{
    return ref ? *ref : *opt;
}

template<typename T>
T nullable_ref<T>::copy_zero() const // copy if exists, else return zero equivalent value
{
    return is_null() ? T() : (ref ? *ref : *opt);
}