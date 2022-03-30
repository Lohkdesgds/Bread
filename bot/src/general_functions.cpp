#include <general_functions.hpp>

select_row& select_row::set_group_name(const std::string& nnam)
{
    if(unsafe_string_name(nnam)) throw std::invalid_argument("Unsafe name! May break stuff!");
    group_name = nnam;
    return *this;
}

select_row& select_row::push_item(item<std::string> ni)
{
    if (items.size() == 25) throw std::overflow_error("Too many items!");
    items.push_back(ni);
    return *this;
}

void select_row::build(dpp::component src)
{
    if (src.components.empty()) return;
    const dpp::component& d = src.components[0]; // actionrow with select must have one select only.

    for(const auto& opt : d.options) {
        item<std::string> ni;

        ni.import_id(opt.value);
        ni.label = opt.label;
        ni.extra = opt.description;

        ni.custom_emoji.id = opt.emoji.id;
        ni.custom_emoji.name = opt.emoji.name;
        ni.custom_emoji.flags = (opt.emoji.animated) ? dpp::e_animated : 0;

        push_item(ni);
    }

    group_name = d.custom_id;
}

dpp::component select_row::dump() const
{
    dpp::component d;
    d.set_type(dpp::cot_selectmenu);

    for(const auto& opt : items) {
        dpp::select_option sel;

        sel.value = opt.export_id();
        sel.label = opt.label;
        sel.description = opt.extra;

        if (!opt.custom_emoji.name.empty()) {
            sel.emoji.id = opt.custom_emoji.id;
            sel.emoji.name = opt.custom_emoji.name;
            sel.emoji.animated = opt.custom_emoji.flags & dpp::e_animated;
        }

        d.add_select_option(sel);
    }

    d.custom_id = group_name;

    return dpp::component().add_component(d); // actionrow containing this selectmenu
}

bool select_row::can_dump() const
{
    return items.size();
}


button_row& button_row::set_group_name(const std::string& nnam)
{
    if(unsafe_string_name(nnam)) throw std::invalid_argument("Unsafe name! May break stuff!");
    group_name = nnam;
    return *this;
}

button_row& button_row::push_item(item<button_props> ni)
{
    if (items.size() >= 5) throw std::overflow_error("Too many items!");
    items.push_back(ni);
    return *this;
}

// component with options
void button_row::build(dpp::component src)
{
    for(const auto& d : src.components) // each component is button. max 5 buttons.
    {
        item<button_props> ni;

        ni.import_id(d.custom_id);
        if (group_name.empty()) group_name = select_between(d.custom_id, GROUPID_WRAP);
        ni.label = d.label;
        ni.extra = { d.style, d.disabled };

        ni.custom_emoji.id = d.emoji.id;
        ni.custom_emoji.name = d.emoji.name;
        ni.custom_emoji.flags = (d.emoji.animated) ? dpp::e_animated : 0;

        push_item(ni);
    }
}

dpp::component button_row::dump() const
{
    dpp::component d;

    for(const auto& opt : items) {
        dpp::component btn;

        btn.set_type(dpp::cot_button);
        // must have group name inside because buttons are one by one :(
        btn.set_id(GROUPID_WRAP + group_name + GROUPID_WRAP + opt.export_id());
        btn.set_label(opt.label);
        btn.set_style(opt.extra.style);
        btn.set_disabled(opt.extra.disabled);

        if (!opt.custom_emoji.name.empty()) {
            btn.emoji.id = opt.custom_emoji.id;
            btn.emoji.name = opt.custom_emoji.name;
            btn.emoji.animated = opt.custom_emoji.flags & dpp::e_animated;
        }

        d.add_component(btn);
    }

    return d; // d is already actionrow of button(s).
}

bool button_row::can_dump() const
{
    return items.size();
}


std::string transl_button_event::trigger_info::debug_value_as_string() const
{
    if (std::holds_alternative<dpp::snowflake>(value)) return "ID=" + std::to_string(std::get<dpp::snowflake>(value));
    if (std::holds_alternative<bool>(value)) return "SWITCH=" + std::string(std::get<bool>(value) ? "true" : "false");
    if (std::holds_alternative<std::string>(value)) return "STRING=" + (std::get<std::string>(value));
    return "UNDEF";
}

void transl_button_event::fill_from_msg(const dpp::message& m)
{
    message_content = m.content;

    for(const auto& i : m.components) {
        if (i.components.size() == 0) continue;

        dpp::component_type objs_type = i.components[0].type; // internal is what? button? selectmenu?

        switch(objs_type) {
        case dpp::cot_selectmenu:
        {
            select_row _cur;
            _cur.build(i); // i is actionrow
            rows.push_back(_cur);
        }
            break;
        case dpp::cot_button:
        {
            button_row _cur;
            _cur.build(i);
            rows.push_back(_cur);
        }
            break;
        }
    }
}

