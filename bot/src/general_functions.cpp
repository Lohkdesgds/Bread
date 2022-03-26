#include <general_functions.hpp>

//const std::string button_work::s_buttons[static_cast<size_t>(button_work::preset_buttons_selected::_SIZE)] = { "add-", "del-", "addg-", "delg-", "tag-", "sel-", "selclk-" };
//const std::string button_work::s_row = "row-";
//
//void button_work::get_message_data(const dpp::message& m)
//{
//    dpp::component _tmp;
//
//    for(size_t p = 0; p < static_cast<size_t>(preset_buttons_selected::_SIZE); ++p) {
//        if ((_tmp = find_component_starts_with(m.components, s_buttons[p])).custom_id.size()) {
//            disabled[p] = _tmp.disabled;
//
//            if (base_id.empty()) 
//                base_id = _tmp.custom_id.size() > s_buttons[p].size() ? _tmp.custom_id.substr(s_buttons[p].size()) : "";
//
//            switch(p) {
//            case static_cast<size_t>(preset_buttons_selected::SELECT_CLICK):
//                stored = _tmp.label;
//                break;
//            case static_cast<size_t>(preset_buttons_selected::CUSTOMTAG):
//                stored = _tmp.label;
//                break;
//            }
//
//        }
//    }
//
//    if ((_tmp = find_component_starts_with(m.components, s_row)).options.size()) {
//        for (const auto& op : _tmp.options) rows.push_back(op);
//    }    
//}
//
//dpp::component button_work::find_component_starts_with(const std::vector<dpp::component>& v, const std::string& s)
//{
//    for(const auto& i : v) {
//        if (i.custom_id.find(s) == 0) return i;
//        if (i.components.size() > 0) {
//            dpp::component found = find_component_starts_with(i.components, s);
//            if (found.custom_id.size() > 0) return found;
//        }
//    }
//    return {};
//}
//
//std::string button_work::remove_s_buttons_tag(std::string s) const
//{
//    for(const auto& i : s_buttons) {        
//        if (s.find(i) == 0) {
//            return s.size() > i.size() ? s.substr(i.size()) : "";
//        }
//    }
//    return s;
//}
//
//std::string button_work::remove_s_row_tag(std::string s) const
//{
//    if (s.find(s_row) == 0) {
//        return s.size() > s_row.size() ? s.substr(s_row.size()) : "";
//    }
//    return s;
//}
//
//button_work::button_work(const std::string& tg, const preset_buttons mod)
//    : base_id(tg), button_mode(mod)
//{
//}
//
//button_work::button_work(const dpp::button_click_t& im)
//    : selected(remove_s_buttons_tag(im.custom_id)) // selected button
//{
//    // a row only would never trigger a button, so discarded option
//    if (find_component_starts_with(im.command.msg.components, s_buttons[static_cast<size_t>(preset_buttons_selected::SELECT)]).custom_id.size() > 0) { // found SELECT, must be select
//        button_mode = preset_buttons::BUTTONSELECT;
//    }
//    else if (find_component_starts_with(im.command.msg.components, s_buttons[static_cast<size_t>(preset_buttons_selected::ADDGROUP)]).custom_id.size() > 0) { // found GROUP related, it's group
//        button_mode = preset_buttons::ADDDELGROUP;
//    }
//    else { // must be then
//        button_mode = preset_buttons::ADDDEL;
//    }
//
//    get_message_data(im.command.msg);
//}
//
//button_work::button_work(const dpp::select_click_t& im)
//    : selected(im.values[0]) // selected row, custom_id is row ID, not selected on row.
//{  
//    base_id = remove_s_row_tag(im.custom_id);
//
//    if (im.command.msg.components.size() == 1) { // if has one and it's select, row only ofc
//        button_mode = preset_buttons::ROWONLY;
//    }
//    else if (find_component_starts_with(im.command.msg.components, s_buttons[static_cast<size_t>(preset_buttons_selected::SELECT)]).custom_id.size() > 0) { // found SELECT, must be select
//        button_mode = preset_buttons::BUTTONSELECT;
//    }
//    else if (find_component_starts_with(im.command.msg.components, s_buttons[static_cast<size_t>(preset_buttons_selected::ADDGROUP)]).custom_id.size() > 0) { // found GROUP related, it's group
//        button_mode = preset_buttons::ADDDELGROUP;
//    }
//    else { // must be then
//        button_mode = preset_buttons::ADDDEL;
//    }
//
//    get_message_data(im.command.msg);
//}
//
//button_work::preset_buttons_selected button_work::interpret_selected() const
//{
//    for(size_t p = 0; p < static_cast<size_t>(preset_buttons_selected::_SIZE); ++p) {
//        if (selected.find(s_buttons[static_cast<size_t>(p)]) == 0) return static_cast<preset_buttons_selected>(p);
//    }
//    return preset_buttons_selected::_SIZE;
//}
//
//bool button_work::get_disabled(const preset_buttons_selected k) const
//{
//    if (k != preset_buttons_selected::_SIZE) return disabled[static_cast<size_t>(k)];
//    return false;
//}
//
//void button_work::set_disabled(const preset_buttons_selected k, const bool e)
//{
//    if (k != preset_buttons_selected::_SIZE) disabled[static_cast<size_t>(k)] = e;
//}
//
//dpp::component button_work::generate_buttons() const
//{
//    if (base_id.empty()) throw std::runtime_error("CUSTOM_ID WAS EMPTY SOMEHOW, THIS IS NOT GOOD!");
//
//    dpp::component host;
//
//    switch(button_mode) {
//    case preset_buttons::BUTTONSELECT:
//    {
//        host.add_component(dpp::component()
//            .set_type(dpp::cot_button)
//            .set_style(dpp::cos_primary)
//            .set_label("Select")
//            .set_id(s_buttons[static_cast<size_t>(preset_buttons_selected::SELECT)] + base_id)
//            .set_disabled(get_disabled(preset_buttons_selected::SELECT))
//            .set_emoji("📝")
//        );
//        if (stored.size()) {
//            host.add_component(dpp::component()
//                .set_type(dpp::cot_button)
//                .set_style(dpp::cos_primary)
//                .set_label(stored)
//                .set_id(s_buttons[static_cast<size_t>(preset_buttons_selected::SELECT_CLICK)] + base_id)
//                .set_disabled(get_disabled(preset_buttons_selected::SELECT_CLICK))
//            );
//        }
//    }
//        break;
//    case preset_buttons::ADDDEL:
//    {
//        host.add_component(dpp::component()
//            .set_type(dpp::cot_button)
//            .set_style(dpp::cos_primary)
//            .set_label("Add")
//            .set_id(s_buttons[static_cast<size_t>(preset_buttons_selected::ADD)] + base_id)
//            .set_disabled(get_disabled(preset_buttons_selected::ADD))
//            .set_emoji("📝")
//        );
//        if (stored.size()) {
//            host.add_component(dpp::component()
//                .set_type(dpp::cot_button)
//                .set_style(dpp::cos_primary)
//                .set_label(stored)
//                .set_id(s_buttons[static_cast<size_t>(preset_buttons_selected::CUSTOMTAG)] + base_id)
//                .set_disabled(true)
//            );
//        }
//        host.add_component(dpp::component()
//            .set_type(dpp::cot_button)
//            .set_style(dpp::cos_danger)
//            .set_label("Remove")
//            .set_id(s_buttons[static_cast<size_t>(preset_buttons_selected::DEL)] + base_id)
//            .set_disabled(get_disabled(preset_buttons_selected::DEL))
//            .set_emoji("🗑️")
//        );
//    }
//        break;
//    case preset_buttons::ADDDELGROUP:
//    {
//        host.add_component(dpp::component()
//            .set_type(dpp::cot_button)
//            .set_style(dpp::cos_primary)
//            .set_label("Add group")
//            .set_id(s_buttons[static_cast<size_t>(preset_buttons_selected::ADDGROUP)] + base_id)
//            .set_disabled(get_disabled(preset_buttons_selected::ADDGROUP))
//            .set_emoji("📝")
//        );
//        host.add_component(dpp::component()
//            .set_type(dpp::cot_button)
//            .set_style(dpp::cos_danger)
//            .set_label("Remove group")
//            .set_id(s_buttons[static_cast<size_t>(preset_buttons_selected::DELGROUP)] + base_id)
//            .set_disabled(get_disabled(preset_buttons_selected::DELGROUP))
//            .set_emoji("🗑️")
//        );
//        if (stored.size()) {
//            host.add_component(dpp::component()
//                .set_type(dpp::cot_button)
//                .set_style(dpp::cos_primary)
//                .set_label(stored)
//                .set_id(s_buttons[static_cast<size_t>(preset_buttons_selected::CUSTOMTAG)] + base_id)
//                .set_disabled(true)
//            );
//        }
//        host.add_component(dpp::component()
//            .set_type(dpp::cot_button)
//            .set_style(dpp::cos_primary)
//            .set_label("Add item")
//            .set_id(s_buttons[static_cast<size_t>(preset_buttons_selected::ADD)] + base_id)
//            .set_disabled(get_disabled(preset_buttons_selected::ADD))
//            .set_emoji("📝")
//        );
//        host.add_component(dpp::component()
//            .set_type(dpp::cot_button)
//            .set_style(dpp::cos_danger)
//            .set_label("Remove item")
//            .set_id(s_buttons[static_cast<size_t>(preset_buttons_selected::DEL)] + base_id)
//            .set_disabled(get_disabled(preset_buttons_selected::DEL))
//            .set_emoji("🗑️")
//        );
//    }
//        break;
//    }
//
//    return host;
//}
//
//dpp::component button_work::generate_rows() const
//{
//    if (base_id.empty()) throw std::runtime_error("CUSTOM_ID WAS EMPTY SOMEHOW, THIS IS NOT GOOD!");
//
//    dpp::component rw;
//    rw.set_label("Select one");
//    rw.set_id(s_row + base_id);
//    rw.set_type(dpp::cot_selectmenu);
//    for(const auto& i : rows) rw.add_select_option(i);
//
//    return dpp::component().add_component(rw);
//}
//
//void button_work::replace_components_of(dpp::message& msg)
//{
//    msg.components.clear();
//    if (button_mode != preset_buttons::ROWONLY) msg.add_component(generate_buttons());
//    if (rows.size()) msg.add_component(generate_rows());
//}


