#pragma once

#include <dpp/dpp.h>
#include <dpp/nlohmann/json.hpp>
#include <dpp/fmt/format.h>
#include <console.h>
#include <bomb.h>

constexpr char GROUPID_WRAP = '$';
constexpr char ITEMNAME_WRAP = '@';
constexpr char CUSTOMVAL_WRAP = '%';
constexpr char KINDTYPE_WRAP = '#';
constexpr char BLOCKED_NAMING_CHARS[] = {GROUPID_WRAP, ITEMNAME_WRAP, CUSTOMVAL_WRAP, KINDTYPE_WRAP};
// don't look at this: // do not use %,&

enum class custom_kinds{UNDEF = -1,SWITCH,STRING,ID};
const std::string custom_kinds_str[] = {"sw", "ss", "id"};
const std::string boolean_emojis[] = {"⛔", "✅"};

struct button { // value is LABEL (kinda)
private:
    std::string custom_value;
public:
    custom_kinds kind{};
    std::string group_id;
    std::string item_name;
    std::string label;
    dpp::emoji custom_emoji;

    bool is_trigger = false;
    dpp::component_style style;

    button& set_kind(custom_kinds);
    button& set_item_name(const std::string&);
    button& set_emoji(dpp::emoji);
    button& set_label(const std::string&);
    button& set_style(dpp::component_style);

    bool get_custom_as_bool() const;
    dpp::snowflake get_custom_as_snowflake() const;
    const std::string& get_custom_as_string() const;

    button& set_custom(bool);
    button& set_custom(dpp::snowflake);
    button& set_custom(const char*);
    button& set_custom(const std::string&);

    button() = default;
    
    button(const std::string& lab, const std::string& nam);
};

struct select_item {
private:
    std::string custom_value;
public:
    custom_kinds kind{};
    std::string label;
    std::string item_name;
    std::string desc;
    bool is_trigger;

    bool get_custom_as_bool() const;
    dpp::snowflake get_custom_as_snowflake() const;
    const std::string& get_custom_as_string() const;

    select_item& set_custom(bool);
    select_item& set_custom(dpp::snowflake);
    select_item& set_custom(const char*);
    select_item& set_custom(const std::string&);

    select_item() = default;
    select_item(const std::string&, const std::string&, const std::string&, const std::string& = {});
};

struct select_row {
private:
    std::string custom_value;
public:
    custom_kinds kind{};
    std::string group_id;
    std::vector<select_item> items;

    bool get_custom_as_bool() const;
    dpp::snowflake get_custom_as_snowflake() const;
    const std::string& get_custom_as_string() const;

    select_row& set_custom(bool);
    select_row& set_custom(dpp::snowflake);
    select_row& set_custom(const char*);
    select_row& set_custom(const std::string&);

    select_row() = default;
    select_row(const std::vector<select_item>&);
};


// applicable on select menu or button
//class button_work {
//public:
//    enum class preset_buttons {ADDDEL, ADDDELGROUP, BUTTONSELECT, ROWONLY};
//    enum class preset_buttons_selected{ ADD,DEL,ADDGROUP,DELGROUP,CUSTOMTAG,SELECT,SELECT_CLICK,_SIZE };
//private:
//    static const std::string s_buttons[static_cast<size_t>(preset_buttons_selected::_SIZE)];
//    static const std::string s_row;
//
//    bool disabled[static_cast<size_t>(preset_buttons_selected::_SIZE)]{false};
//
//    void get_message_data(const dpp::message&);
//    dpp::component find_component_starts_with(const std::vector<dpp::component>&, const std::string&);
//    std::string remove_s_buttons_tag(std::string) const;
//    std::string remove_s_row_tag(std::string) const;
//public:
//    // custom id used for generation. This is your key. This is your event name or something.
//    std::string base_id;
//    // if button kind, this is a stored data on customtag or select_click. Changeable because you may want to change it, I don't know.
//    std::string stored;
//    // if row, selected one in that, if button, what button was triggered. Const because it's an input only.
//    const std::string selected;
//    // mode for generation or mode detected. For building buttons on generate_buttons();
//    preset_buttons button_mode = preset_buttons::ADDDEL;
//    // if there's a list thing, this is the list (hopefully). Used on generate_rows();
//    std::vector<dpp::select_option> rows;
//
//    // set a type already
//    button_work(const std::string&, const preset_buttons = preset_buttons::ADDDEL);
//    
//    button_work(const dpp::button_click_t&);
//    button_work(const dpp::select_click_t&);
//
//    // return _SIZE if nothing found.
//    preset_buttons_selected interpret_selected() const;
//
//    bool get_disabled(const preset_buttons_selected) const;
//    void set_disabled(const preset_buttons_selected, const bool);
//
//    dpp::component generate_buttons() const;
//    dpp::component generate_rows() const;
//
//    void replace_components_of(dpp::message&);
//};
//
//class selectable_button {
//public:
//    struct prop {
//        std::string name;
//        bool yesno = false;
//    };
//private:
//    std::vector<prop> opts; // max 5 * 5 (discord rules)
//public:
//    // prefix on buttons
//    std::string group_name;
//
//    selectable_button(const std::string&);
//
//};

class transl_button_event {
    const dpp::button_click_t* ifbtnclick = nullptr;
    const dpp::select_click_t* ifslcclick = nullptr;

    std::vector<button> button_vec;
    std::vector<select_row> select_vec;

    std::string message_content;

    const std::string custom_value;
    const custom_kinds custom_kind;

    void fill_from_msg(const dpp::message&);
public:
    const std::string group_id, trigger_id;

    transl_button_event(const std::string&);

    transl_button_event(const dpp::button_click_t&);
    transl_button_event(const dpp::select_click_t&);

    // returns true if it was possible to add a button
    bool push_button(button);
    // add a new list to the lists if possible
    bool push_select(select_row);

    std::vector<button>& get_buttons();
    const std::vector<button>& get_buttons() const;
    std::vector<select_row>& get_select();
    const std::vector<select_row>& get_select() const;

    void set_content(const std::string&);
    const std::string& get_content() const;

    custom_kinds get_custom_kind() const;
    bool get_custom_as_bool() const;
    dpp::snowflake get_custom_as_snowflake() const;
    const std::string& get_custom_as_string() const;

    bool can_push_button() const;
    bool can_push_select() const;

    // replace old message? Do something with the message before sending?
    bool reply(const bool = true, std::function<void(dpp::message&)> = {}) const;

    dpp::message generate_message() const;
};

bool unsafe_string_name(const std::string&);
std::string custom_kind_to_str(const custom_kinds);
custom_kinds str_to_custom_kind(const std::string&);
std::string bool_to_emoji(const bool);
const bool emoji_to_bool(const std::string&);

std::string select_between(const std::string&, const char);


void lock_indefinitely();
template<typename T, typename K> 
bool find_json_autoabort(const nlohmann::json&, const T&, K&, std::function<void(const std::exception&)> = {});
template<typename J, typename T, typename K> 
bool find_json_array_autoabort(const nlohmann::json&, const T&, K&, std::function<void(const std::exception&)> = {});

std::vector<dpp::snowflake> slice_string_auto_snowflake(const std::string&);

unsigned long long get_time_ms();

// generates the lower ADD/REMOVE or ADD/STRING/REMOVE or ADD/REMOVE/STRING/ADD/REMOVE buttons (vector = disabled? A B C D)
// PRESET, custom_id format, MIDDLE STRING (if needed, custom custom), disable ordering
//dpp::component generate_button_auto(const preset_buttons, const std::string&, const std::string& = {}, const std::vector<bool> = {});


#include <impl/general_functions.ipp>