void transl_button_event::link_refs()
{
    for(auto& it : rows) {
        if (std::holds_alternative<select_row>(it)){
            select_row& ref = std::get<select_row>(it);
            if (ref.group_name != trigg.group_name) continue;
            for(auto& ea : ref.items) {
                if (ea.name == trigg.item_name) {
                    trigg.target_if_select = &ea;
                    trigg.target_if_button = nullptr;
                    return;
                }
            }
        }
        else {
            button_row& ref = std::get<button_row>(it);
            if (ref.group_name != trigg.group_name) continue;
            for(auto& ea : ref.items) {
                if (ea.name == trigg.item_name) {
                    trigg.target_if_button = &ea;
                    trigg.target_if_select = nullptr;
                    return;
                }
            }
        }
    }
}

transl_button_event::transl_button_event(const dpp::button_click_t& b)
    : ifbtnclick(&b)
{
    fill_from_msg(b.command.msg);

    trigg.group_name = select_between(b.custom_id, GROUPID_WRAP);
    trigg.item_name = select_between(b.custom_id, ITEMNAME_WRAP);

    const custom_kinds currkind = str_to_custom_kind(select_between(b.custom_id, KINDTYPE_WRAP));
    const std::string currval = select_between(b.custom_id, CUSTOMVAL_WRAP);

    switch(currkind) {
    case custom_kinds::SWITCH:
        trigg.value = (currval == "1");
        break;
    case custom_kinds::STRING:
        trigg.value = currval;
        break;
    case custom_kinds::ID:
        trigg.value = dpp::from_string<dpp::snowflake>(currval);
        break;
    default:
        trigg.value = std::monostate();
    }
    
    link_refs();
}

transl_button_event::transl_button_event(const dpp::select_click_t& b)
    : ifslcclick(&b)
{
    fill_from_msg(b.command.msg);

    trigg.group_name = b.custom_id;
    if (b.values.size()) {
        trigg.item_name = select_between(b.values[0], ITEMNAME_WRAP);
        const custom_kinds currkind = str_to_custom_kind(select_between(b.values[0], KINDTYPE_WRAP));
        const std::string currval = select_between(b.values[0], CUSTOMVAL_WRAP);

        switch(currkind) {
        case custom_kinds::SWITCH:
            trigg.value = (currval == "1");
            break;
        case custom_kinds::STRING:
            trigg.value = currval;
            break;
        case custom_kinds::ID:
            trigg.value = dpp::from_string<dpp::snowflake>(currval);
            break;
        default:
            trigg.value = std::monostate();
        }
    }

    link_refs();
}


transl_button_event::transl_button_event(const dpp::form_submit_t& b)
    : iffrmclick(&b)
{
    fill_from_msg(b.command.msg);
    trigg.item_name = select_between(b.custom_id, ITEMNAME_WRAP);
    trigg.group_name = select_between(b.custom_id, GROUPID_WRAP);
    trigg.value = std::monostate{};

    for(const auto& eachform : b.components)
    {
        for(const auto& i : eachform.components) {
            if (i.type != dpp::cot_text) continue; // can only be 4

            if (std::holds_alternative<std::string>(i.value)) modal_response.push_back({i.custom_id, std::get<std::string>(i.value)});
            else if (std::holds_alternative<int64_t>(i.value)) modal_response.push_back({i.custom_id, std::to_string(std::get<int64_t>(i.value))});
            else if (std::holds_alternative<double>(i.value)) modal_response.push_back({i.custom_id, std::to_string(std::get<double>(i.value))});
        }
    }

    link_refs();
}

bool transl_button_event::push_or_replace(button_row b, const size_t p)
{
    if (!can_push()) return false;

    for(auto& it : rows) {
        if (std::holds_alternative<button_row>(it)){
            button_row& ref = std::get<button_row>(it);
            if (ref.group_name == b.group_name){
                ref = b;
                return true;
            }
        }
    }

    if (p >= rows.size()) rows.push_back(b);
    else rows.insert(rows.begin() + p, b);
    return true;
}

bool transl_button_event::push_or_replace(select_row b, const size_t p)
{
    if (!can_push()) return false;

    for(auto& it : rows) {
        if (std::holds_alternative<select_row>(it)){
            select_row& ref = std::get<select_row>(it);
            if (ref.group_name == b.group_name){
                ref = b;
                return true;
            }
        }
    }
    
    if (p >= rows.size()) rows.push_back(b);
    else rows.insert(rows.begin() + p, b);
    return true;
}

std::vector<std::variant<select_row, button_row>>& transl_button_event::get_rows()
{
    return rows;
}

const std::vector<std::variant<select_row, button_row>>& transl_button_event::get_rows() const
{
    return rows;
}

