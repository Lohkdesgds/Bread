#pragma once

#include <dpp/dpp.h>
#include <dpp/nlohmann/json.hpp>
#include <dpp/fmt/format.h>
#include <Lunaris-Console/console.h>
#include <Lunaris-Bomb/bomb.h>
#include <Lunaris-Mutex/mutex.h>

#include <defaults.hpp>

constexpr char GROUPID_WRAP = '$';
constexpr char ITEMNAME_WRAP = '@';
constexpr char CUSTOMVAL_WRAP = '%';
constexpr char KINDTYPE_WRAP = '#';
constexpr char BLOCKED_NAMING_CHARS[] = {GROUPID_WRAP, ITEMNAME_WRAP, CUSTOMVAL_WRAP, KINDTYPE_WRAP};
// don't look at this: // do not use %,&

enum class custom_kinds{UNDEF = -1,SWITCH,STRING,ID};
const std::string custom_kinds_str[] = {"sw", "ss", "id"};
const std::string boolean_emojis[] = {"⛔", "✅"};

template<typename T>
struct customizable {
protected:
    virtual T* get_this() = 0;
    std::string custom_value;
public:
    custom_kinds kind = custom_kinds::UNDEF;

    bool get_custom_as_bool() const;
    dpp::snowflake get_custom_as_snowflake() const;
    const std::string& get_custom_as_string() const;

    T& set_custom(bool);
    T& set_custom(dpp::snowflake);
    T& set_custom(const char*);
    T& set_custom(const std::string&);
};

template<typename Extra>
struct item : public customizable<item<Extra>> {
protected:
    item<Extra>* get_this();
public:
    std::string name; // internal name
    std::string label; // shown name
    dpp::emoji custom_emoji; // emoji alongside this
    Extra extra; // button: dpp::component_style, select item: string

    item& set_name(const std::string&);
    item& set_label(const std::string&);
    item& set_emoji(dpp::emoji);
    item& set_emoji(const std::string&);
    item& set_extra(Extra);

    // from `custom_id` or `value` get: name, custom_value, kind
    std::string export_id() const;
    // to `custom_id` or `value` export: name, custom_value, kind
    void import_id(const std::string&);

    item() = default;    
    item(const std::string& lab, const std::string& nam);
    item(const std::string& lab, const std::string& nam, const Extra& ext);

    using customizable<item<Extra>>::get_custom_as_bool;
    using customizable<item<Extra>>::get_custom_as_snowflake;
    using customizable<item<Extra>>::get_custom_as_string;
    using customizable<item<Extra>>::set_custom;
};

struct generic_row {
    virtual void build(dpp::component) = 0;
    virtual dpp::component dump() const = 0;
    virtual bool can_dump() const = 0;
};

struct button_props {
    dpp::component_style style;
    bool disabled = false;
};

struct select_row : public generic_row {
    std::string group_name;
    std::vector<item<std::string>> items;

    select_row& set_group_name(const std::string&);
    select_row& push_item(item<std::string>);

    // expects actionrow
    void build(dpp::component);
    // actionrow
    dpp::component dump() const;
    // has something to dump
    bool can_dump() const;
};

struct button_row : public generic_row {
    std::string group_name;
    std::vector<item<button_props>> items;

    button_row& set_group_name(const std::string&);
    button_row& push_item(item<button_props>);

    // expects actionrow
    void build(dpp::component);
    // actionrow
    dpp::component dump() const;
    // has something to dump
    bool can_dump() const;
};

class transl_button_event {
public:
    struct trigger_info {
        std::variant<std::monostate, dpp::snowflake, std::string, bool> value;
        std::string group_name;
        std::string item_name;
        item<button_props>* target_if_button = nullptr;
        item<std::string>* target_if_select = nullptr;

        // do the casting auto, make it string if necessary
        std::string debug_value_as_string() const;
    };
private:
    const dpp::button_click_t* ifbtnclick = nullptr;
    const dpp::select_click_t* ifslcclick = nullptr;
    const dpp::form_submit_t* iffrmclick = nullptr;

    std::vector<std::variant<select_row, button_row>> rows;