button& button::set_kind(custom_kinds v)
{
    kind = v;
    return *this;
}

button& button::set_item_name(const std::string& v)
{
    if(unsafe_string_name(v)) throw std::invalid_argument("Value contains invalid characters! (button check)");
    item_name = v;
    return *this;
}

button& button::set_label(const std::string& v)
{
    label = v;
    return *this;
}

button& button::set_style(dpp::component_style v)
{
    style = v;
    return *this;
}

bool button::get_custom_as_bool() const
{
    return custom_value == "1" ? true : false;
}

dpp::snowflake button::get_custom_as_snowflake() const
{
    return dpp::from_string<dpp::snowflake>(custom_value);
}

const std::string& button::get_custom_as_string() const
{
    return custom_value;
}

button& button::set_custom(bool b)
{
    custom_value = b ? "1" : "0";
    kind = custom_kinds::SWITCH;
    return *this;
}

button& button::set_custom(dpp::snowflake v)
{
    custom_value = std::to_string(v);
    kind = custom_kinds::ID;
    return *this;
}

button& button::set_custom(const char* v)
{
    if(unsafe_string_name(v)) throw std::invalid_argument("Value contains invalid characters! (button check)");
    custom_value = v;
    kind = custom_kinds::STRING;
    return *this;
}

button& button::set_custom(const std::string& v)
{
    if(unsafe_string_name(v)) throw std::invalid_argument("Value contains invalid characters! (button check)");
    custom_value = v;
    kind = custom_kinds::STRING;
    return *this;
}