bool transl_button_event::find_button_do(const std::string& group, const std::string& name, std::function<void(item<button_props>&)> f)
{
    if (!f) return false;
    for(auto& it : rows) {
        if (std::holds_alternative<button_row>(it)){
            button_row& ref = std::get<button_row>(it);
            if (ref.group_name != group) continue;
            for(auto& ea : ref.items) {
                if (ea.name == name) {
                    f(ea);
                    return true;
                }
            }
        }
    }
    return false;
}

bool transl_button_event::find_select_do(const std::string& group, const std::string& name, std::function<void(item<std::string>&)> f)
{
    if (!f) return false;
    for(auto& it : rows) {
        if (std::holds_alternative<select_row>(it)){
            select_row& ref = std::get<select_row>(it);
            if (ref.group_name != group) continue;
            for(auto& ea : ref.items) {
                if (ea.name == name) {
                    f(ea);
                    return true;
                }
            }
        }
    }
    return false;
}

const std::vector<std::pair<std::string, std::string>>& transl_button_event::get_modal_items() const
{
    return modal_response;
}

bool transl_button_event::remove_group_named(const std::string& srch, std::function<bool(size_t, size_t)> frule)
{
    if (!frule) return false;
    
    for(auto it  = rows.begin(); it != rows.end();) {
        if (std::holds_alternative<button_row>(*it))
        {
            button_row& i = std::get<button_row>(*it);
            size_t fin_beg = i.group_name.find(srch);
            if (fin_beg != std::string::npos){
                if (frule(fin_beg, (i.group_name.size() - fin_beg - srch.size()))) {
                    rows.erase(it);
                    return true;
                }
            }
            ++it;
        }
        else {
            select_row& i = std::get<select_row>(*it);
            size_t fin_beg = i.group_name.find(srch);
            if (fin_beg != std::string::npos){
                if (frule(fin_beg, (i.group_name.size() - fin_beg - srch.size()))) {
                    rows.erase(it);
                    return true;
                }
            }
            ++it;
        }
    }
    return false;
}

bool transl_button_event::remove_group_named_all(const std::string& srch, std::function<bool(size_t, size_t)> frule)
{
    if (!remove_group_named(srch, frule)) return false;
    while (remove_group_named(srch, frule));
    return true;
}

void transl_button_event::set_content(const std::string& str)
{
    message_content = str;
}

const std::string& transl_button_event::get_content() const
{
    return message_content;
}

const transl_button_event::trigger_info& transl_button_event::get_trigger() const
{
    return trigg;
}

bool transl_button_event::has_valid_ref() const
{
    return trigg.target_if_button || trigg.target_if_select;
}

bool transl_button_event::can_push() const
{
    return rows.size() < 5;
}

bool transl_button_event::reply(const bool silent, const bool replac, std::function<void(dpp::message&)> dosmth) const
{
    dpp::message msg = generate_message(silent);
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
        if (iffrmclick) iffrmclick->reply(dpp::ir_update_message, msg, autoerr);
    }
    else {
        if (ifbtnclick) ifbtnclick->reply(msg, autoerr);
        if (ifslcclick) ifslcclick->reply(msg, autoerr);
        if (iffrmclick) iffrmclick->reply(msg, autoerr);
    }

    return true;
}

dpp::message transl_button_event::generate_message(const bool silent) const
{
    dpp::message msg;
    msg.set_content(message_content);

    for(const auto& row : rows)
    {
        if (std::holds_alternative<button_row>(row)) // button row
        {
            const button_row& dis = std::get<button_row>(row);
            if (dis.can_dump()) msg.add_component(dis.dump());
        }
        else { // select row
            const select_row& dis= std::get<select_row>(row);
            if (dis.can_dump()) msg.add_component(dis.dump());
        }
    }

    if (silent) msg.set_flags(dpp::m_ephemeral);

    return msg;
}

//dpp::component make_fancy_modal(const std::string& label, const std::string& groupid, const std::string& itemid)
//{
//    dpp::component nd;
//    nd.set_label(label);
//    nd.set_id(
//        (groupid.empty() ? "" : (GROUPID_WRAP + groupid + GROUPID_WRAP)) +
//        (itemid.empty() ? "" : (ITEMNAME_WRAP + itemid + ITEMNAME_WRAP))
//    );
//    nd.set_type(dpp::cot_text);
//    return nd;
//}


dpp::interaction_modal_response modal_generate(const std::string& groupid, const std::string itemid, const std::string& title)
{
    return dpp::interaction_modal_response(GROUPID_WRAP + groupid + GROUPID_WRAP + ITEMNAME_WRAP + itemid + ITEMNAME_WRAP, title);
}

dpp::component& modal_add_component(dpp::interaction_modal_response& modal, const std::string& label,
    const std::string id, const std::string placeholder, dpp::text_style_type styl, uint32_t min, uint32_t max)
{
    modal.add_component(
        dpp::component()
            .set_label(label)
            .set_id(id)
            .set_placeholder(placeholder)
            .set_text_style(styl)
            .set_min_length(min)
            .set_max_length(max)
    );
    return modal.components.back().back();
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