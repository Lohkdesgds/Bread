#pragma once

#include <general_functions.hpp>

template<typename T, typename K>
inline bool find_json_autoabort(const nlohmann::json& json, const T& key, K& to, std::function<void(const std::exception&)> errfunc)
{ 
    auto jr = json.find(key);
    if (jr != json.end()) {
        try {
            to = (*jr);
            return true;
        }
        catch(const std::exception& e) {
            if (errfunc) errfunc(e);
        }
        catch(...) {
            if (errfunc) errfunc(std::runtime_error("UNCAUGHT"));
        }
    }
    return false;
}

template<typename T, typename K>
inline bool find_json_array_autoabort(const nlohmann::json& json, const T& key, K& to, std::function<void(const std::exception&)> errfunc)
{ 
    if (!to) return false;
    auto jr = json.find(key);
    if (jr != json.end()) {
        try {
            for(auto it : jr) to.insert(to.end(), it);
            return true;
        }
        catch(const std::exception& e) {
            if (errfunc) errfunc(e);
        }
        catch(...) {
            if (errfunc) errfunc(std::runtime_error("UNCAUGHT"));
        }
    }
    return false;
}