button::button(const std::string& lab, const std::string& nam)
    : label(lab), item_name(nam), kind(custom_kinds::UNDEF)
{
}

bool select_item::get_custom_as_bool() const
{
    return custom_value == "1" ? true : false;
}

dpp::snowflake select_item::get_custom_as_snowflake() const
{
    return dpp::from_string<dpp::snowflake>(custom_value);
}

const std::string& select_item::get_custom_as_string() const
{
    return custom_value;
}

select_item& select_item::set_custom(bool b)
{
    custom_value = b ? "1" : "0";
    kind = custom_kinds::SWITCH;
    return *this;
}

select_item& select_item::set_custom(dpp::snowflake v)
{
    custom_value = std::to_string(v);
    kind = custom_kinds::ID;
    return *this;
}

select_item& select_item::set_custom(const char* v)
{
    if(unsafe_string_name(v)) throw std::invalid_argument("Value contains invalid characters! (select_item check)");
    custom_value = v;
    kind = custom_kinds::STRING;
    return *this;
}

select_item& select_item::set_custom(const std::string& v)
{
    if(unsafe_string_name(v)) throw std::invalid_argument("Value contains invalid characters! (select_item check)");
    custom_value = v;
    kind = custom_kinds::STRING;
    return *this;
}

select_item::select_item(const std::string& a, const std::string& b, const std::string& c, const std::string& val)
    : label(a), item_name(b), desc(c), is_trigger(false), custom_value(val), kind(custom_kinds::STRING)
{
}

