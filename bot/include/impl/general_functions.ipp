#pragma once

#include <general_functions.hpp>

template<typename T>
inline bool customizable<T>::get_custom_as_bool() const
{
    return custom_value == "1" ? true : false;
}

template<typename T>
inline dpp::snowflake customizable<T>::get_custom_as_snowflake() const
{
    return dpp::from_string<dpp::snowflake>(custom_value);
}

template<typename T>
inline const std::string& customizable<T>::get_custom_as_string() const
{
    return custom_value;
}

template<typename T>
inline T& customizable<T>::set_custom(bool b)
{
    custom_value = b ? "1" : "0";
    kind = custom_kinds::SWITCH;
    return *get_this();
}

template<typename T>
inline T& customizable<T>::set_custom(dpp::snowflake v)
{
    custom_value = std::to_string(v);
    kind = custom_kinds::ID;
    return *get_this();
}

template<typename T>
inline T& customizable<T>::set_custom(const char* v)
{
    if(unsafe_string_name(v)) throw std::invalid_argument("Value contains invalid characters! (custom check)");
    custom_value = v;
    kind = custom_kinds::STRING;
    return *get_this();
}

template<typename T>
inline T& customizable<T>::set_custom(const std::string& v)
{
    if(unsafe_string_name(v)) throw std::invalid_argument("Value contains invalid characters! (custom check)");
    custom_value = v;
    kind = custom_kinds::STRING;
    return *get_this();
}


template<typename Extra>
item<Extra>* item<Extra>::get_this()
{
    return this;
}

template<typename Extra>
item<Extra>& item<Extra>::set_name(const std::string& v)
{
    if(unsafe_string_name(v)) throw std::invalid_argument("Value contains invalid characters! (button check)");
    name = v;
    return *this;
}

template<typename Extra>
item<Extra>& item<Extra>::set_label(const std::string& v)
{
    label = v;
    return *this;
}

template<typename Extra>
item<Extra>& item<Extra>::set_emoji(dpp::emoji e)
{
    custom_emoji = e;
    return *this;
}

template<typename Extra>
item<Extra>& item<Extra>::set_emoji(const std::string& v)
{
    custom_emoji.name = v;
    custom_emoji.id = 0;
    custom_emoji.flags = 0;
    return *this;
}

template<typename Extra>
item<Extra>& item<Extra>::set_extra(Extra v)
{
    extra = v;
    return *this;
}

template<typename Extra>
std::string item<Extra>::export_id() const
{
    const std::string& cstm = get_custom_as_string();
    return ((name.size()) ? (ITEMNAME_WRAP + name + ITEMNAME_WRAP) : "") + 
        ((cstm.size()) ? (KINDTYPE_WRAP + custom_kind_to_str(this->kind) + KINDTYPE_WRAP + CUSTOMVAL_WRAP + cstm + CUSTOMVAL_WRAP) : "");
}

template<typename Extra>
void item<Extra>::import_id(const std::string& s)
{
    name = select_between(s, ITEMNAME_WRAP);
    set_custom(select_between(s, CUSTOMVAL_WRAP));
    this->kind = str_to_custom_kind(select_between(s, KINDTYPE_WRAP));
}

template<typename Extra>
item<Extra>::item(const std::string& lab, const std::string& nam)
    : label(lab), name(nam)
{
}
template<typename Extra>
item<Extra>::item(const std::string& lab, const std::string& nam, const Extra& ext)
    : label(lab), name(nam), extra(ext)
{
}


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

template<typename J, typename T, typename K>
inline bool find_json_array_autoabort(const nlohmann::json& json, const T& key, K& to, std::function<void(const std::exception&)> errfunc)
{ 
    auto jr = json.find(key);
    if (jr != json.end()) {
        try {
            for(auto it : *jr) to.insert(to.end(), J(it));
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