    std::string message_content;
    std::vector<std::pair<std::string, std::string>> modal_response;

    trigger_info trigg;

    void fill_from_msg(const dpp::message&);
    void link_refs(); // trigger_info ref
public:

    transl_button_event() = default;

    transl_button_event(const dpp::button_click_t&);
    transl_button_event(const dpp::select_click_t&);
    transl_button_event(const dpp::form_submit_t&);

    // returns true if it was possible to add a button. Extra: where?
    bool push_or_replace(button_row, const size_t = static_cast<size_t>(-1));
    // returns true if it was possible to add a button. Extra: where?
    bool push_or_replace(select_row, const size_t = static_cast<size_t>(-1));

    std::vector<std::variant<select_row, button_row>>& get_rows();
    const std::vector<std::variant<select_row, button_row>>& get_rows() const;

    bool find_button_do(const std::string&, const std::string&, std::function<void(item<button_props>&)>);
    bool find_select_do(const std::string&, const std::string&, std::function<void(item<std::string>&)>);

    const std::vector<std::pair<std::string, std::string>>& get_modal_items() const;
    std::string find_modal_get(const std::string&) const;

    // find this string on names. Function gets left offset and right offset (right to left) and must return true to remove
    bool remove_group_named(const std::string&, std::function<bool(size_t, size_t)> = [](auto,auto){return true;});
    // as the remove_group_named, but all cases
    bool remove_group_named_all(const std::string&, std::function<bool(size_t, size_t)> = [](auto,auto){return true;});

    void set_content(const std::string&);
    const std::string& get_content() const;

    const trigger_info& get_trigger() const;

    // check if on create the references were valid once. If you deleted them this won't check that!
    bool has_valid_ref() const;
    bool can_push() const;

    // replace old message? Do something with the message before sending? (first bool == silent aka ephemeral?)
    bool reply(const bool, const bool = true, std::function<void(dpp::message&)> = {}) const;
    bool edit_response(std::function<void(dpp::message&)> = {}) const;

    // generate bool -> ephemeral?
    dpp::message generate_message(const bool) const;
};

//dpp::component make_fancy_modal(const std::string& label, const std::string& groupid, const std::string& itemid);

dpp::interaction_modal_response modal_generate(const std::string& groupid, const std::string itemid, const std::string& title);
dpp::component& modal_add_component(dpp::interaction_modal_response& modal, const std::string& label,
    const std::string id, const std::string placeholder, dpp::text_style_type styl, bool require = false, uint32_t min = 0, uint32_t max = 0);    

bool unsafe_string_name(const std::string&);
std::string custom_kind_to_str(const custom_kinds);
custom_kinds str_to_custom_kind(const std::string&);
std::string bool_to_emoji(const bool);
const bool emoji_to_bool(const std::string&);

std::string select_between(const std::string&, const char);

// expects: a;b;c;d;e...
std::vector<std::string> extract_emojis_auto(const std::string&);
std::string get_customid_as_str(const std::vector<dpp::component>&, const std::string&);

void lock_indefinitely();
template<typename T, typename K> 
bool find_json_autoabort(const nlohmann::json&, const T&, K&, std::function<void(const std::exception&)> = {});
template<typename J, typename T, typename K> 
bool find_json_array_autoabort(const nlohmann::json&, const T&, K&, std::function<void(const std::exception&)> = {});

std::vector<dpp::snowflake> slice_string_auto_snowflake(const std::string&);

unsigned long long get_time_ms();

nlohmann::json get_from_file(const std::string& path, const std::string& name, const std::string& extension);
bool save_file(const nlohmann::json& j, const std::string& path, const std::string& name, const std::string& extension);

// generates the lower ADD/REMOVE or ADD/STRING/REMOVE or ADD/REMOVE/STRING/ADD/REMOVE buttons (vector = disabled? A B C D)
// PRESET, custom_id format, MIDDLE STRING (if needed, custom custom), disable ordering
//dpp::component generate_button_auto(const preset_buttons, const std::string&, const std::string& = {}, const std::vector<bool> = {});


#include <impl/general_functions.ipp>