bool select_row::get_custom_as_bool() const
{
    return custom_value == "1" ? true : false;
}

dpp::snowflake select_row::get_custom_as_snowflake() const
{
    return dpp::from_string<dpp::snowflake>(custom_value);
}

const std::string& select_row::get_custom_as_string() const
{
    return custom_value;
}

select_row& select_row::set_custom(bool b)
{
    custom_value = b ? "1" : "0";
    kind = custom_kinds::SWITCH;
    return *this;
}

select_row& select_row::set_custom(dpp::snowflake v)
{
    custom_value = std::to_string(v);
    kind = custom_kinds::ID;
    return *this;
}

select_row& select_row::set_custom(const char* v)
{
    if(unsafe_string_name(v)) throw std::invalid_argument("Value contains invalid characters! (select_item check)");
    custom_value = v;
    kind = custom_kinds::STRING;
    return *this;
}

select_row& select_row::set_custom(const std::string& v)
{
    if(unsafe_string_name(v)) throw std::invalid_argument("Value contains invalid characters! (select_item check)");
    custom_value = v;
    kind = custom_kinds::STRING;
    return *this;
}

select_row::select_row(const std::vector<select_item>& v)
    : items(v)
{    
}

void transl_button_event::fill_from_msg(const dpp::message& m)
{
    message_content = m.content;

    for(const auto& i : m.components) {
        for(const auto& j : i.components) {
            switch(j.type) {
            case dpp::cot_selectmenu:
            {
                select_row _cur;

                for (const auto& opt : j.options) {
                    select_item item;

                    item.item_name = select_between(opt.value, ITEMNAME_WRAP);
                    item.set_custom(select_between(opt.value, CUSTOMVAL_WRAP)); // must be first because kind is set next.
                    item.kind = str_to_custom_kind(select_between(opt.value, KINDTYPE_WRAP));
                    
                    item.label = opt.label;
                    item.desc = opt.description;
                    item.is_trigger = (item.item_name == trigger_id);

                    _cur.items.push_back(item);
                }

                _cur.set_custom(select_between(j.custom_id, CUSTOMVAL_WRAP)); // must be first because kind is set next.
                _cur.kind = str_to_custom_kind(select_between(j.custom_id, KINDTYPE_WRAP));
                _cur.group_id = select_between(j.custom_id, GROUPID_WRAP);

                if (!_cur.items.empty()) select_vec.push_back(_cur);
            }
                break;
            case dpp::cot_button:
            {
                button item;

                item.set_custom(select_between(j.custom_id, CUSTOMVAL_WRAP)); // must be first because kind is set next.
                item.kind = str_to_custom_kind(select_between(j.custom_id, KINDTYPE_WRAP));

                item.group_id = select_between(j.custom_id, GROUPID_WRAP);
                item.item_name = select_between(j.custom_id, ITEMNAME_WRAP);
                item.is_trigger = (item.item_name == trigger_id);

                item.custom_emoji.id = j.emoji.id;
                item.custom_emoji.name = j.emoji.name;
                if (j.emoji.animated) item.custom_emoji.flags |= dpp::e_animated;

                item.style = j.style;
                item.label = j.label;

                button_vec.push_back(item);
            }
                break;
            }
        }
    }
}

