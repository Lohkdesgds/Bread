#include <specific_functions.hpp>
// SHOULD BE SAFE (MUTEXES)

void g_on_modal(const dpp::form_submit_t& ev)
{
    if (ev.command.usr.id != ev.command.msg.interaction.usr.id && ev.command.msg.interaction.usr.id != 0) {
        ev.reply(make_ephemeral_message("If you want to use this command, please trigger it yourself. This is not yours.")); 
        return;
    }
    
    transl_button_event wrk(ev);

    force_const<guild_info> guil = tf_guild_info[ev.command.guild_id];
    if (!guil) { ev.reply(make_ephemeral_message("Something went wrong! Guild do not exist?! Please report error! I'm so sorry.")); return; }

    std::unique_lock<std::shared_mutex> guilmtx(guil.unsafe().muu); // modals generally apply stuff. Assume write

    if (wrk.get_modal_items().empty()) { ev.reply(make_ephemeral_message("Something went wrong! Internal modal copy got LOST!")); return; }

    auto& trigg = wrk.get_trigger();

    if (trigg.group_name == "roles_roles") { // /roles

        size_t selected_group = 0;
        std::string selected_group_name;
        wrk.find_button_do("TMProlesconf", "selectedgroup", [&](item<button_props>& i){
            selected_group_name = i.get_custom_as_string();
            for(size_t p = 0; p < guil->roles_available.size(); ++p) {
                if (guil->roles_available[p].name == selected_group_name){
                    selected_group = p;
                    break;
                }
            }
        });

        // have selected_group and selected_group_name if has size().
        //std::string _feedback;

        if (trigg.item_name == "addg"){
            const std::string name = wrk.find_modal_get("name");
            const std::string limit = wrk.find_modal_get("limit");
            
            auto it = std::find_if(guil.unsafe().roles_available.begin(), guil.unsafe().roles_available.end(), [&](const guild_info::category& gc){ return gc.name == name; });
            if (it != guil.unsafe().roles_available.end()) {
                it->can_combine = (limit != "ONE");
                selected_group = std::distance(guil.unsafe().roles_available.begin(), it);
                selected_group_name = name;
                //_feedback = "Updated settings for `" + selected_group_name + "`, combinable? " + std::string((it->can_combine) ? "yes" : "no");
            }
            else {
                if (guil->roles_available.size() >= guild_props::max_role_groups) {
                    ev.reply(make_ephemeral_message("Something went wrong! Already full of role groups!"));
                    return;
                }
                guild_info::category cg;
                cg.name = name;
                cg.can_combine = (limit != "ONE");
                guil.unsafe().roles_available.push_back(cg);
                selected_group = guil->roles_available.size() - 1;
                selected_group_name = name;
                //_feedback = "Added new group `" + selected_group_name + "`!";
            }
        }
        else if (trigg.item_name == "delg"){
            const std::string name = wrk.find_modal_get("name");

            if (name == "*") {
                guil.unsafe().roles_available.clear();
                selected_group = 0;
                selected_group_name.clear();
                //_feedback = "Removed all groups!";
            }
            else {
                auto it = std::find_if(guil.unsafe().roles_available.begin(), guil.unsafe().roles_available.end(), [&](const guild_info::category& gc){ return gc.name == name; });
                if (it != guil.unsafe().roles_available.end()) {
                    //_feedback = "Removed group: `" + it->name + "`";
                    auto opt = guil.unsafe().roles_available.erase(it);
                    if (opt != guil.unsafe().roles_available.end()) {
                        selected_group = std::distance(guil.unsafe().roles_available.begin(), opt);
                        selected_group_name = opt->name;                    
                    }
                    else {
                        selected_group = 0;
                        selected_group_name.clear();
                    }
                }
            }
        }
        else if (trigg.item_name == "add"){
            if (guil->roles_available.size() == 0) {
                ev.reply(make_ephemeral_message("Something went wrong! Invalid operation. You must have a group to add a role in it!"));
                return;
            }

            const std::string role = wrk.find_modal_get("role");
            const std::string name = wrk.find_modal_get("name");

            if (selected_group >= guil->roles_available.size()){ ev.reply(make_ephemeral_message("Something went wrong! Invalid operation on expected valid one.")); return; }

            const dpp::snowflake _num = role.empty() ? 0 : dpp::from_string<dpp::snowflake>(role);
            if (_num == 0 || name.empty()) { ev.reply(make_ephemeral_message("Something went wrong! Invalid role ID or name is empty somehow!")); return; }

            auto& rg = guil.unsafe().roles_available[selected_group];

            auto it = std::find_if(rg.list.begin(), rg.list.end(), [&](const guild_info::pair_id_name& k){ return k.name == name || k.id == _num; });
            
            if (it != rg.list.end()) {
                //_feedback = "Updated role config: `" + name + "` -> `" + std::to_string(_num) + "`";
                if (it->id == _num) {
                    it->name = name;
                }
                else if (it->name == name) {
                    it->id = _num;
                }
            }
            else {
                if (rg.list.size() < guild_props::max_role_group_each) {
                    rg.list.push_back({_num, name});                    
                    //_feedback = "Added new role to config: `" + name + "` -> `" + std::to_string(_num) + "`";
                }
                else {
                    ev.reply(make_ephemeral_message("Something went wrong! You shouldn't be able to add a role on a full group!"));
                    return;
                }
            }
        }
        else if (trigg.item_name == "del"){
            if (guil->roles_available.size() == 0) {
                ev.reply(make_ephemeral_message("Something went wrong! Invalid operation. You must have a group to remove a role from it!"));
                return;
            }

            const std::string role = wrk.find_modal_get("role");

            if (selected_group >= guil->roles_available.size()){ ev.reply(make_ephemeral_message("Something went wrong! Invalid operation on expected valid one.")); return; }
                        
            const dpp::snowflake _num = role.empty() ? 0 : dpp::from_string<dpp::snowflake>(role);
            if (_num == 0 && role != "*") { ev.reply(make_ephemeral_message("Something went wrong! Invalid role ID!")); return; }

            auto& rg = guil.unsafe().roles_available[selected_group];

            if (role == "*") {
                rg.list.clear();
                //_feedback = "Removed all roles from this group.";
            }
            else {
                auto it = std::find_if(rg.list.begin(), rg.list.end(), [&](const guild_info::pair_id_name& k){ return k.id == _num; });
                if (it != rg.list.end()) {
                    //_feedback = "Removed role: `" + it->name + "` -> `" + std::to_string(it->id) + "`";
                    rg.list.erase(it);
                }
            }
        }

        wrk.remove_group_named_all("TMProlesconfgroup");

        select_row group;
        group.set_group_name("TMProlesconfgroup");
        for(const auto& i : guil->roles_available) {
            group.push_item(item<std::string>(i.name, i.name, "Select to configure or check"));
        }
        wrk.push_or_replace(group, 0);

        wrk.set_content(
            std::string("**__Role command setup__**\n") +
            "*Current groups: " + std::to_string(guil->roles_available.size()) + " of " + std::to_string(guild_props::max_role_groups) + "*\n" +
            ((guil->roles_available.size() > 0) ? ("*Roles in this group: " + std::to_string(guil->roles_available[selected_group].list.size()) + " of " + std::to_string(guild_props::max_role_group_each) + "*\n") : "") +
            std::string((guil->roles_available.size() > selected_group) ? 
                guil->roles_available[selected_group].render_string_block() :
                "```cs\nCreate a role group to render a group view\n```"
            )
        );

        wrk.find_button_do("TMProlesconf", "selectedgroup", [&](item<button_props>& i){
            i.set_custom(selected_group_name);
        });

        const bool can_addg = guil->roles_available.size() < guild_props::max_role_groups;
        const bool can_delg = guil->roles_available.size() > 0;
        const bool can_add = guil->roles_available.size() > selected_group ? (guil->roles_available[selected_group].list.size() < guild_props::max_role_group_each) : false;
        const bool can_del = guil->roles_available.size() > selected_group ? (guil->roles_available[selected_group].list.size() > 0) : false;

        wrk.find_button_do("TMProlesconf", "addg", [&](item<button_props>& i){ i.extra.disabled = !can_addg; });
        wrk.find_button_do("TMProlesconf", "delg", [&](item<button_props>& i){ i.extra.disabled = !can_delg; });
        wrk.find_button_do("TMProlesconf", "add",  [&](item<button_props>& i){ i.extra.disabled = !can_add; });
        wrk.find_button_do("TMProlesconf", "del",  [&](item<button_props>& i){ i.extra.disabled = !can_del; });

        wrk.reply(!guil->commands_public);
        return;

    }
    if (trigg.group_name == "roles_aroles") { // automatic
        const std::string role = wrk.find_modal_get("role"); // must exist
        const dpp::snowflake _num = role.empty() ? 0 : dpp::from_string<dpp::snowflake>(role);

        if (trigg.item_name == "add"){
            if (_num == 0) { ev.reply(make_ephemeral_message("Something went wrong! Invalid role ID!")); return; }
            if (guil->roles_when_join.size() >= guild_props::max_onjoin_roles_len) { ev.reply(make_ephemeral_message("Something went wrong! You shall not add more roles!")); return;}

            auto it = std::find(guil.unsafe().roles_when_join.begin(), guil.unsafe().roles_when_join.end(), _num);
            
            if (it == guil.unsafe().roles_when_join.end()) {
                guil.unsafe().roles_when_join.push_back(_num);
            }
        }
        if (trigg.item_name == "del"){
            if (_num == 0 && role != "*") { ev.reply(make_ephemeral_message("Something went wrong! Invalid role ID!")); return; }

            if (role == "*") {
                guil.unsafe().roles_when_join.clear();
            }
            else {
                auto it = std::find(guil.unsafe().roles_when_join.begin(), guil.unsafe().roles_when_join.end(), _num);
                if (it != guil.unsafe().roles_when_join.end()) guil.unsafe().roles_when_join.erase(it);
            }
        }

        std::string content;
        content = 
            "**__Automatic roles setup__**\n"
            "*Current role usage: " + std::to_string(guil->roles_when_join.size()) + " of " + std::to_string(guild_props::max_onjoin_roles_len) + "*\n";

        {
            content += "```cs\n";
            const auto& list = guil->roles_when_join;
            if (list.size() > 0) {
                dpp::cache<dpp::role>* cach = dpp::get_role_cache();
                std::shared_lock<std::shared_mutex> lu(cach->get_mutex());
                const auto& rols = cach->get_container();

                for(const auto& it : list) {
                    content += " " + std::to_string(it) + ": #";
                    auto found = std::find_if(rols.begin(), rols.end(), [&](const std::pair<dpp::snowflake, dpp::role*>& s){ return s.first == it;});
                    if (found != rols.end()) content += found->second->name;
                    content += "\n";
                }
            }
            else {
                content += " <empty list>\n";
            }
            content += "```";
        }

        wrk.set_content(content);

        const bool can_add = guil->roles_when_join.size() < guild_props::max_onjoin_roles_len;
        const bool can_del = guil->roles_when_join.size() > 0;

        wrk.find_button_do("TMParolesconf", "add",  [&](item<button_props>& i){ i.extra.disabled = !can_add; });
        wrk.find_button_do("TMParolesconf", "del",  [&](item<button_props>& i){ i.extra.disabled = !can_del; });

        wrk.reply(!guil->commands_public);
        return;
    }
    if (trigg.group_name == "roles_lroles") { // leveling
        const std::string role = wrk.find_modal_get("role"); // must exist
        const dpp::snowflake _num = role.empty() ? 0 : dpp::from_string<dpp::snowflake>(role);

        if (trigg.item_name == "add"){
            const std::string level = wrk.find_modal_get("level"); // can be null
            const dpp::snowflake _lvl = dpp::from_string<unsigned long long>(level);
            if (_num == 0 || _lvl == 0) { ev.reply(make_ephemeral_message("Something went wrong! Invalid role ID or level!")); return; }
            if (guil->role_per_level.size() >= guild_props::max_leveling_roles_len) { ev.reply(make_ephemeral_message("Something went wrong! You shall not add more roles!")); return;}
            
            auto it = std::find_if(guil.unsafe().role_per_level.begin(), guil.unsafe().role_per_level.end(), [&](const guild_info::pair_id_level& p) {return p.id == _num || p.level == _lvl; });

            if (it != guil.unsafe().role_per_level.end()) {
                it->id = _num;
                it->level = _lvl;
            }
            else {
                guil.unsafe().role_per_level.push_back({_num, _lvl});
            }
        }
        if (trigg.item_name == "del"){
            if (_num == 0 && role != "*") { ev.reply(make_ephemeral_message("Something went wrong! Invalid role ID!")); return; }

            if (role == "*") {
                guil.unsafe().role_per_level.clear();
            }
            else {
                auto it = std::find_if(guil.unsafe().role_per_level.begin(), guil.unsafe().role_per_level.end(), [&](const guild_info::pair_id_level& p) {return p.id == _num; });
                if (it != guil.unsafe().role_per_level.end()) guil.unsafe().role_per_level.erase(it);
            }
        }

        std::sort(guil.unsafe().role_per_level.begin(), guil.unsafe().role_per_level.end(), [&](const guild_info::pair_id_level& a, const guild_info::pair_id_level& b) { return a.level < b.level; });

        std::string content;
        content = 
            "**__Leveling role setup__**\n"
            "*Current role usage: " + std::to_string(guil->role_per_level.size()) + " of " + std::to_string(guild_props::max_leveling_roles_len) + "*\n";

        {
            content += "```cs\n";
            const auto& list = guil->role_per_level;
            if (list.size() > 0) {
                dpp::cache<dpp::role>* cach = dpp::get_role_cache();
                std::shared_lock<std::shared_mutex> lu(cach->get_mutex());
                const auto& rols = cach->get_container();

                for(const auto& it : list) {
                    content += " Level " + std::to_string(it.level) + " -> " + std::to_string(it.id) + ": #";
                    auto found = std::find_if(rols.begin(), rols.end(), [&](const std::pair<dpp::snowflake, dpp::role*>& s){ return s.first == it.id;});
                    if (found != rols.end()) content += found->second->name;
                    content += "\n";
                }
            }
            else {
                content += " <empty list>\n";
            }
            content += "```";
        }

        wrk.set_content(content);

        const bool can_add = guil->role_per_level.size() < guild_props::max_leveling_roles_len;
        const bool can_del = guil->role_per_level.size() > 0;

        wrk.find_button_do("TMPlrolesconf", "add",  [&](item<button_props>& i){ i.extra.disabled = !can_add; });
        wrk.find_button_do("TMPlrolesconf", "del",  [&](item<button_props>& i){ i.extra.disabled = !can_del; });

        wrk.reply(!guil->commands_public);
        return;
    }
    if (trigg.group_name == "pointsconf")
    {
        if (trigg.item_name == "select") { // select user
            const std::string __in = wrk.find_modal_get("select");
            const dpp::snowflake _num = __in.empty() ? 0 : dpp::from_string<dpp::snowflake>(__in);

            if (_num == 0) {
                wrk.find_button_do("TMPpointsconf", "select", [&](item<button_props>& i){
                    i.set_label("Invalid ID");
                    i.set_custom(dpp::snowflake(0));
                    i.extra.style = dpp::cos_danger;
                });
                wrk.find_button_do("TMPpointsconf", "prtpts", [&](item<button_props>& i){
                    i.set_label("Guild points: nan");
                });
                wrk.find_button_do("TMPpointsconf", "setpts", [&](item<button_props>& i){
                    i.extra.disabled = true;
                    i.extra.style = dpp::cos_secondary;
                });
            }
            else {
                wrk.find_button_do("TMPpointsconf", "select", [&](item<button_props>& i){
                    i.set_label("Selected!");// + std::to_string(_num));
                    i.set_custom(_num);
                    i.extra.style = dpp::cos_success;
                });
                wrk.find_button_do("TMPpointsconf", "setpts", [&](item<button_props>& i){
                    i.extra.disabled = false;
                    i.extra.style = dpp::cos_primary;
                });

                force_const<user_info> you = tf_user_info[_num];
                if (!you) { ev.reply(make_ephemeral_message("Something went wrong! Can't get user?! Please report error! I'm so sorry.")); return; }                

                std::shared_lock<std::shared_mutex> lu(you.unsafe().muu);

                wrk.find_button_do("TMPpointsconf", "prtpts", [&](item<button_props>& i){
                    i.set_label("Guild points: " + std::to_string(you->get_points_on_guild(ev.command.guild_id)));
                });
            }
            wrk.reply(!guil->commands_public);
            return;
        }
        else if (trigg.item_name == "setpts") { // set user points
            dpp::snowflake target = 0;

            wrk.find_button_do("TMPpointsconf", "select", [&](item<button_props>& i){
                target = i.get_custom_as_snowflake();
            });
            
            if (target == 0) {
                wrk.find_button_do("TMPpointsconf", "select", [&](item<button_props>& i){
                    i.set_label("ID got invalid");
                    i.set_custom(dpp::snowflake(0));
                    i.extra.style = dpp::cos_danger;
                });
                wrk.find_button_do("TMPpointsconf", "prtpts", [&](item<button_props>& i){
                    i.set_label("Guild points: nan");
                });
                wrk.find_button_do("TMPpointsconf", "setpts", [&](item<button_props>& i){
                    i.extra.disabled = true;
                    i.extra.style = dpp::cos_danger;
                });
            }
            else {
                const unsigned long long _fin = dpp::from_string<unsigned long long>(wrk.find_modal_get("points"));
                
                force_const<user_info> you = tf_user_info[target];
                if (!you) { ev.reply(make_ephemeral_message("Something went wrong! Can't get user?! Please report error! I'm so sorry.")); return; }                

                std::unique_lock<std::shared_mutex> lu(you.unsafe().muu);

                you.unsafe().points_per_guild[ev.command.guild_id] = _fin;

                wrk.find_button_do("TMPpointsconf", "prtpts", [&](item<button_props>& i){
                    i.set_label("Guild points: " + std::to_string(you.unsafe().get_points_on_guild(ev.command.guild_id)));
                });
                wrk.find_button_do("TMPpointsconf", "setpts", [&](item<button_props>& i){
                    i.extra.disabled = false;
                    i.extra.style = dpp::cos_success;
                });
            }
            wrk.reply(!guil->commands_public);
            return;
        }
    }
    if (trigg.group_name == "selfconf") {
        force_const<user_info> you = tf_user_info[ev.command.usr.id];
        if (!you) { ev.reply(make_ephemeral_message("Something went wrong! You do not exist?! Please report error! I'm so sorry.")); return; }

        std::unique_lock<std::shared_mutex> lu(you.unsafe().muu);

        if (trigg.item_name == "colorpicker") {
            const std::string _raw = wrk.find_modal_get("color"); // must be 1
            const int64_t sel = interpret_color(_raw);
            you.unsafe().pref_color = sel;

            wrk.find_button_do("selfconf", "setcolor", [&](item<button_props>& i){
                i.extra.style = dpp::cos_success;
                i.label = "Updated: " + print_hex_color_auto(you->pref_color);
            });

            wrk.reply(!guil->commands_public);
            return;
        }
    }
    if (trigg.group_name == "commconf") {

        if (trigg.item_name == "levelch") {
            const std::string _raw = wrk.find_modal_get("chid"); // must be 1
            const dpp::snowflake sel = dpp::from_string<dpp::snowflake>(_raw);
            guil.unsafe().fallback_levelup_message_channel = sel;

            wrk.find_button_do("TMPcommconf", "levelch", [&](item<button_props>& i){
                i.extra.style = dpp::cos_success;
                i.set_label("Value set: " + std::string(sel == 0 ? "NONE" : std::to_string(sel)));
            });

            wrk.reply(!guil->commands_public);
            return;
        }
    }
    if (trigg.group_name == "poll") {
        if (trigg.item_name == "create") {
            force_const<user_info> you = tf_user_info[ev.command.usr.id];
            if (!you) {
                ev.reply(make_ephemeral_message("Something went wrong! You do not exist?! Please report error! I'm so sorry."));
                return;
            }

            std::shared_lock<std::shared_mutex> lu(you.unsafe().muu);

            const std::string title = get_customid_as_str(ev.components, "title"); // a must have
            const std::string desc = get_customid_as_str(ev.components, "desc");
            const std::string emojis = get_customid_as_str(ev.components, "emojis");
            const std::string imglink = get_customid_as_str(ev.components, "imglink");
            const std::string color = get_customid_as_str(ev.components, "color");

            auto emojis_selected = extract_emojis_auto(emojis);
            if (emojis_selected.size() == 0) { emojis_selected.push_back(u8"👍"); emojis_selected.push_back(u8"👎"); }
            const int64_t transl_clr = interpret_color(color);

            dpp::message replying;
            dpp::embed poll_enq;
            dpp::embed_author author;

            author.name = ev.command.usr.format_username();
            author.icon_url = ev.command.usr.get_avatar_url(256);

            poll_enq.set_author(author);
            poll_enq.set_title(title);
            if (desc.size()) poll_enq.set_description(desc);
            poll_enq.set_color((transl_clr < 0 ? (you->pref_color < 0 ? random() : you->pref_color) : transl_clr));
            poll_enq.set_thumbnail(images::poll_image_url);
            if (imglink.size()) poll_enq.set_image(imglink);

            replying.add_embed(poll_enq);

            const std::function<void(const dpp::confirmation_callback_t&)> dumb_var_copy_test = [dmbcore = ev.from->creator, emojis_selected](const dpp::confirmation_callback_t& data) {
                if (data.is_error()) {
                    cout << console::color::DARK_RED << "Someone called /poll and had issues: " << data.get_error().message;
                    return;
                }
                dpp::message msg = std::get<dpp::message>(data.value);
                for(auto& i : emojis_selected) {
                    dmbcore->message_add_reaction(msg, i);
                }
            };

            ev.reply(replying, [ev, dumb_var_copy_test](const dpp::confirmation_callback_t& conf){
                if (conf.is_error()) {
                    cout << console::color::DARK_RED << "Someone called /poll and had issues: " << conf.get_error().message;
                    return;
                }

                ev.get_original_response(dumb_var_copy_test);
            });

            return;
        }
    }
    if (trigg.group_name == "paste") {
        if (trigg.item_name == "post") {
            force_const<user_info> you = tf_user_info[ev.command.usr.id];
            if (!you) { ev.reply(make_ephemeral_message("Something went wrong! You do not exist?! Please report error! I'm so sorry.")); return; }
            force_const<guild_info> guil = tf_guild_info[ev.command.guild_id];
            if (!guil) { ev.reply(make_ephemeral_message("Something went wrong! Guild do not exist?! Please report error! I'm so sorry.")); return; }

            std::shared_lock<std::shared_mutex> lg(guil.unsafe().muu);
            std::shared_lock<std::shared_mutex> lu(you.unsafe().muu);

            auto& clipboard = you->clipboard;

            if (clipboard.guild_id != ev.command.guild_id && !guil->allow_external_paste){ ev.reply(make_ephemeral_message("This server doesn't allow paste from external sources.")); return; }

            const std::string paste_content = get_customid_as_str(ev.components, "comment");
                        
            dpp::message replying;
            ev.reply(dpp::interaction_response_type::ir_deferred_channel_message_with_source, replying);

            ev.from->creator->message_get(clipboard.message_id, clipboard.channel_id, 
            [ev, you, guil, replying, paste_content](const dpp::confirmation_callback_t data) mutable {
                auto& clipboard = you->clipboard;

                if (data.is_error()) {
                    ev.edit_response(make_ephemeral_message("I couldn't get clipboard source! Is it not available anymore? Maybe I just can't read it."));
                    //you->clipboard.clear();
                    return;
                }

                dpp::message sourcemsg = std::get<dpp::message>(data.value);

                dpp::embed emb;
                dpp::embed_author authr;
                authr.icon_url = images::url_author_icon_clipboard;
                authr.name = "Access source directly";
                authr.url = clipboard.generate_url();

                dpp::embed_footer emb_footer;
                emb_footer.set_text("Referenced by " + ev.command.usr.format_username() +
                    ", Source: " + sourcemsg.author.format_username() +
                    ", Guild #" + std::to_string(clipboard.guild_id));

                emb_footer.set_icon(ev.command.usr.get_avatar_url(256));

                emb.set_author(authr);
                emb.set_color((you->pref_color < 0 ? random() : you->pref_color));
                emb.set_footer(emb_footer);

                if (!paste_content.empty()){
                    emb.description += "`Commented:`\n```\n" + dpp::utility::markdown_escape(paste_content.substr(0, 280), true) + "```\n";
                }

                if (!sourcemsg.content.empty()) {
                    emb.description += "`Original text:`\n```\n";
                    for (const auto& i : sourcemsg.content) { if (i != '`') emb.description += i; }
                    emb.description += "```\n";
                }
                else {
                    emb.description += "`Original message has no text`\n";
                }

                if (sourcemsg.attachments.size()) {
                    emb.set_image(sourcemsg.attachments[0].url);
                }

                if (sourcemsg.embeds.size()) emb.description += "`Original message had embeds`";

                replying.add_embed(emb);

                // this is not really a cool thing
                //for(const auto& i : sourcemsg.embeds) replying.add_embed(i);

                ev.edit_response(replying);
            });
            return;
        }
    }


    ev.reply(make_ephemeral_message("Something went wrong! Track: on_modal > ?"));
    return;
}