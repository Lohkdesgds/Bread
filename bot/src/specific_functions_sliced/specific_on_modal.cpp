#include <specific_functions.hpp>

void g_on_modal(const dpp::form_submit_t& ev)
{
    transl_button_event wrk(ev);

    const auto guil = tf_guild_info[ev.command.guild_id];
    if (!guil) { ev.reply(make_ephemeral_message("Something went wrong! Guild do not exist?! Please report error! I'm so sorry.")); return; }

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
            
            auto it = std::find_if(guil->roles_available.begin(), guil->roles_available.end(), [&](const guild_info::category& gc){ return gc.name == name; });
            if (it != guil->roles_available.end()) {
                it->can_combine = (limit != "ONE");
                selected_group = std::distance(guil->roles_available.begin(), it);
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
                guil->roles_available.push_back(cg);
                selected_group = guil->roles_available.size() - 1;
                selected_group_name = name;
                //_feedback = "Added new group `" + selected_group_name + "`!";
            }
        }
        else if (trigg.item_name == "delg"){
            const std::string name = wrk.find_modal_get("name");

            if (name == "*") {
                guil->roles_available.clear();
                selected_group = 0;
                selected_group_name.clear();
                //_feedback = "Removed all groups!";
            }
            else {
                auto it = std::find_if(guil->roles_available.begin(), guil->roles_available.end(), [&](const guild_info::category& gc){ return gc.name == name; });
                if (it != guil->roles_available.end()) {
                    //_feedback = "Removed group: `" + it->name + "`";
                    auto opt = guil->roles_available.erase(it);
                    if (opt != guil->roles_available.end()) {
                        selected_group = std::distance(guil->roles_available.begin(), opt);
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

            const dpp::snowflake _num = dpp::from_string<dpp::snowflake>(role);
            if (_num == 0 || name.empty()) { ev.reply(make_ephemeral_message("Something went wrong! Invalid role ID or name is empty somehow!")); return; }

            auto& rg = guil->roles_available[selected_group];

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
                        
            const dpp::snowflake _num = dpp::from_string<dpp::snowflake>(role);
            if (_num == 0 && role != "*") { ev.reply(make_ephemeral_message("Something went wrong! Invalid role ID!")); return; }

            auto& rg = guil->roles_available[selected_group];

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

        if (trigg.item_name == "add"){

        }
        if (trigg.item_name == "del"){

        }
    }
    if (trigg.group_name == "roles_lroles") { // leveling
        const std::string role = wrk.find_modal_get("role"); // must exist

        if (trigg.item_name == "add"){
            const std::string level = wrk.find_modal_get("level"); // can be null
            
        }
        if (trigg.item_name == "del"){

        }
    }
    if (trigg.group_name == "pointsconf")
    {
        if (trigg.item_name == "select") { // select user
            const dpp::snowflake _num = dpp::from_string<dpp::snowflake>(wrk.get_modal_items()[0].second);

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

                const auto you = tf_user_info[_num];
                if (!you) { ev.reply(make_ephemeral_message("Something went wrong! Can't get user?! Please report error! I'm so sorry.")); return; }                

                wrk.find_button_do("TMPpointsconf", "prtpts", [&](item<button_props>& i){
                    i.set_label("Guild points: " + std::to_string(you->points_per_guild[ev.command.guild_id]));
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
                const unsigned long long _fin = dpp::from_string<unsigned long long>(wrk.get_modal_items()[0].second);
                
                const auto you = tf_user_info[target];
                if (!you) { ev.reply(make_ephemeral_message("Something went wrong! Can't get user?! Please report error! I'm so sorry.")); return; }                

                you->points_per_guild[ev.command.guild_id] = _fin;

                wrk.find_button_do("TMPpointsconf", "prtpts", [&](item<button_props>& i){
                    i.set_label("Guild points: " + std::to_string(you->points_per_guild[ev.command.guild_id]));
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
        const auto you = tf_user_info[ev.command.usr.id];
        if (!you) {
            ev.reply(make_ephemeral_message("Something went wrong! You do not exist?! Please report error! I'm so sorry."));
            return;
        }

        if (trigg.item_name == "colorpicker") {
            const std::string _raw = wrk.get_modal_items()[0].second; // must be 1
            const int64_t sel = interpret_color(_raw);
            you->pref_color = sel;

            wrk.find_button_do("selfconf", "setcolor", [&](item<button_props>& i){
                i.extra.style = dpp::cos_success;
                i.label = "Updated: " + print_hex_color_auto(you->pref_color);
            });

            wrk.reply(!guil->commands_public);
            return;
        }
    }
    if (trigg.group_name == "poll") {
        if (trigg.item_name == "create") {
            const auto you = tf_user_info[ev.command.usr.id];
            if (!you) {
                ev.reply(make_ephemeral_message("Something went wrong! You do not exist?! Please report error! I'm so sorry."));
                return;
            }

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

            ev.reply(replying);

            ev.get_original_response([dmbcore = ev.from->creator, emojis_selected](const dpp::confirmation_callback_t& data) {
                if (data.is_error()) {
                    cout << console::color::DARK_RED << "Someone called /poll and had issues: " << data.get_error().message;
                    return;
                }
                dpp::message msg = std::get<dpp::message>(data.value);
                for(auto& i : emojis_selected) {
                    dmbcore->message_add_reaction(msg, i);
                }
            });
            return;
        }
    }
    if (trigg.group_name == "paste") {
        if (trigg.item_name == "post") {
            const auto you = tf_user_info[ev.command.usr.id];
            if (!you) { ev.reply(make_ephemeral_message("Something went wrong! You do not exist?! Please report error! I'm so sorry.")); return; }
            const auto guil = tf_guild_info[ev.command.guild_id];
            if (!guil) { ev.reply(make_ephemeral_message("Something went wrong! Guild do not exist?! Please report error! I'm so sorry.")); return; }

            auto& clipboard = you->clipboard;

            if (clipboard.guild_id != ev.command.guild_id && !guil->allow_external_paste){ ev.reply(make_ephemeral_message("This server doesn't allow paste from external sources.")); return; }

            const std::string paste_content = get_customid_as_str(ev.components, "comment");
                        
            dpp::message replying;
            ev.reply(dpp::interaction_response_type::ir_deferred_channel_message_with_source, replying);

            ev.from->creator->message_get(clipboard.message_id, clipboard.channel_id, 
            [ev, you, guil, replying, paste_content](const dpp::confirmation_callback_t data) mutable {
                auto& clipboard = you->clipboard;

                if (data.is_error()) {
                    ev.edit_response(make_ephemeral_message("I couldn't get clipboard source! Is it not available anymore?"));
                    you->clipboard.clear();
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
                    emb.description += "`Commented:`\n```\n";
                    for (const auto& i : paste_content.substr(0, 280)) { if (i != '`') emb.description += i; }
                    emb.description += "```\n";
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
















//    if (ev.custom_id == "user-pref_color") {
//        try {
//            const auto you = tf_user_info[ev.command.usr.id];
//            if (!you) {
//                ev.reply(make_ephemeral_message("Something went wrong! You do not exist?! Please report error! I'm so sorry."));
//                return;
//            }
//            const std::string val = get_customid_as_str(ev.components, "color");
//            const int64_t sel = interpret_color(val);
//            
//            auto_handle_button_switch(ev, ev.custom_id, [&](dpp::component& it){
//                you->pref_color = sel;
//                it.set_label("Profile color: " + print_hex_color_auto(you->pref_color));
//            });
//        }
//        catch(...) {
//            ev.reply(make_ephemeral_message("Sorry, something went wrong! I'm so sorry."));
//        }
//        return;
//    }
//    else if (ev.custom_id == "poll-create") {
//        try {
//            const auto you = tf_user_info[ev.command.usr.id];
//            if (!you) {
//                ev.reply(make_ephemeral_message("Something went wrong! You do not exist?! Please report error! I'm so sorry."));
//                return;
//            }
//
//            const auto extract_emojis_of = [](const std::string& src) -> std::vector<std::string> {
//                std::vector<std::string> emojis;
//                std::stringstream ss(src);
//                std::string token;
//                while (std::getline(ss, token, ';')){                    
//                    while(token.length() && (token.front() == ' ' || token.front() == '<' || token.front() == 'a' || token.front() == ':'))
//                        token.erase(token.begin());
//                    
//                    while(token.length() && (token.back() == ' ' || token.back() == '>')) 
//                        token.pop_back();
//
//                    if (token.size()) {
//                        emojis.push_back(token);
//                    }
//                }
//                return emojis;
//            };
//
//            const std::string title = get_customid_as_str(ev.components, "title"); // a must have
//            const std::string desc = get_customid_as_str(ev.components, "desc");
//            const std::string emojis = get_customid_as_str(ev.components, "emojis");
//            const std::string imglink = get_customid_as_str(ev.components, "imglink");
//            const std::string color = get_customid_as_str(ev.components, "color");
//
//            //ev.reply(make_ephemeral_message("In the works!\nTITLE: " + title + "\nDESC: " + desc + "\nEMOJIS: " + emojis + "\nIMGLINK: " + imglink + "\nCOLOR: " + color));
//
//            auto emojis_selected = extract_emojis_of(emojis);
//            if (emojis_selected.size() == 0) { emojis_selected.push_back(u8"👍"); emojis_selected.push_back(u8"👎"); }
//            const int64_t transl_clr = interpret_color(color);
//
//            dpp::message replying;
//            replying.id = ev.command.id;
//            replying.channel_id = ev.command.channel_id;
//            replying.set_type(dpp::message_type::mt_application_command);
//            replying.set_flags(0);
//
//            dpp::embed poll_enq;
//            dpp::embed_author author;
//
//            author.name = ev.command.usr.format_username();
//            author.icon_url = ev.command.usr.get_avatar_url(256);
//
//            poll_enq.set_author(author);
//            poll_enq.set_title(title);
//            if (desc.size()) poll_enq.set_description(desc);
//            poll_enq.set_color((transl_clr < 0 ? (you->pref_color < 0 ? random() : you->pref_color) : transl_clr));
//            poll_enq.set_thumbnail(images::poll_image_url);
//            if (imglink.size()) poll_enq.set_image(imglink);
//
//            replying.add_embed(poll_enq);
//
//            ev.reply(replying);
//
//            ev.get_original_response([dmbcore = ev.from->creator, emojis_selected](const dpp::confirmation_callback_t& data) {
//                if (data.is_error()) {
//                    cout << console::color::DARK_RED << "Someone called /poll and had issues: " << data.get_error().message;
//                    return;
//                }
//
//                dpp::message msg = std::get<dpp::message>(data.value);
//
//                for(auto& i : emojis_selected) 
//                {
//                    //cout << console::color::DARK_BLUE << " __ add react: to=" << (uint64_t)((void*)msg.owner) << ";react=" << i;
//                    dmbcore->message_add_reaction(msg, i); // dmbcore is a temp fix, msg.owner is nullptr rn.
//                }
//            });
//        }
//        catch(...) {
//            ev.reply(make_ephemeral_message("Sorry, something went wrong! I'm so sorry."));
//        }
//        return;
//    }
//    else if (ev.custom_id == "paste-comment") {
//        try {
//            const auto you = tf_user_info[ev.command.usr.id];
//            if (!you) {
//                ev.reply(make_ephemeral_message("Something went wrong! You do not exist?! Please report error! I'm so sorry."));
//                return;
//            }
//            const auto guil = tf_guild_info[ev.command.guild_id];
//            if (!guil) {
//                ev.reply(make_ephemeral_message("Something went wrong! Guild do not exist?! Please report error! I'm so sorry."));
//                return;
//            }
//
//            auto& clipboard = you->clipboard;
//
//            if (clipboard.guild_id != ev.command.guild_id && !guil->allow_external_paste)
//            {
//                ev.reply(make_ephemeral_message("This server doesn't allow paste from external sources."));
//                return;   
//            }
//
//            const std::string paste_content = get_customid_as_str(ev.components, "comment");
//                        
//            dpp::message replying;
//            replying.id = ev.command.id;
//            replying.channel_id = ev.command.channel_id;
//            replying.set_type(dpp::message_type::mt_application_command);
//            ev.reply(dpp::interaction_response_type::ir_deferred_channel_message_with_source, replying);
//
//            ev.from->creator->message_get(clipboard.message_id, clipboard.channel_id, [ev, you, guil, replying, paste_content](const dpp::confirmation_callback_t data) mutable {
//                auto& clipboard = you->clipboard;
//
//                if (data.is_error()) {
//                    ev.reply(make_ephemeral_message("I couldn't get clipboard source! Is it not available anymore?"));
//                    you->clipboard.clear();
//                    return;
//                }
//
//                replying.set_flags(0);
//
//                dpp::message sourcemsg = std::get<dpp::message>(data.value);
//
//                dpp::embed emb;
//                dpp::embed_author authr;
//                authr.icon_url = images::url_author_icon_clipboard;
//                authr.name = "Access source directly";
//                authr.url = clipboard.generate_url();
//
//                dpp::embed_footer emb_footer;
//                emb_footer.set_text("Referenced by " + ev.command.usr.format_username() +
//                    ", Source: " + sourcemsg.author.format_username() +
//                    ", Guild #" + std::to_string(clipboard.guild_id));
//
//                emb_footer.set_icon(ev.command.usr.get_avatar_url(256));
//
//                emb.set_author(authr);
//                emb.set_color((you->pref_color < 0 ? random() : you->pref_color));
//                emb.set_footer(emb_footer);
//
//                if (!paste_content.empty()){
//                    emb.description += "`Commented:`\n```\n";
//                    for (const auto& i : paste_content.substr(0, 280)) { if (i != '`') emb.description += i; }
//                    emb.description += "```\n";
//                }
//
//                if (!sourcemsg.content.empty()) {
//                    emb.description += "`Original text:`\n```\n";
//                    for (const auto& i : sourcemsg.content) { if (i != '`') emb.description += i; }
//                    emb.description += "```\n";
//                }
//
//                if (sourcemsg.attachments.size()) {
//                    emb.set_image(sourcemsg.attachments[0].url);
//                }
//
//                if (sourcemsg.embeds.size()) emb.description += "`Original message embeds below:`";
//
//                replying.add_embed(emb);
//
//                for(const auto& i : sourcemsg.embeds) replying.add_embed(i);
//
//                ev.edit_response(replying);
//            });
//
//        }
//        catch(...) {
//            ev.reply(make_ephemeral_message("Sorry, something went wrong! I'm so sorry."));
//        }
//        return;
//    }
//    else if (ev.custom_id == "guildconf-member_points-select_userid")
//    {
//        try {
//            const std::string val = get_customid_as_str(ev.components, "number");
//            unsigned long long literal = 0;
//            const bool guuuuud = (val.find("-") == std::string::npos) && (sscanf(val.c_str(), "%llu", &literal) == 1);
//
//            dpp::message cpy = ev.command.msg;
//            dpp::snowflake target_user = 0;
//
//            change_component(cpy.components, "guildconf-member_points-select_userid",  [&](dpp::component& it){
//                if (guuuuud) {
//                    std::string unam = "<name not in cache>";
//                    {
//                        dpp::cache<dpp::user>* cach = dpp::get_user_cache();
//                        std::shared_lock<std::shared_mutex> lu(cach->get_mutex());
//                        auto fusr = std::find_if(cach->get_container().begin(), cach->get_container().end(), [&](const std::pair<dpp::snowflake, dpp::user*>& u){ return u.first == literal; });
//                        if (fusr != cach->get_container().end()) unam = fusr->second->format_username();
//                    }
//
//                    it.set_label(std::to_string(literal) + " - " + unam);
//                    target_user = literal;
//                    it.set_style(dpp::cos_success);
//                }
//                else {
//                    it.set_label("Please enter a NUMBER (UNIQUE ID)");
//                    it.set_style(dpp::cos_danger);
//                }
//            });
//            
//            change_component(cpy.components, "guildconf-member_points-select_userpts", [&](dpp::component& it){
//                if (guuuuud && target_user != 0) {
//                    
//                    const auto you = tf_user_info[target_user];
//                    if (!you) {
//                        ev.reply(make_ephemeral_message("Something went wrong! You do not exist?! Please report error! I'm so sorry."));
//                        return;
//                    }
//
//                    it.set_style(dpp::cos_secondary);
//                    it.set_label("Points: " + std::to_string(you->points_per_guild[ev.command.guild_id]) + " (click to set new value)");
//                    it.set_disabled(false);
//                }
//                else {
//                    it.set_style(dpp::cos_secondary);
//                    it.set_label("Select user first");
//                    it.set_disabled(true);
//                }
//            });
//
//            ev.reply(dpp::ir_update_message, cpy, error_autoprint);
//        }
//        catch(...) {
//            ev.reply(make_ephemeral_message("Sorry, something went wrong! I'm so sorry."));
//        }
//        return;
//    }
//    else if (ev.custom_id == "guildconf-member_points-select_userpts")
//    {
//        try {
//            const std::string val = get_customid_as_str(ev.components, "number");
//            unsigned long long literal = 0;
//            const bool guuuuud = (val.find("-") == std::string::npos) && (sscanf(val.c_str(), "%llu", &literal) == 1);
//
//            dpp::message cpy = ev.command.msg;
//            dpp::snowflake target_user = 0;
//            
//            change_component(cpy.components, "guildconf-member_points-select_userid",  [&](dpp::component& it){
//                unsigned long long _tmp = 0;
//                if (sscanf(it.label.c_str(), "%llu", &_tmp) == 1) target_user = _tmp;
//            });
//
//            change_component(cpy.components, "guildconf-member_points-select_userpts", [&](dpp::component& it){
//                if (guuuuud && target_user != 0) {
//                    
//                    const auto you = tf_user_info[target_user];
//                    if (!you) {
//                        ev.reply(make_ephemeral_message("Something went wrong! You do not exist?! Please report error! I'm so sorry."));
//                        return;
//                    }
//
//                    you->points_per_guild[ev.command.guild_id] = literal;
//                    it.set_style(dpp::cos_success);
//                    it.set_label("Points: " + std::to_string(you->points_per_guild[ev.command.guild_id]) + " (click to set new value)");
//                    it.set_disabled(false);
//                }
//                else {
//                    it.set_style(dpp::cos_danger);
//                    it.set_label("Something went wrong");
//                    it.set_disabled(true);
//                }
//            });
//
//            ev.reply(dpp::ir_update_message, cpy, error_autoprint);
//        }
//        catch(...) {
//            ev.reply(make_ephemeral_message("Sorry, something went wrong! I'm so sorry."));
//        }
//        return;
//    }
//    else if (ev.custom_id == "guildconf-auto_roles-add")
//    {
//        try {
//            const std::string val = get_customid_as_str(ev.components, "paragraph");
//
//            const auto guil = tf_guild_info[ev.command.guild_id];
//            if (!guil) {
//                ev.reply(make_ephemeral_message("Something went wrong! Guild do not exist?! Please report error! I'm so sorry."));
//                return;
//            }
//
//            const std::vector<dpp::snowflake> listu = slice_string_auto_snowflake(val);
//            dpp::message cpy = ev.command.msg;
//
//            for(const auto& each : listu) {
//                if (guil->roles_when_join.size() >= guild_props::max_onjoin_roles_len) break;
//                
//                if (std::find(guil->roles_when_join.begin(), guil->roles_when_join.end(), each) == guil->roles_when_join.end())
//                    guil->roles_when_join.push_back(each);
//            }
//
//            cpy.content = "**Current on join roles list [" + std::to_string(guil->roles_when_join.size()) + "/" + std::to_string(guild_props::max_onjoin_roles_len) + "]:**\n```cs\n";
//            change_component(cpy.components, "guildconf-auto_roles-del", [&](dpp::component& d){
//                d.set_disabled(guil->roles_when_join.size() == 0);
//            });
//            change_component(cpy.components, "guildconf-auto_roles-add", [&](dpp::component& d){
//                d.set_disabled(guil->roles_when_join.size() >= guild_props::max_onjoin_roles_len);
//            });
//
//            if (guil->roles_when_join.size()){
//                dpp::cache<dpp::role>* cach = dpp::get_role_cache();
//                {
//                    std::shared_lock<std::shared_mutex> lu(cach->get_mutex());
//                    auto& rols = cach->get_container();
//
//                    for(const auto& it : guil->roles_when_join) {
//                        cpy.content += std::to_string(it) + ": #";
//                        auto found = std::find_if(rols.begin(), rols.end(), [&](const std::pair<dpp::snowflake, dpp::role*>& s){ return s.first == it;});
//                        if (found != rols.end()) cpy.content += found->second->name;
//                        cpy.content += "\n";
//                    }
//                }
//            }
//            else {
//                cpy.content += "<empty>";
//            }
//
//            cpy.content += "\n```";
//
//            ev.reply(dpp::ir_update_message, cpy, error_autoprint);
//        }
//        catch(...) {
//            ev.reply(make_ephemeral_message("Sorry, something went wrong! I'm so sorry."));
//        }
//        return;
//    }
//    else if (ev.custom_id == "guildconf-auto_roles-del")
//    {
//        try {
//            const std::string val = get_customid_as_str(ev.components, "paragraph");
//
//            const auto guil = tf_guild_info[ev.command.guild_id];
//            if (!guil) {
//                ev.reply(make_ephemeral_message("Something went wrong! Guild do not exist?! Please report error! I'm so sorry."));
//                return;
//            }
//
//            const std::vector<dpp::snowflake> listu = slice_string_auto_snowflake(val);
//            dpp::message cpy = ev.command.msg;
//
//            if (val != "*") {
//                for(const auto& each : listu) {
//                    auto it = std::find(guil->roles_when_join.begin(), guil->roles_when_join.end(), each);
//                    if (it != guil->roles_when_join.end()) guil->roles_when_join.erase(it);
//                }
//            }
//            else {
//                guil->roles_when_join.clear();
//            }
//
//            cpy.content = "**Current on join roles list [" + std::to_string(guil->roles_when_join.size()) + "/" + std::to_string(guild_props::max_onjoin_roles_len) + "]:**\n```cs\n";
//            change_component(cpy.components, "guildconf-auto_roles-del", [&](dpp::component& d){
//                d.set_disabled(guil->roles_when_join.size() == 0);
//            });
//            change_component(cpy.components, "guildconf-auto_roles-add", [&](dpp::component& d){
//                d.set_disabled(guil->roles_when_join.size() >= guild_props::max_onjoin_roles_len);
//            });
//
//            if (guil->roles_when_join.size()){
//                dpp::cache<dpp::role>* cach = dpp::get_role_cache();
//                {
//                    std::shared_lock<std::shared_mutex> lu(cach->get_mutex());
//                    auto& rols = cach->get_container();
//
//                    for(const auto& it : guil->roles_when_join) {
//                        cpy.content += std::to_string(it) + ": #";
//                        auto found = std::find_if(rols.begin(), rols.end(), [&](const std::pair<dpp::snowflake, dpp::role*>& s){ return s.first == it;});
//                        if (found != rols.end()) cpy.content += found->second->name;
//                        cpy.content += "\n";
//                    }
//                }
//            }
//            else {
//                cpy.content += "<empty>";
//            }
//
//            cpy.content += "\n```";
//
//            ev.reply(dpp::ir_update_message, cpy, error_autoprint);
//        }
//        catch(...) {
//            ev.reply(make_ephemeral_message("Sorry, something went wrong! I'm so sorry."));
//        }
//        return;
//    }
//    else if (ev.custom_id == "guildconf-roles_command-addgroup")
//    {
//        try {
//            const std::string val = get_customid_as_str(ev.components, "name");
//            if (val.empty()) {
//                ev.reply(make_ephemeral_message("Hmm your string was empty? Error."));
//                return;
//            }
//
//            const auto guil = tf_guild_info[ev.command.guild_id];
//            if (!guil) {
//                ev.reply(make_ephemeral_message("Something went wrong! Guild do not exist?! Please report error! I'm so sorry."));
//                return;
//            }
//            
//            ev.reply(dpp::ir_update_message, roleguild_auto_do(guil, ev.command.msg, roleguild_tasks::GROUP_ADD, val), error_autoprint);
//        }
//        catch(...) {
//            ev.reply(make_ephemeral_message("Sorry, something went wrong! I'm so sorry."));
//        }
//        return;
//    }
//    else if (ev.custom_id == "guildconf-roles_command-delgroup")
//    {
//        try {
//            const std::string val = get_customid_as_str(ev.components, "name");
//            if (val.empty()) {
//                ev.reply(make_ephemeral_message("Hmm your string was empty? Error."));
//                return;
//            }
//
//            const auto guil = tf_guild_info[ev.command.guild_id];
//            if (!guil) {
//                ev.reply(make_ephemeral_message("Something went wrong! Guild do not exist?! Please report error! I'm so sorry."));
//                return;
//            }
//
//            ev.reply(dpp::ir_update_message, roleguild_auto_do(guil, ev.command.msg, roleguild_tasks::GROUP_REMOVE, val), error_autoprint);
//        }
//        catch(...) {
//            ev.reply(make_ephemeral_message("Sorry, something went wrong! I'm so sorry."));
//        }
//        return;
//    }
//    else if (ev.custom_id == "guildconf-roles_command-add")
//    {
//        try { // get_customid_as_str
//            const std::string val = get_customid_as_str(ev.components, "roleid"); // Role ID
//            const std::string val2 = get_customid_as_str(ev.components, "name"); // Role naming
//
//            const auto guil = tf_guild_info[ev.command.guild_id];
//            if (!guil) {
//                ev.reply(make_ephemeral_message("Something went wrong! Guild do not exist?! Please report error! I'm so sorry."));
//                return;
//            }
//
//            if (guil->roles_available.size() == 0) {
//                ev.reply(make_ephemeral_message("You have no groups yet! Please create one first!"));
//                return;
//            }
//
//            const unsigned long long transl_val = dpp::from_string<dpp::snowflake>(val);
//            if (transl_val == 0 || val2.empty()) {
//                ev.reply(make_ephemeral_message("Invalid input, my friend!"));
//                return;
//            }
//
//            ev.reply(dpp::ir_update_message, roleguild_auto_do(guil, ev.command.msg, roleguild_tasks::ROLE_ADD, guild_info::pair_id_name{transl_val, val2 }), error_autoprint);
//        }
//        catch(...) {
//            ev.reply(make_ephemeral_message("Sorry, something went wrong! I'm so sorry."));
//        }
//        return;
//    }
//    else if (ev.custom_id == "guildconf-roles_command-del")
//    {
//        try {
//            const std::string val = get_customid_as_str(ev.components, "roleid"); // Role ID
//
//            const auto guil = tf_guild_info[ev.command.guild_id];
//            if (!guil) {
//                ev.reply(make_ephemeral_message("Something went wrong! Guild do not exist?! Please report error! I'm so sorry."));
//                return;
//            }
//            
//            ev.reply(dpp::ir_update_message, roleguild_auto_do(guil, ev.command.msg, roleguild_tasks::ROLE_REMOVE, val), error_autoprint);
//        }
//        catch(...) {
//            ev.reply(make_ephemeral_message("Sorry, something went wrong! I'm so sorry."));
//        }
//        return;
//    }
//    
//    ev.reply(make_ephemeral_message("Sorry, something went wrong! I can't find what to do. Please report!"));
}