transl_button_event::transl_button_event(const std::string& s)
    : group_id(s), custom_kind(custom_kinds::UNDEF)
{
}

transl_button_event::transl_button_event(const dpp::button_click_t& b)
    : ifbtnclick(&b), 
    group_id(select_between(b.custom_id, GROUPID_WRAP)),
    trigger_id(select_between(b.custom_id, ITEMNAME_WRAP)), 
    custom_value(select_between(b.custom_id, CUSTOMVAL_WRAP)),
    custom_kind(str_to_custom_kind(select_between(b.custom_id, KINDTYPE_WRAP)))
{
    fill_from_msg(b.command.msg);
}

transl_button_event::transl_button_event(const dpp::select_click_t& b)
    : ifslcclick(&b), group_id(select_between(b.custom_id, GROUPID_WRAP)), 
    trigger_id(b.values.size() ? select_between(b.values[0], ITEMNAME_WRAP) : ""),
    custom_value(b.values.size() ? select_between(b.values[0], CUSTOMVAL_WRAP) : ""),
    custom_kind(b.values.size() ? str_to_custom_kind(select_between(b.values[0], KINDTYPE_WRAP)) : custom_kinds::STRING)
{
    fill_from_msg(b.command.msg);
}

bool transl_button_event::push_button(button b)
{
    if (!can_push_button()) return false;

    b.group_id = group_id;
    b.is_trigger = false;
    
    button_vec.push_back(b);
    return true;
}

bool transl_button_event::push_select(select_row ro)
{
    if (!can_push_select()) return false;

    for(auto& i : ro.items) {
        i.is_trigger = false;
    }

    ro.group_id = group_id;

    select_vec.push_back(ro);
    return true;
}

std::vector<button>& transl_button_event::get_buttons()
{
    return button_vec;
}

const std::vector<button>& transl_button_event::get_buttons() const
{
    return button_vec;
}

custom_kinds transl_button_event::get_custom_kind() const
{
    return custom_kind;
}

bool transl_button_event::get_custom_as_bool() const
{
    return custom_value == "1";
}

dpp::snowflake transl_button_event::get_custom_as_snowflake() const
{
    return dpp::from_string<dpp::snowflake>(custom_value);
}

const std::string& transl_button_event::get_custom_as_string() const
{
    return custom_value;
}

std::vector<select_row>& transl_button_event::get_select()
{
    return select_vec;
}

const std::vector<select_row>& transl_button_event::get_select() const
{
    return select_vec;
}

void transl_button_event::set_content(const std::string& str)
{
    message_content = str;
}

const std::string& transl_button_event::get_content() const
{
    return message_content;
}

bool transl_button_event::can_push_button() const
{
    return ((5 * select_vec.size()) + button_vec.size()) < 25;
}

bool transl_button_event::can_push_select() const
{
    return ((5 * select_vec.size()) + button_vec.size()) <= 20; // 21 or more means buttons + select already occup 5 rows. 
}

bool transl_button_event::reply(const bool replac, std::function<void(dpp::message&)> dosmth) const
{
    dpp::message msg = generate_message();
    if (dosmth) dosmth(msg);

    const auto autoerr = [](const dpp::confirmation_callback_t& e){
        if (e.is_error()) {
            Lunaris::cout << Lunaris::console::color::DARK_PURPLE << e.get_error().message;
            Lunaris::cout << Lunaris::console::color::DARK_AQUA << e.http_info.body;
        }
    };

    if (replac) {
        if (ifbtnclick) ifbtnclick->reply(dpp::ir_update_message, msg, autoerr);
        if (ifslcclick) ifslcclick->reply(dpp::ir_update_message, msg, autoerr);
    }
    else {
        if (ifbtnclick) ifbtnclick->reply(msg, autoerr);
        if (ifslcclick) ifslcclick->reply(msg, autoerr);
    }

    return true;
}

dpp::message transl_button_event::generate_message() const
{
    dpp::message msg;
    msg.set_content(message_content);

    dpp::component actrow;

    for(const auto& i : button_vec) {
        dpp::component _cp;

        _cp.set_label(i.label)
            .set_id(
                GROUPID_WRAP + i.group_id + GROUPID_WRAP + 
                ITEMNAME_WRAP + i.item_name + ITEMNAME_WRAP + 
                KINDTYPE_WRAP + custom_kind_to_str(i.kind) + KINDTYPE_WRAP +
                CUSTOMVAL_WRAP + i.get_custom_as_string() + CUSTOMVAL_WRAP
                )
            .set_type(dpp::cot_button)
            .set_style(i.style);

        if (i.custom_emoji.id != 0 || !i.custom_emoji.name.empty())
            _cp.set_emoji(i.custom_emoji.format());

        actrow.add_component(_cp);

        if (actrow.components.size() >= 5) {
            msg.add_component(actrow);
            actrow.components.clear();
        }
    }

    if (actrow.components.size() > 0) {
        msg.add_component(actrow);
        actrow.components.clear();
    }

    for(const auto& j : select_vec) {
        dpp::component cmp;
        cmp.set_type(dpp::cot_selectmenu);
        cmp.custom_id = GROUPID_WRAP + j.group_id + GROUPID_WRAP +
            KINDTYPE_WRAP + custom_kind_to_str(j.kind) + KINDTYPE_WRAP +
            CUSTOMVAL_WRAP + j.get_custom_as_string() + CUSTOMVAL_WRAP;

        for(const auto& k : j.items) {
            cmp.add_select_option(dpp::select_option(k.label, 
                ITEMNAME_WRAP + k.item_name + ITEMNAME_WRAP + 
                KINDTYPE_WRAP + custom_kind_to_str(k.kind) + KINDTYPE_WRAP +
                CUSTOMVAL_WRAP + k.get_custom_as_string() + CUSTOMVAL_WRAP,
                k.desc));
        }

        msg.add_component(dpp::component().add_component(cmp));
    }

    msg.set_flags(64);

    return msg;
}


bool unsafe_string_name(const std::string& s)
{
    for(const auto& uns : BLOCKED_NAMING_CHARS) {
        if (s.find(uns) != std::string::npos) return true;
    }
    return false;
}

std::string custom_kind_to_str(const custom_kinds i)
{
    if (i != custom_kinds::UNDEF) return custom_kinds_str[static_cast<size_t>(i)];
    return "";
}

custom_kinds str_to_custom_kind(const std::string& i)
{
    for(size_t p = 0; p < std::size(custom_kinds_str); ++p){
        if (i == custom_kinds_str[p]) return static_cast<custom_kinds>(p);
    }
    return custom_kinds::UNDEF;
}

std::string bool_to_emoji(const bool v)
{
    return boolean_emojis[static_cast<size_t>(v)];
}

const bool emoji_to_bool(const std::string& s)
{
    return bool_to_emoji(true) == s;
}

std::string select_between(const std::string& e, const char c)
{
	const size_t a = e.find(c);
	if (a == std::string::npos) return{};
	const size_t b = e.find(c, a + 1);
	if (b == std::string::npos || b < a) return {};
	return e.substr(a + 1, b - (a + 1));
}

void lock_indefinitely()
{
    while(1) std::this_thread::sleep_for(std::chrono::seconds(3600));
}

std::vector<dpp::snowflake> slice_string_auto_snowflake(const std::string& str)
{
    if (str.empty()) return {};
    std::vector<dpp::snowflake> nd;

    const char* noww = str.data();
    char* endd = nullptr;

    while(1) {
        unsigned long long _val = 0;
        _val = std::strtoull(noww, &endd, 10);
        if (endd == noww) break;
        if (_val != 0) nd.push_back(_val);
        noww = endd;
    }

    return nd;
}

unsigned long long get_time_ms()
{
    return std::chrono::duration_cast<std::chrono::duration<unsigned long long, std::milli>>(std::chrono::system_clock::now().time_since_epoch()).count();
}

//dpp::component generate_button_auto(const preset_buttons pre, const std::string tagg, const std::string& middl, const std::vector<bool> disab)
//{
//    
//}