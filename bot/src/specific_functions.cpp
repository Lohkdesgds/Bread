#include <specific_functions.hpp>

void g_on_react(const dpp::message_reaction_add_t& ev)
{
    if (ev.reacting_user.is_bot()) return;

    ev.from->creator->message_get(ev.message_id, ev.reacting_channel->id, 
    [emoj = ev.reacting_emoji, targ = ev.reacting_user, boot = ev.from->creator](const dpp::confirmation_callback_t& dat) mutable {
        if (dat.is_error()) {
            cout << console::color::DARK_PURPLE << "Failed on reaction once. Can't get message.";
            return;
        }

        dpp::message msg = std::get<dpp::message>(dat.value);

        if (msg.interaction.name == "poll" && msg.author.id == boot->me.id) {
            if (msg.interaction.usr.id == targ.id) {
                if (emoj.id == 0 && emoj.name == poll::poll_emoji_delete_reactions)
                {
                    boot->message_delete_all_reactions(msg);
                }
                else if (emoj.id == 0 && emoj.name == poll::poll_emoji_delete_message)
                {
                    boot->message_delete(msg.id, msg.channel_id);
                }
                else { // add
                    boot->message_add_reaction(msg, emoj.format());
                }
            }
            else {
                if (std::find_if(msg.reactions.begin(), msg.reactions.end(), [&](const dpp::reaction& e){ return e.emoji_id == emoj.id && e.emoji_name == emoj.name; }) != msg.reactions.end()){ // add
                    boot->message_delete_reaction(msg, targ.id, emoj.format());
                }
            }
        }
    });
}

void g_on_log(const dpp::log_t& log)
{
    switch(log.severity){
    case dpp::loglevel::ll_critical:
        cout << console::color::RED << "[DPP][CRITICAL] " << log.message;
        break;
    case dpp::loglevel::ll_error:
        cout << console::color::RED << "[DPP][ERROR] " << log.message;
        break;
    case dpp::loglevel::ll_warning:
        cout << console::color::GOLD << "[DPP][WARN] " << log.message;
        break;
    case dpp::loglevel::ll_info:
        cout << console::color::BLUE << "[DPP][INFO] " << log.message;
        break;
    case dpp::loglevel::ll_debug:
        cout << console::color::DARK_PURPLE << "[DPP][DEBUG] " << log.message;
        break;
    case dpp::loglevel::ll_trace:
        // trace is not needed tbh
        break;
    }
}

void g_on_message(const dpp::message_create_t& ev)
{
    using mull = unsigned long long;

    if (ev.msg.author.is_bot()) return;

    auto you = tf_user_info[ev.msg.author.id];
    if (!you) return; // discard

    you->messages_sent++;
    you->messages_sent_per_guild[ev.msg.guild_id]++;

    you->attachments_sent += ev.msg.attachments.size();
    you->attachments_sent_per_guild[ev.msg.guild_id] += ev.msg.attachments.size();

    auto guil = tf_guild_info[ev.msg.guild_id];
    if (!guil) return; // abort :(

    if (get_time_ms() <= guil->last_user_earn_points) return;
    if (get_time_ms() <= you->last_points_earned) return;
    
    std::unique_lock<std::shared_mutex> l1(guil->mu, std::defer_lock);
    std::unique_lock<std::shared_mutex> l2(you->mu, std::defer_lock);
    std::lock(l1, l2);

    mull old_pts_g = you->points;
    mull old_pts_l = you->points_per_guild[ev.msg.guild_id];

    int boost_final = 0;
    const bool had_boost = (random() % leveling::range_boost_chances == 0);

    if (had_boost) boost_final = static_cast<int>(rand() % leveling::range_boost_total) + leveling::range_boost_low;
    else boost_final = static_cast<int>(rand() % leveling::range_total) + leveling::range_low;

    you->last_points_earned = get_time_ms() + leveling::time_to_earn_points_sameuser_ms;
    guil->last_user_earn_points = get_time_ms() + leveling::time_to_earn_points_sameuser_ms;

    if (boost_final == 0) return; // no changes

    if (boost_final < 0) {
        if (you->points_per_guild[ev.msg.guild_id] < (-boost_final)) you->points_per_guild[ev.msg.guild_id] = 0;
        else you->points_per_guild[ev.msg.guild_id] += boost_final;
        if (you->points < (-boost_final)) you->points = 0;
        else you->points += boost_final;
    }
    if (boost_final > 0) you->times_they_got_positive_points++;
    if (boost_final < 0) you->times_they_got_negative_points++;

    //Lunaris::cout << ev.msg->author->username << " -> " << std::to_string(boost_final) ;
        
    mull old_lvl_g = 1, old_lvl_l = 1;
    mull new_lvl_g = 1, new_lvl_l = 1;

    mull new_pts_g = you->points;
    mull new_pts_l = you->points_per_guild[ev.msg.guild_id];

    while(static_cast<mull>(pow(leveling::calc_level_div, old_lvl_g + leveling::threshold_points_level_0 - 1)) <= old_pts_g) old_lvl_g++;
    while(static_cast<mull>(pow(leveling::calc_level_div, old_lvl_l + leveling::threshold_points_level_0 - 1)) <= old_pts_l) old_lvl_l++;
    while(static_cast<mull>(pow(leveling::calc_level_div, new_lvl_g + leveling::threshold_points_level_0 - 1)) <= new_pts_g) new_lvl_g++;
    while(static_cast<mull>(pow(leveling::calc_level_div, new_lvl_l + leveling::threshold_points_level_0 - 1)) <= new_pts_l) new_lvl_l++;

    const bool global_level_up = (new_lvl_g > old_lvl_g && new_lvl_g >= 1);
    const bool local_level_up =  (new_lvl_l > old_lvl_l && new_lvl_l >= 1);
    const bool global_level_down = (new_lvl_g < old_lvl_g && old_lvl_g >= 1);
    const bool local_level_down =  (new_lvl_l < old_lvl_l && old_lvl_l >= 1);

    if (!global_level_up && !local_level_up && !global_level_down && !local_level_down) return;
    const bool was_level_up = (global_level_up || local_level_up);

    if (local_level_up || local_level_down) {
        const auto& vecref = guil->role_per_level;

        if (vecref.size() > 0) {
            dpp::guild_member member;
            member.guild_id = ev.msg.guild_id;
            member.user_id 	= ev.msg.author.id;
            member.roles 	= ev.msg.member.roles;

            bool had_update = false;

            for (const auto& i : vecref) {
                if (i.level <= new_lvl_l && (std::find(member.roles.begin(), member.roles.end(), i.id) == member.roles.end())) {
                    member.roles.push_back(i.id);
                    had_update = true;
                    //Lunaris::cout << "+" << i.id ;
                }
                else if (i.level > new_lvl_l){
                    auto it = std::find(member.roles.begin(), member.roles.end(), i.id);
                    if (it != member.roles.end()) {
                        member.roles.erase(it);
                        //Lunaris::cout << "-" << i.id ;
                        had_update = true;
                    }
                }
            }

            if (had_update) {
                ev.from->creator->guild_edit_member(member);
            }
        }
    }

    if (you->show_level_up_messages && !guil->block_levelup_user_event) {

        dpp::message replying;
        replying.set_type(dpp::message_type::mt_reply);
        if (const auto _temp = guil->fallback_levelup_message_channel; _temp != 0) {
            replying.channel_id = _temp;
        }
        else {
            replying.channel_id = ev.msg.channel_id;
            replying.message_reference.channel_id = ev.msg.channel_id;
            replying.message_reference.message_id = ev.msg.id;
        }
        replying.set_flags(64);

        std::string desc = (was_level_up ? ("📈 **LEVEL UP**\n") : ("📉 **LEVEL DOWN**\n"));
        if (global_level_up || global_level_down) desc += (desc.length() ? "\n" : "") + (u8"✨ **GLOBAL:** " + std::string(was_level_up ? u8"🔺" : u8"🔻") + u8" `LEVEL " + std::to_string(new_lvl_g) + "`");
        if (local_level_up  || local_level_down)  desc += (desc.length() ? "\n" : "") + (u8"✨ **LOCAL:** "  + std::string(was_level_up ? u8"🔺" : u8"🔻") + u8" `LEVEL " + std::to_string(new_lvl_l) + "`");

        //Lunaris::cout << "Level up stuff:\nTITLE=" << title << "\nDESC=" << desc ;

        dpp::embed autoembed = dpp::embed()
            .set_author(
                dpp::embed_author{
                    .name = ev.msg.author.format_username(),
                    .url = ev.msg.author.get_avatar_url(256),
                    .icon_url = ev.msg.author.get_avatar_url(256)
                })
            .set_description(desc)
            .set_color((you->pref_color < 0 ? random() : you->pref_color) % 0xFFFFFF)
            .set_thumbnail(images::points_image_url);

        replying.embeds.push_back(autoembed);
        replying.set_content("");

        ev.from->creator->message_create(replying);
    }
}

void g_on_ready(const dpp::ready_t& ev, safe_data<slash_global>& sg)
{
    sg.safe<void>([&ev](slash_global& s){ s.update_bot_id(*ev.from->creator); });
}

void g_on_modal(const dpp::form_submit_t& ev)
{    
    if (ev.custom_id == "user-pref_color") {
        try {
            const auto you = tf_user_info[ev.command.usr.id];
            if (!you) {
                ev.reply(make_ephemeral_message("Something went wrong! You do not exist?! Please report error! I'm so sorry."));
                return;
            }
            const std::string val = get_customid_as_str(ev.components, "color");
            const int64_t sel = interpret_color(val);
            
            auto_handle_button_switch(ev, ev.custom_id, [&](dpp::component& it){
                you->pref_color = sel;
                it.set_label("Profile color: " + (you->pref_color < 0 ? "DEFAULT" : print_hex(you->pref_color)));
            });
        }
        catch(...) {
            ev.reply(make_ephemeral_message("Sorry, something went wrong! I'm so sorry."));
        }
        return;
    }
    else if (ev.custom_id == "poll-create") {
        try {
            const auto you = tf_user_info[ev.command.usr.id];
            if (!you) {
                ev.reply(make_ephemeral_message("Something went wrong! You do not exist?! Please report error! I'm so sorry."));
                return;
            }

            const auto extract_emojis_of = [](const std::string& src) -> std::vector<std::string> {
                std::vector<std::string> emojis;
                std::stringstream ss(src);
                std::string token;
                while (std::getline(ss, token, ';')){                    
                    while(token.length() && (token.front() == ' ' || token.front() == '<' || token.front() == 'a' || token.front() == ':'))
                        token.erase(token.begin());
                    
                    while(token.length() && (token.back() == ' ' || token.back() == '>')) 
                        token.pop_back();

                    if (token.size()) {
                        emojis.push_back(token);
                    }
                }
                return emojis;
            };

            const std::string title = get_customid_as_str(ev.components, "title"); // a must have
            const std::string desc = get_customid_as_str(ev.components, "desc");
            const std::string emojis = get_customid_as_str(ev.components, "emojis");
            const std::string imglink = get_customid_as_str(ev.components, "imglink");
            const std::string color = get_customid_as_str(ev.components, "color");

            //ev.reply(make_ephemeral_message("In the works!\nTITLE: " + title + "\nDESC: " + desc + "\nEMOJIS: " + emojis + "\nIMGLINK: " + imglink + "\nCOLOR: " + color));

            auto emojis_selected = extract_emojis_of(emojis);
            if (emojis_selected.size() == 0) { emojis_selected.push_back(u8"👍"); emojis_selected.push_back(u8"👎"); }
            const int64_t transl_clr = interpret_color(color);

            dpp::message replying;
            replying.id = ev.command.id;
            replying.channel_id = ev.command.channel_id;
            replying.set_type(dpp::message_type::mt_application_command);
            replying.set_flags(0);

            dpp::embed poll_enq;
            dpp::embed_author author;

            author.name = ev.command.usr.format_username();
            author.icon_url = ev.command.usr.get_avatar_url(256);

            poll_enq.set_author(author);
            poll_enq.set_title(title);
            if (desc.size()) poll_enq.set_description(desc);
            poll_enq.set_color((transl_clr < 0 ? (you->pref_color < 0 ? random() : you->pref_color) : transl_clr) % 0xFFFFFF);
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

                for(auto& i : emojis_selected) 
                {
                    //cout << console::color::DARK_BLUE << " __ add react: to=" << (uint64_t)((void*)msg.owner) << ";react=" << i;
                    dmbcore->message_add_reaction(msg, i); // dmbcore is a temp fix, msg.owner is nullptr rn.
                }
            });
        }
        catch(...) {
            ev.reply(make_ephemeral_message("Sorry, something went wrong! I'm so sorry."));
        }
        return;
    }
    else if (ev.custom_id == "paste-comment") {
        try {
            const auto you = tf_user_info[ev.command.usr.id];
            if (!you) {
                ev.reply(make_ephemeral_message("Something went wrong! You do not exist?! Please report error! I'm so sorry."));
                return;
            }
            const auto guil = tf_guild_info[ev.command.guild_id];
            if (!guil) {
                ev.reply(make_ephemeral_message("Something went wrong! Guild do not exist?! Please report error! I'm so sorry."));
                return;
            }

            auto& clipboard = you->clipboard;

            if (clipboard.guild_id != ev.command.guild_id && !guil->allow_external_paste)
            {
                ev.reply(make_ephemeral_message("This server doesn't allow paste from external sources."));
                return;   
            }

            const std::string paste_content = get_customid_as_str(ev.components, "comment");
                        
            dpp::message replying;
            replying.id = ev.command.id;
            replying.channel_id = ev.command.channel_id;
            replying.set_type(dpp::message_type::mt_application_command);
            ev.reply(dpp::interaction_response_type::ir_deferred_channel_message_with_source, replying);

            ev.from->creator->message_get(clipboard.message_id, clipboard.channel_id, [ev, you, guil, replying, paste_content](const dpp::confirmation_callback_t data) mutable {
                auto& clipboard = you->clipboard;

                if (data.is_error()) {
                    ev.reply(make_ephemeral_message("I couldn't get clipboard source! Is it not available anymore?"));
                    you->clipboard.clear();
                    return;
                }

                replying.set_flags(0);

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
                emb.set_color((you->pref_color < 0 ? random() : you->pref_color) % 0xFFFFFF);
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

                if (sourcemsg.attachments.size()) {
                    emb.set_image(sourcemsg.attachments[0].url);
                }

                if (sourcemsg.embeds.size()) emb.description += "`Original message embeds below:`";

                replying.add_embed(emb);

                for(const auto& i : sourcemsg.embeds) replying.add_embed(i);

                ev.edit_response(replying);
            });

        }
        catch(...) {
            ev.reply(make_ephemeral_message("Sorry, something went wrong! I'm so sorry."));
        }
        return;
    }
    else if (ev.custom_id == "guildconf-member_points-select_userid")
    {
        try {
            const std::string val = get_customid_as_str(ev.components, "number");
            unsigned long long literal = 0;
            const bool guuuuud = (val.find("-") == std::string::npos) && (sscanf(val.c_str(), "%llu", &literal) == 1);

            dpp::message cpy = ev.command.msg;
            dpp::snowflake target_user = 0;

            change_component(cpy.components, "guildconf-member_points-select_userid",  [&](dpp::component& it){
                if (guuuuud) {
                    std::string unam = "<name not in cache>";
                    {
                        dpp::cache<dpp::user>* cach = dpp::get_user_cache();
                        std::shared_lock<std::shared_mutex> lu(cach->get_mutex());
                        auto fusr = std::find_if(cach->get_container().begin(), cach->get_container().end(), [&](const std::pair<dpp::snowflake, dpp::user*>& u){ return u.first == literal; });
                        if (fusr != cach->get_container().end()) unam = fusr->second->format_username();
                    }

                    it.set_label(std::to_string(literal) + " - " + unam);
                    target_user = literal;
                    it.set_style(dpp::cos_success);
                }
                else {
                    it.set_label("Please enter a NUMBER (UNIQUE ID)");
                    it.set_style(dpp::cos_danger);
                }
            });
            
            change_component(cpy.components, "guildconf-member_points-select_userpts", [&](dpp::component& it){
                if (guuuuud && target_user != 0) {
                    
                    const auto you = tf_user_info[target_user];
                    if (!you) {
                        ev.reply(make_ephemeral_message("Something went wrong! You do not exist?! Please report error! I'm so sorry."));
                        return;
                    }

                    it.set_style(dpp::cos_secondary);
                    it.set_label("Points: " + std::to_string(you->points_per_guild[ev.command.guild_id]) + " (click to set new value)");
                    it.set_disabled(false);
                }
                else {
                    it.set_style(dpp::cos_secondary);
                    it.set_label("Select user first");
                    it.set_disabled(true);
                }
            });

            ev.reply(dpp::ir_update_message, cpy, error_autoprint);
        }
        catch(...) {
            ev.reply(make_ephemeral_message("Sorry, something went wrong! I'm so sorry."));
        }
        return;
    }
    else if (ev.custom_id == "guildconf-member_points-select_userpts")
    {
        try {
            const std::string val = get_customid_as_str(ev.components, "number");
            unsigned long long literal = 0;
            const bool guuuuud = (val.find("-") == std::string::npos) && (sscanf(val.c_str(), "%llu", &literal) == 1);

            dpp::message cpy = ev.command.msg;
            dpp::snowflake target_user = 0;
            
            change_component(cpy.components, "guildconf-member_points-select_userid",  [&](dpp::component& it){
                unsigned long long _tmp = 0;
                if (sscanf(it.label.c_str(), "%llu", &_tmp) == 1) target_user = _tmp;
            });

            change_component(cpy.components, "guildconf-member_points-select_userpts", [&](dpp::component& it){
                if (guuuuud && target_user != 0) {
                    
                    const auto you = tf_user_info[target_user];
                    if (!you) {
                        ev.reply(make_ephemeral_message("Something went wrong! You do not exist?! Please report error! I'm so sorry."));
                        return;
                    }

                    you->points_per_guild[ev.command.guild_id] = literal;
                    it.set_style(dpp::cos_success);
                    it.set_label("Points: " + std::to_string(you->points_per_guild[ev.command.guild_id]) + " (click to set new value)");
                    it.set_disabled(false);
                }
                else {
                    it.set_style(dpp::cos_danger);
                    it.set_label("Something went wrong");
                    it.set_disabled(true);
                }
            });

            ev.reply(dpp::ir_update_message, cpy, error_autoprint);
        }
        catch(...) {
            ev.reply(make_ephemeral_message("Sorry, something went wrong! I'm so sorry."));
        }
        return;
    }
    else if (ev.custom_id == "guildconf-auto_roles-add")
    {
        try {
            const std::string val = get_customid_as_str(ev.components, "paragraph");

            const auto guil = tf_guild_info[ev.command.guild_id];
            if (!guil) {
                ev.reply(make_ephemeral_message("Something went wrong! Guild do not exist?! Please report error! I'm so sorry."));
                return;
            }

            const std::vector<dpp::snowflake> listu = slice_string_auto_snowflake(val);
            dpp::message cpy = ev.command.msg;

            for(const auto& each : listu) {
                if (guil->roles_when_join.size() >= guild_props::max_onjoin_roles_len) break;
                
                if (std::find(guil->roles_when_join.begin(), guil->roles_when_join.end(), each) == guil->roles_when_join.end())
                    guil->roles_when_join.push_back(each);
            }

            cpy.content = "**Current on join roles list [" + std::to_string(guil->roles_when_join.size()) + "/" + std::to_string(guild_props::max_onjoin_roles_len) + "]:**\n```cs\n";
            change_component(cpy.components, "guildconf-auto_roles-del", [&](dpp::component& d){
                d.set_disabled(guil->roles_when_join.size() == 0);
            });
            change_component(cpy.components, "guildconf-auto_roles-add", [&](dpp::component& d){
                d.set_disabled(guil->roles_when_join.size() >= guild_props::max_onjoin_roles_len);
            });

            if (guil->roles_when_join.size()){
                dpp::cache<dpp::role>* cach = dpp::get_role_cache();
                {
                    std::shared_lock<std::shared_mutex> lu(cach->get_mutex());
                    auto& rols = cach->get_container();

                    for(const auto& it : guil->roles_when_join) {
                        cpy.content += std::to_string(it) + ": #";
                        auto found = std::find_if(rols.begin(), rols.end(), [&](const std::pair<dpp::snowflake, dpp::role*>& s){ return s.first == it;});
                        if (found != rols.end()) cpy.content += found->second->name;
                        cpy.content += "\n";
                    }
                }
            }
            else {
                cpy.content += "<empty>";
            }

            cpy.content += "\n```";

            ev.reply(dpp::ir_update_message, cpy, error_autoprint);
        }
        catch(...) {
            ev.reply(make_ephemeral_message("Sorry, something went wrong! I'm so sorry."));
        }
        return;
    }
    else if (ev.custom_id == "guildconf-auto_roles-del")
    {
        try {
            const std::string val = get_customid_as_str(ev.components, "paragraph");

            const auto guil = tf_guild_info[ev.command.guild_id];
            if (!guil) {
                ev.reply(make_ephemeral_message("Something went wrong! Guild do not exist?! Please report error! I'm so sorry."));
                return;
            }

            const std::vector<dpp::snowflake> listu = slice_string_auto_snowflake(val);
            dpp::message cpy = ev.command.msg;

            if (val != "*") {
                for(const auto& each : listu) {
                    auto it = std::find(guil->roles_when_join.begin(), guil->roles_when_join.end(), each);
                    if (it != guil->roles_when_join.end()) guil->roles_when_join.erase(it);
                }
            }
            else {
                guil->roles_when_join.clear();
            }

            cpy.content = "**Current on join roles list [" + std::to_string(guil->roles_when_join.size()) + "/" + std::to_string(guild_props::max_onjoin_roles_len) + "]:**\n```cs\n";
            change_component(cpy.components, "guildconf-auto_roles-del", [&](dpp::component& d){
                d.set_disabled(guil->roles_when_join.size() == 0);
            });
            change_component(cpy.components, "guildconf-auto_roles-add", [&](dpp::component& d){
                d.set_disabled(guil->roles_when_join.size() >= guild_props::max_onjoin_roles_len);
            });

            if (guil->roles_when_join.size()){
                dpp::cache<dpp::role>* cach = dpp::get_role_cache();
                {
                    std::shared_lock<std::shared_mutex> lu(cach->get_mutex());
                    auto& rols = cach->get_container();

                    for(const auto& it : guil->roles_when_join) {
                        cpy.content += std::to_string(it) + ": #";
                        auto found = std::find_if(rols.begin(), rols.end(), [&](const std::pair<dpp::snowflake, dpp::role*>& s){ return s.first == it;});
                        if (found != rols.end()) cpy.content += found->second->name;
                        cpy.content += "\n";
                    }
                }
            }
            else {
                cpy.content += "<empty>";
            }

            cpy.content += "\n```";

            ev.reply(dpp::ir_update_message, cpy, error_autoprint);
        }
        catch(...) {
            ev.reply(make_ephemeral_message("Sorry, something went wrong! I'm so sorry."));
        }
        return;
    }
    else if (ev.custom_id == "guildconf-roles_command-addgroup")
    {
        try {
            const std::string val = get_customid_as_str(ev.components, "name");
            if (val.empty()) {
                ev.reply(make_ephemeral_message("Hmm your string was empty? Error."));
                return;
            }

            const auto guil = tf_guild_info[ev.command.guild_id];
            if (!guil) {
                ev.reply(make_ephemeral_message("Something went wrong! Guild do not exist?! Please report error! I'm so sorry."));
                return;
            }
            
            ev.reply(dpp::ir_update_message, roleguild_auto_do(guil, ev.command.msg, roleguild_tasks::GROUP_ADD, val), error_autoprint);
        }
        catch(...) {
            ev.reply(make_ephemeral_message("Sorry, something went wrong! I'm so sorry."));
        }
        return;
    }
    else if (ev.custom_id == "guildconf-roles_command-delgroup")
    {
        try {
            const std::string val = get_customid_as_str(ev.components, "name");
            if (val.empty()) {
                ev.reply(make_ephemeral_message("Hmm your string was empty? Error."));
                return;
            }

            const auto guil = tf_guild_info[ev.command.guild_id];
            if (!guil) {
                ev.reply(make_ephemeral_message("Something went wrong! Guild do not exist?! Please report error! I'm so sorry."));
                return;
            }

            ev.reply(dpp::ir_update_message, roleguild_auto_do(guil, ev.command.msg, roleguild_tasks::GROUP_REMOVE, val), error_autoprint);
        }
        catch(...) {
            ev.reply(make_ephemeral_message("Sorry, something went wrong! I'm so sorry."));
        }
        return;
    }
    else if (ev.custom_id == "guildconf-roles_command-add")
    {
        try { // get_customid_as_str
            const std::string val = get_customid_as_str(ev.components, "roleid"); // Role ID
            const std::string val2 = get_customid_as_str(ev.components, "name"); // Role naming

            const auto guil = tf_guild_info[ev.command.guild_id];
            if (!guil) {
                ev.reply(make_ephemeral_message("Something went wrong! Guild do not exist?! Please report error! I'm so sorry."));
                return;
            }

            if (guil->roles_available.size() == 0) {
                ev.reply(make_ephemeral_message("You have no groups yet! Please create one first!"));
                return;
            }

            const unsigned long long transl_val = dpp::from_string<dpp::snowflake>(val);
            if (transl_val == 0 || val2.empty()) {
                ev.reply(make_ephemeral_message("Invalid input, my friend!"));
                return;
            }

            ev.reply(dpp::ir_update_message, roleguild_auto_do(guil, ev.command.msg, roleguild_tasks::ROLE_ADD, guild_info::pair_id_name{transl_val, val2 }), error_autoprint);
        }
        catch(...) {
            ev.reply(make_ephemeral_message("Sorry, something went wrong! I'm so sorry."));
        }
        return;
    }
    else if (ev.custom_id == "guildconf-roles_command-del")
    {
        try {
            const std::string val = get_customid_as_str(ev.components, "roleid"); // Role ID

            const auto guil = tf_guild_info[ev.command.guild_id];
            if (!guil) {
                ev.reply(make_ephemeral_message("Something went wrong! Guild do not exist?! Please report error! I'm so sorry."));
                return;
            }
            
            ev.reply(dpp::ir_update_message, roleguild_auto_do(guil, ev.command.msg, roleguild_tasks::ROLE_REMOVE, val), error_autoprint);
        }
        catch(...) {
            ev.reply(make_ephemeral_message("Sorry, something went wrong! I'm so sorry."));
        }
        return;
    }
    
    ev.reply(make_ephemeral_message("Sorry, something went wrong! I can't find what to do. Please report!"));
}

void g_on_button_click(const dpp::button_click_t& ev)
{
    if (ev.custom_id == "user-show_level_up_messages")
    {
        const auto you = tf_user_info[ev.command.usr.id];
        if (!you) {
            ev.reply(make_ephemeral_message("Something went wrong! You do not exist?! Please report error! I'm so sorry."));
            return;
        }

        auto_handle_button_switch(ev, ev.custom_id, [&](dpp::component& it){
            you->show_level_up_messages = !you->show_level_up_messages;
            set_boolean_button(you->show_level_up_messages, it);
        });
    }
    else if (ev.custom_id == "user-pref_color")
    {
        dpp::interaction_modal_response modal("user-pref_color", "Select color");
        modal.add_component(
            dpp::component()
                .set_label("What color best describes you?")
                .set_id("color")
                .set_type(dpp::cot_text)
                .set_required(true)
                .set_placeholder("red, green, blue, ..., black, default, 0xHEX or DECIMAL")
                .set_min_length(1)
                .set_max_length(20)
                .set_text_style(dpp::text_short)
        );
        ev.dialog(modal, error_autoprint);
    }
    else if (ev.custom_id == "user-download_user_data")
    {
        const auto you = tf_user_info[ev.command.usr.id];
        if (!you) {
            ev.reply(make_ephemeral_message("Something went wrong! You do not exist?! Please report error! I'm so sorry."));
            return;
        }

        dpp::message msg;
        msg.set_content("Your user data:");
        msg.add_file("user_data.json", you->to_json().dump(2));
        msg.set_flags(64);
        ev.reply(msg, error_autoprint);
    }
    else if (ev.custom_id == "guildconf-paste-switch")
    {
        const auto guil = tf_guild_info[ev.command.guild_id];
        if (!guil) {
            ev.reply(make_ephemeral_message("Something went wrong! Guild do not exist?! Please report error! I'm so sorry."));
            return;
        }

        auto_handle_button_switch(ev, ev.custom_id, [&](dpp::component& it){
            guil->allow_external_paste = !guil->allow_external_paste;
            set_boolean_button(guil->allow_external_paste, it);
        });
    }
    else if (ev.custom_id == "guildconf-member_points-select_userid")
    {        
        dpp::interaction_modal_response modal(ev.custom_id, "Enter user ID");
        modal.add_component(
            dpp::component()
                .set_label("Enter a numeric user ID")
                .set_id("number")
                .set_type(dpp::cot_text)
                .set_required(true)
                .set_placeholder("0123456789...")
                .set_min_length(1)
                .set_max_length(20)
                .set_text_style(dpp::text_short)
        );
        ev.dialog(modal, error_autoprint);
    }
    else if (ev.custom_id == "guildconf-member_points-select_userpts")
    {
        dpp::interaction_modal_response modal(ev.custom_id, "Change user local points");
        modal.add_component(
            dpp::component()
                .set_label("Set this user points on this guild")
                .set_id("number")
                .set_type(dpp::cot_text)
                .set_required(true)
                .set_placeholder("A number")
                .set_min_length(1)
                .set_max_length(20)
                .set_text_style(dpp::text_short)
        );
        ev.dialog(modal, error_autoprint);
    }
    else if (ev.custom_id == "guildconf-auto_roles-add")
    {
        dpp::interaction_modal_response modal(ev.custom_id, "Add one or more roles to list");
        modal.add_component(
            dpp::component()
                .set_label("Paste IDs here")
                .set_id("paragraph")
                .set_type(dpp::cot_text)
                .set_required(true)
                .set_placeholder("123456...")
                .set_min_length(1)
                .set_max_length(512)
                .set_text_style(dpp::text_paragraph)
        );
        ev.dialog(modal, error_autoprint);
    }
    else if (ev.custom_id == "guildconf-auto_roles-del")
    {
        dpp::interaction_modal_response modal(ev.custom_id, "Remove one or more roles from list");
        modal.add_component(
            dpp::component()
                .set_label("Paste IDs or * to delete all.")
                .set_id("paragraph")
                .set_type(dpp::cot_text)
                .set_required(true)
                .set_placeholder("123456...")
                .set_min_length(1)
                .set_max_length(512)
                .set_text_style(dpp::text_paragraph)
        );
        ev.dialog(modal, error_autoprint);
    }
    else if (ev.custom_id == "guildconf-roles_command-addgroup")
    {
        dpp::interaction_modal_response modal(ev.custom_id, "Create new role group");
        modal.add_component(
            dpp::component()
                .set_label("Give it a name")
                .set_id("name")
                .set_type(dpp::cot_text)
                .set_required(true)
                .set_placeholder("Funky group")
                .set_min_length(1)
                .set_max_length(20)
                .set_text_style(dpp::text_short)
        );
        ev.dialog(modal, error_autoprint);
    }
    else if (ev.custom_id == "guildconf-roles_command-delgroup")
    {
        dpp::interaction_modal_response modal(ev.custom_id, "Remove one role group");
        modal.add_component(
            dpp::component()
                .set_label("Tell me its name")
                .set_id("name")
                .set_type(dpp::cot_text)
                .set_required(true)
                .set_placeholder("Funky group")
                .set_min_length(1)
                .set_max_length(20)
                .set_text_style(dpp::text_short)
        );
        ev.dialog(modal, error_autoprint);
    }
    else if (ev.custom_id == "guildconf-roles_command-add")
    {
        dpp::interaction_modal_response modal(ev.custom_id, "Add role to list");
        modal.add_component(
            dpp::component()
                .set_label("Role ID")
                .set_id("roleid")
                .set_type(dpp::cot_text)
                .set_required(true)
                .set_placeholder("123456...")
                .set_min_length(1)
                .set_max_length(20)
                .set_text_style(dpp::text_short)
        )
        .add_row()
        .add_component(
            dpp::component()
                .set_label("Name to show")
                .set_id("name")
                .set_type(dpp::cot_text)
                .set_required(true)
                .set_placeholder("My cool role")
                .set_min_length(1)
                .set_max_length(40)
                .set_text_style(dpp::text_short)
        );
        ev.dialog(modal, error_autoprint);
    }
    else if (ev.custom_id == "guildconf-roles_command-del")
    {
        dpp::interaction_modal_response modal(ev.custom_id, "Remove role from list");
        modal.add_component(
            dpp::component()
                .set_label("Paste ID or * to delete all.")
                .set_id("roleid")
                .set_type(dpp::cot_text)
                .set_required(true)
                .set_placeholder("123456...")
                .set_min_length(1)
                .set_max_length(20)
                .set_text_style(dpp::text_short)
        );
        ev.dialog(modal, error_autoprint);
    }
    else {
        ev.reply(dpp::ir_update_message, "This interaction is unknown. Please try again later!", error_autoprint);
    }
}

void g_on_select(const dpp::select_click_t& ev)
{
    if (ev.custom_id == "guild-generate_menu") {
        const auto& selected = ev.values[0];

        if (selected == "guildconf-export"){ // create a message with json there lol

            const auto guil = tf_guild_info[ev.command.guild_id];
            if (!guil) {
                ev.reply(make_ephemeral_message("Something went wrong! Guild do not exist?! Please report error! I'm so sorry."));
                return;
            }

            dpp::message msg;
            msg.set_content("Guild configuration");
            msg.add_file("guild_data.json", guil->to_json().dump(2));
            msg.set_flags(64);
            ev.reply(dpp::ir_update_message, msg, error_autoprint);
            return;
        }
        else if (selected == "guildconf-paste"){ // Show button enable/disable external copy/paste

            const auto guil = tf_guild_info[ev.command.guild_id];
            if (!guil) {
                ev.reply(make_ephemeral_message("Something went wrong! Guild do not exist?! Please report error! I'm so sorry."));
                return;
            }

            dpp::message msg(ev.command.channel_id, "**Paste command configuration**");
            msg.add_component(
                dpp::component()
                    .add_component(
                        make_boolean_button(guil->allow_external_paste)
                        .set_label("Allow external paste?")
                        .set_id("guildconf-paste-switch")
                    )
            );
            msg.set_flags(64);
            ev.reply(dpp::ir_update_message, msg, error_autoprint);
            return;
        }
        else if (selected == "guildconf-member_points"){ // Show buttons "select user", "set value/`$current_value`"
            dpp::message msg(ev.command.channel_id, "**Handle a user's point in this guild**");
            msg.add_component(
                dpp::component()
                    .add_component(dpp::component()
                        .set_type(dpp::cot_button)
                        .set_style(dpp::cos_primary)
                        .set_label("Select a user by ID")
                        .set_id("guildconf-member_points-select_userid")
                    )
                    .add_component(dpp::component()
                        .set_type(dpp::cot_button)
                        .set_style(dpp::cos_secondary)
                        .set_label("Select user first")
                        .set_disabled(true)
                        .set_id("guildconf-member_points-select_userpts")
                    )
            );
            msg.set_flags(64);
            ev.reply(dpp::ir_update_message, msg, error_autoprint);
            return;
        }
        else if (selected == "guildconf-leveling_roles"){ // Show buttons "select user", "set value/`$current_value`"
            dpp::message msg(ev.command.channel_id, "**Leveling setup**");
            msg.add_component(
                dpp::component()
                    .add_component(dpp::component()
                        .set_type(dpp::cot_button)
                        .set_style(dpp::cos_primary)
                        .set_label("Select specific chat to log leveling")
                        .set_id("guildconf-leveling_roles-select_chid")
                    )
                    .add_component(dpp::component()
                        .set_type(dpp::cot_button)
                        .set_style(dpp::cos_secondary)
                        .set_emoji("📩") // 🗑️✅📩 // discard all (disable select), use channel (enable select), on message (disable select)
                        .set_id("guildconf-leveling_roles-switch_channel")
                    )
            );
            msg.add_component(
                dpp::component()
                    .add_component(dpp::component()
                        .set_type(dpp::cot_button)
                        .set_style(dpp::cos_secondary)
                        .set_label("Manage roles per level")
                        .set_id("guildconf-leveling_roles-spawn_role_setup")
                    )
            );
            msg.set_flags(64);
            ev.reply(dpp::ir_update_message, msg, error_autoprint);
            return;
        }
        else if (selected == "guildconf-roles_command"){ // selectable list "guild-roles_command-list" + buttons: "new" (list), "replace" (list) and "trashcan" (select)

            const auto guil = tf_guild_info[ev.command.guild_id];
            if (!guil) {
                ev.reply(make_ephemeral_message("Something went wrong! Guild do not exist?! Please report error! I'm so sorry."));
                return;
            }

            size_t offset = 0;

            // first new message
            dpp::message msg(ev.command.channel_id, "**Role commands**");
            
            ev.reply(dpp::ir_update_message, roleguild_auto_do(guil, msg, roleguild_tasks::UPDATE, {}), error_autoprint);
            return;
        }
        else if (selected == "guildconf-auto_roles") { // selectable list "guild-auto_roles" + buttons: "new" (list) and "trashcan"

            const auto guil = tf_guild_info[ev.command.guild_id];
            if (!guil) {
                ev.reply(make_ephemeral_message("Something went wrong! Guild do not exist?! Please report error! I'm so sorry."));
                return;
            }

            dpp::message msg(ev.command.channel_id, "**Automatic roles on join**");
            msg.add_component(
                dpp::component()
                    .add_component(dpp::component()
                        .set_type(dpp::cot_button)
                        .set_style(dpp::cos_primary)
                        .set_label("Add a role to the list")
                        .set_disabled(guil->roles_when_join.size() >= guild_props::max_onjoin_roles_len)
                        .set_id("guildconf-auto_roles-add")
                        .set_emoji("🆕")
                    )
                    .add_component(dpp::component()
                        .set_type(dpp::cot_button)
                        .set_style(dpp::cos_secondary)
                        .set_label("Remove a role from the list")
                        .set_disabled(guil->roles_when_join.size() == 0)
                        .set_id("guildconf-auto_roles-del")
                        .set_emoji("🗑️")
                    )
            );

            msg.content = "**Current on join roles list [" + std::to_string(guil->roles_when_join.size()) + "/" + std::to_string(guild_props::max_onjoin_roles_len) + "]:**\n```cs\n";

            if (guil->roles_when_join.size()){
                dpp::cache<dpp::role>* cach = dpp::get_role_cache();
                {
                    std::shared_lock<std::shared_mutex> lu(cach->get_mutex());
                    auto& rols = cach->get_container();

                    for(const auto& it : guil->roles_when_join) {
                        msg.content += std::to_string(it) + ": #";
                        auto found = std::find_if(rols.begin(), rols.end(), [&](const std::pair<dpp::snowflake, dpp::role*>& s){ return s.first == it;});
                        if (found != rols.end()) msg.content += found->second->name;
                        msg.content += "\n";
                    }
                }
            }
            else {
                msg.content += "<empty>";
            }

            msg.content += "\n```";

            msg.set_flags(64);
            ev.reply(dpp::ir_update_message, msg, error_autoprint);
            return;
        }
        else if (selected == "guildconf-leveling_roles"){ // Show buttons for "messages? (true/false)" "where? (modal w/ chat name or id)"

        }
    }
    else if (ev.custom_id == "guildconf-roles_command-select") {
        const std::string name_sel = ev.values[0];

        const auto guil = tf_guild_info[ev.command.guild_id];
        if (!guil) {
            ev.reply(make_ephemeral_message("Something went wrong! Guild do not exist?! Please report error! I'm so sorry."));
            return;
        }
        
        ev.reply(dpp::ir_update_message, roleguild_auto_do(guil, ev.command.msg, roleguild_tasks::SELECTUPDATE, name_sel), error_autoprint);
        return;
    }
    ev.reply(make_ephemeral_message("Unexpected command. How is this possible? Command id that failed: `" + ev.custom_id + "`."));
    




    //const auto you = tf_user_info[ev.command.usr.id];
    //if (!you) {
    //    ev.reply(make_ephemeral_message("Something went wrong! You do not exist?! Please report error! I'm so sorry.");
    //    return;
    //}    

}

void g_on_interaction(const dpp::interaction_create_t& ev)
{
    //ev.thinking();
    if (ev.command.type == dpp::interaction_type::it_ping) {
        ev.reply(dpp::interaction_response_type::ir_pong, "");
        return;
    }
    if (ev.command.type != dpp::interaction_type::it_application_command)
    {
        cout << console::color::GOLD << "Unexpected other type on interaction: #" << ev.command.type;
        return;
    }

    dpp::command_interaction cmd = std::get<dpp::command_interaction>(ev.command.data);   
    

    const auto you = tf_user_info[ev.command.usr.id];
    if (!you) {
        ev.reply(make_ephemeral_message("Something went wrong! You do not exist?! Please report error! I'm so sorry."));
        return;
    }

    you->commands_used++;

    bool went_good = false;

    switch(slash_to_discord_command(cmd.name)) {
        case discord_slashcommands::BOTSTATUS:
            went_good = run_botstatus(ev, cmd);
            break;
        case discord_slashcommands::CONFIGURATION:
            went_good = run_config_server(ev, cmd);
            break;
        case discord_slashcommands::PASTE:
            went_good = run_paste(ev, cmd);
            break;
        case discord_slashcommands::PING:
            went_good = run_ping(ev);
            break;
        case discord_slashcommands::POLL:
            went_good = run_poll(ev, cmd);
            break;
        case discord_slashcommands::ROLES:
            break;
        case discord_slashcommands::SELF:
            went_good = run_self(ev);
            break;
        case discord_slashcommands::RC_SHOWINFO:
            went_good = run_showinfo(ev);
            break;
        case discord_slashcommands::RC_COPY:
            went_good = run_copy(ev, cmd);
            break;
        default:
            break;
    }

    if (!went_good) {
        ev.reply(make_ephemeral_message("Oh no, something went wrong. I can't remember what I should do in this case. Please report! :("));
    }
        
}

void g_tick_presence(const safe_data<general_config>& g, dpp::cluster& bot)
{
    g.csafe<void>([&bot](const general_config& gc){
        dpp::activity act = dpp::activity(static_cast<dpp::activity_type>(gc.status_code), gc.status_text, "", gc.status_link);
        dpp::presence pres(static_cast<dpp::presence_status>(gc.status_mode), act);
        bot.set_presence(pres);
    });
}

void g_apply_guild_local_commands(dpp::cluster& bot, const safe_data<std::vector<slash_local>>& conf)
{    
    conf.csafe<void>([&bot](const std::vector<slash_local>& vec){

        // there's a bulk version for create on discord too. Check that.
        for(const auto& i : vec) {
            // do apply things
        }
    });
}

void input_handler_cmd(dpp::cluster& bot, bool& _keep, safe_data<general_config>& config, safe_data<std::vector<slash_local>>& lslashes, const safe_data<slash_global>& gslash, const std::string& cmd)
{
    std::string arg;
    switch(g_interp_cmd(cmd, arg)) {
    case commands::HELP:
    {
        cout << console::color::GRAY << "Help:";
        cout << console::color::GRAY << "> help            : Shows this help";
        cout << console::color::GRAY << "> exit            : Close bot properly (or as close as possible to that)";
        cout << console::color::GRAY << "> sss [string]    : [Set Status String] Set status text (presence)";
        cout << console::color::GRAY << "> ssm [uint]      : [Set Status Mode] Set status mode (0-3; off, on, dnd, idle)" ;
        cout << console::color::GRAY << "> ssc [uint]      : [Set Status Code] Set status code (0-5; game, streaming, listening, watching, custom, competing)";
        cout << console::color::GRAY << "> ssl <string>    : [Set Status Link] Set status link (if mode allows it)";
        cout << console::color::GRAY << "> rgl <bool>      : [Request Guilds List] Get guild list. Optional: verbose?";
        cout << console::color::GRAY << "> rgs [snowflake] : [Request Guilds Snowflake] Get information about a guild on list by SNOWFLAKE.";
        cout << console::color::GRAY << "> rus [snowflake] : [Request User Snowflake] Search and try to get information of user by SNOWFLAKE.";
        cout << console::color::GRAY << "> rsc             : [Reset Slash Commands] Clean up local guild slash commands and bulk global ones. (WARN: DO ONLY FOR DEBUG! MAY BREAK GUILD CONFIGURED STUFF!)";
        cout << console::color::GRAY << "> mem             : Get memory information (things in memory)";
        cout << console::color::GRAY << "> dgs             : [Delete Global Slash] Remove global commands from the bot (DANGEROUS!)";
    }
        break;
    case commands::EXIT:
    {
        cout << console::color::YELLOW << "[MAIN] Closing stuff soon.";
        _keep = false;
    }
        break;
    case commands::SETSTATUSSTR:
    {
        config.safe<void>([&](general_config& g){
            g.status_text = arg;
            cout << console::color::GREEN << "[MAIN] Status string updated to '" << g.status_text << "'.";
        });
    }
        break;
    case commands::SETSTATUSMODE: // online, offline etc
    {
        config.safe<void>([&](general_config& g){
            if (arg.size()) sscanf(arg.c_str(), "%u", &g.status_mode);
            if (g.status_mode > 3) g.status_mode = 0;
            cout << console::color::GREEN << "[MAIN] Status mode updated to '" << g.status_mode << "'.";
        });
    }
        break;
    case commands::SETSTATUSCODE: // streaming, listening etc
    {
        config.safe<void>([&](general_config& g){
            if (arg.size()) sscanf(arg.c_str(), "%u", &g.status_code);
            if (g.status_code > 5) g.status_code = 0;
            cout << console::color::GREEN << "[MAIN] Status code updated to '" << g.status_code << "'.";
        });
    }
        break;
    case commands::SETSTATUSLINK:
    {
        config.safe<void>([&](general_config& g){
            g.status_link = arg;
            cout << console::color::GREEN << "[MAIN] Status link updated to '" << g.status_link << "'.";
        });
    }
        break;
    case commands::REQUESTGUILDLIST:
    {
        cout << console::color::GRAY << "[MAIN] Checking on cache...";

        auto* map = dpp::get_guild_cache();
        std::shared_lock<std::shared_mutex> lu(map->get_mutex());

        cout << console::color::GREEN << "[MAIN] Bot is currently on: " << map->count() << " guild(s).";

        if (arg.find("true") == 0) { // verbose
            cout << console::color::GRAY << "[MAIN] Detailed: ";

            for(const auto& it : map->get_container()) {
                cout << console::color::GREEN << g_smash_guild_info(*it.second);
            }
        }
    }
        break;
    case commands::REQUESTGUILDSNOWFLAKE:
    {
        const auto gid = std::stoull(arg);
        if (gid == 0){
            cout << console::color::RED << "[MAIN] Invalid ID.";
            break;
        }

        cout << console::color::GRAY << "[MAIN] Checking on cache...";

        auto* map = dpp::get_guild_cache();
        std::shared_lock<std::shared_mutex> lu(map->get_mutex());

        auto* g = map->find(gid);
        if (!g) {
            cout << console::color::YELLOW << "[MAIN] Can't find Guild #" << gid << ".";
            break;
        }
        
        cout << console::color::GREEN << "==== About Guild #" << g->id << " ====";
        cout << console::color::GREEN << "Name: " << g->name;
        cout << console::color::GREEN << "Member count (approx): " << g->member_count;
        cout << console::color::GREEN << "Is community?: " << (g->is_community() ? "Y" : "N");
        cout << console::color::GREEN << "Is discoverable?: " << (g->is_discoverable() ? "Y" : "N");
        cout << console::color::GREEN << "Is featureable?: " << (g->is_featureable() ? "Y" : "N");
        cout << console::color::GREEN << "Member max count: " << g->max_members;
        cout << console::color::GREEN << "NSFW status: " << g_transl_nsfw_code(g->nsfw_level);
        cout << console::color::GREEN << "System Channel ID: " << g->system_channel_id;
        cout << console::color::GREEN << "Rules Channel ID: " << g->rules_channel_id;
        cout << console::color::GREEN << "Widget Channel ID: " << g->widget_channel_id;
        cout << console::color::GREEN << "AFK Channel ID: " << g->afk_channel_id;
        cout << console::color::GREEN << "AFK timeout: " << g->afk_timeout;
    }
        break;
    case commands::REQUESTUSERSNOWFLAKE:
    {
        const auto uid = std::stoull(arg);
        if (uid == 0){
            cout << console::color::RED << "[MAIN] Invalid ID.";
            break;
        }

        cout << console::color::GRAY << "[MAIN] Checking on cache...";

        auto* map = dpp::get_user_cache();
        std::shared_lock<std::shared_mutex> lu(map->get_mutex());

        auto* u = map->find(uid);
        if (!u) {
            cout << console::color::YELLOW << "[MAIN] Can't find User #" << uid << ".";
            break;
        }

        cout << console::color::GREEN << "==== About User #" << u->id << " ====";
        cout << console::color::GREEN << "User: " << u->username;
        cout << console::color::GREEN << "Discriminator: " << u->discriminator;
        cout << console::color::GREEN << "Has Nitro classic? " << (u->has_nitro_classic() ? "Y" : "N");
        cout << console::color::GREEN << "Has Nitro full? " << (u->has_nitro_classic() ? "Y" : "N");
        cout << console::color::GREEN << "Is bot? " << (u->is_bot() ? "Y" : "N");
        cout << console::color::GREEN << "Has MFA? " << (u->is_mfa_enabled() ? "Y" : "N");
        cout << console::color::GREEN << "Is Discord employee? " << (u->is_discord_employee() ? "Y" : "N");
        cout << console::color::GREEN << "Is verified? " << (u->is_verified() ? "Y" : "N");
        cout << console::color::GREEN << "Is verified bot? " << (u->is_verified_bot() ? "Y" : "N");
        cout << console::color::GREEN << "Is verified bot developer? " << (u->is_verified_bot_dev() ? "Y" : "N");
        cout << console::color::GREEN << "Guilds on (known to this bot): " << u->refcount;
    }
        break;
    case commands::RESETSLASHCOMMANDS:
    {
        cout << console::color::GRAY << "[MAIN] Refreshing global commands...";

        gslash.csafe<void>([&bot](const slash_global& g){ g.apply_bulk(bot); });

        cout << console::color::GRAY << "[MAIN] Loading guild list from cache...";

        std::vector<dpp::snowflake> gids;
        {
            auto* map = dpp::get_guild_cache();
            std::shared_lock<std::shared_mutex> lu(map->get_mutex());
            for(const auto& fun : map->get_container()) {
                gids.push_back(fun.first);
            }
        }

        cout << console::color::GRAY << "[MAIN] Setting up reapply bomb...";

        auto thebomb = std::shared_ptr<bomb>(new Lunaris::bomb([&bot,&lslashes]{
            cout << console::color::GRAY << "[MAIN] Reapply bomb triggered. Reapplying local guild commands...";
            g_apply_guild_local_commands(bot, lslashes);
            cout << console::color::GREEN << "[MAIN] All local guild commands will be available sometime soon.";
        }));

        cout << console::color::GRAY << "[MAIN] Cleaning up all " << gids.size() << " guilds...";

        for(const auto& i : gids){
            bot.guild_commands_get(i, [i,&bot,thebomb](const dpp::confirmation_callback_t& dat) {
                if (dat.is_error()) {
                    cout << console::color::RED << "[MAIN] Error getting slash command info from Guild #" << i << " (clear slash commands command): " << dat.get_error().message;
                    return;
                }

                auto smap = std::get<dpp::slashcommand_map>(dat.value);

                if (smap.size()) cout << console::color::GRAY << "[MAIN] Queueing " << smap.size() << " cleaning tasks for guild #" << i << ".";
                else cout << console::color::DARK_GRAY << "[MAIN] No cleaning tasks needed for guild #" << i << ".";

                for(const auto& it : smap) {
                    bot.guild_command_delete(it.second.id, i, 
                        [i, id = it.second.id, thebomb](const dpp::confirmation_callback_t& conf){ 
                            if (conf.is_error()) {cout << console::color::RED << "[MAIN] Error deleting slash command #" << id << " from Guild #" << i << " (clear slash commands command): " << conf.get_error().message; } 
                        });
                }
            });
        }
    }
        break;
    case commands::MEMSTATUS:
    {
        cout << console::color::GREEN << "[MAIN] Data in memory (custom data):";
        cout << console::color::GREEN << "[MAIN] Users: " << tf_user_info.size();
    }
        break;
    case commands::DELETEGLOBALSLASH:
        cout << console::color::GREEN << "[MAIN] Queued slash commands removal.";
        gslash.csafe<void>([&bot](const slash_global& g){ g.remove_global(bot); });
        break;
    }

}


commands g_interp_cmd(const std::string& s, std::string& o)
{
    if (s.find("help") == 0) { if (s.size() > 5) { o = s.substr(5); } return commands::HELP; }
    if (s.find("exit") == 0) { if (s.size() > 5) { o = s.substr(5); } return commands::EXIT; }
    if (s.find("sss") == 0)  { if (s.size() > 4) { o = s.substr(4); } return commands::SETSTATUSSTR; }
    if (s.find("ssm") == 0)  { if (s.size() > 4) { o = s.substr(4); } return commands::SETSTATUSMODE; }
    if (s.find("ssc") == 0)  { if (s.size() > 4) { o = s.substr(4); } return commands::SETSTATUSCODE; }
    if (s.find("ssl") == 0)  { if (s.size() > 4) { o = s.substr(4); } return commands::SETSTATUSLINK; }
    if (s.find("rgl") == 0)  { if (s.size() > 4) { o = s.substr(4); } return commands::REQUESTGUILDLIST; }
    if (s.find("rgs") == 0)  { if (s.size() > 4) { o = s.substr(4); } return commands::REQUESTGUILDSNOWFLAKE; }
    if (s.find("rus") == 0)  { if (s.size() > 4) { o = s.substr(4); } return commands::REQUESTUSERSNOWFLAKE; }
    if (s.find("rsc") == 0)  { if (s.size() > 4) { o = s.substr(4); } return commands::RESETSLASHCOMMANDS; }
    if (s.find("mem") == 0)  { if (s.size() > 4) { o = s.substr(4); } return commands::MEMSTATUS; }
    if (s.find("dgs") == 0)  { if (s.size() > 4) { o = s.substr(4); } return commands::DELETEGLOBALSLASH; }
    return commands::NONE;
}

std::string g_transl_nsfw_code(const dpp::guild_nsfw_level_t& c)
{
    switch(c){
    case dpp::guild_nsfw_level_t::nsfw_safe:
        return "SAFE";
    case dpp::guild_nsfw_level_t::nsfw_explicit:
        return "EXPLICIT";
    case dpp::guild_nsfw_level_t::nsfw_age_restricted:
        return "+18";
    default:
        return "DEFAULT";
    }
}

std::string g_smash_guild_info(const dpp::guild& g)
{
    return "[#" + std::to_string(g.id) + "]"
        "{Name:" + g.name + ";"
        "MemCount:" + std::to_string(g.member_count) + ";"
        "IsCommu:" + (g.is_community() ? "Y" : "N") + ";"
        "MemMax:" + std::to_string(g.max_members) + ";"
        "NSFWlvl:" + g_transl_nsfw_code(g.nsfw_level) + ";"
        "SysCHID:" + std::to_string(g.system_channel_id) + "}";
}

void setup_bot(dpp::cluster& bot, safe_data<slash_global>& sg)
{
    bot.on_log(g_on_log);
    bot.on_ready([&sg](const dpp::ready_t& arg){ g_on_ready(arg, sg); });
    bot.on_form_submit([&](const dpp::form_submit_t& arg){ g_on_modal(arg); });
    bot.on_button_click([&](const dpp::button_click_t& arg) { g_on_button_click(arg); });
    bot.on_select_click([&](const dpp::select_click_t& arg) { g_on_select(arg); });
    bot.on_interaction_create([&](const dpp::interaction_create_t& arg) { g_on_interaction(arg); });
    bot.on_message_reaction_add([&](const dpp::message_reaction_add_t& arg) { g_on_react(arg); });
    bot.on_message_create([&](const dpp::message_create_t& arg) { g_on_message(arg); });
}

void error_autoprint(const dpp::confirmation_callback_t& err)
{
    if (err.is_error()) 
        cout << console::color::RED << "Response error: " << err.http_info.body;
}

std::unique_ptr<dpp::cluster> build_bot_from(safe_data<general_config>& c)
{
    return c.csafe<std::unique_ptr<dpp::cluster>>([](const general_config& g) -> std::unique_ptr<dpp::cluster> {
        return std::unique_ptr<dpp::cluster>(new dpp::cluster(g.token, g.intents, g.shard_count));
    });
}

bool change_component(std::vector<dpp::component>& vec, const std::string& key, std::function<void(dpp::component&)> doo)
{
    for(auto& i : vec){
        if (i.custom_id == key) {
            doo(i);
            return true;
        }
        else if (i.components.size()) {
            if (change_component(i.components, key, doo)) return true;
        }
    }
    return false;
}

std::string get_customid_as_str(const std::vector<dpp::component>& v, const std::string& key)
{
    for(const auto& i : v) {
        if (i.custom_id == key) {
            try {
                return std::get<std::string>(i.value);
            }
            catch(...) {
                return {};
            }
        }
        if (i.components.size()) {
            auto _str = get_customid_as_str(i.components, key);
            if (_str.size()) return _str;
        }
    }
    return {};
}

std::string get_label(const std::vector<dpp::component>& v, const std::string& key)
{
    for(const auto& i : v) {
        if (i.custom_id == key) {
            return i.label;
        }
        if (i.components.size()) {
            auto _str = get_label(i.components, key);
            if (_str.size()) return _str;
        }
    }
    return {};
}

dpp::component make_boolean_button(const bool m)
{
    dpp::component _tmp;
    set_boolean_button(m, _tmp);
    return _tmp;
}

dpp::component& set_boolean_button(const bool m, dpp::component& d)
{
    return d
        .set_emoji(confirm_emojis[static_cast<size_t>(m)])
        .set_type(dpp::cot_button)
        .set_style(m ? dpp::cos_success : dpp::cos_danger);
}

dpp::message make_ephemeral_message(const std::string& str)
{
    dpp::message msg;
    msg.set_content(str);
    msg.set_flags(64);
    return msg;
}

dpp::message roleguild_auto_do(const std::shared_ptr<guild_info>& guil, const dpp::message& origmsg, const roleguild_tasks task, std::variant<std::monostate, guild_info::pair_id_name, std::string> input)
{
    dpp::message cpy = origmsg;
    std::string selected_group;
    size_t offset_group = 0;
    bool is_empty = false;
    std::string emplaced_error;

    cpy.set_flags(64);

    const auto f_find_ret_sizet = [&](const std::string& s) {
        for(size_t oof = 0; oof < guil->roles_available.size(); ++oof) {
            if (guil->roles_available[oof].name == s) return oof;
        }
        return static_cast<size_t>(-1);
    };

    const auto f_reset_select = [&]{
        offset_group = 0;
        is_empty = guil->roles_available.empty();
        selected_group.clear();
    };

    switch(task) {
    case roleguild_tasks::SELECTUPDATE:
    {
        selected_group = std::get<std::string>(input);
        offset_group = f_find_ret_sizet(selected_group);
        is_empty = false;

        if (offset_group > guil->roles_available.size()) {
            emplaced_error = "Something went wrong while indexing groups.";
            f_reset_select();
        }
    }
        break;
    case roleguild_tasks::UPDATE:
    {
        if (guil->roles_available.size()) {
            selected_group = get_label(origmsg.components, "guildconf-roles_command-selected");

            offset_group = f_find_ret_sizet(selected_group);
            is_empty = false;

            if (offset_group > guil->roles_available.size()) {
                f_reset_select();
            }
        }
        else {
            f_reset_select();
        }
    }
        break;
    case roleguild_tasks::GROUP_ADD:
    {
        if (guil->roles_available.size() >= guild_props::max_role_groups) {
            emplaced_error = "Already full of groups!";

            selected_group = get_label(origmsg.components, "guildconf-roles_command-selected");
            offset_group = f_find_ret_sizet(selected_group);

            if (offset_group > guil->roles_available.size()) {
                emplaced_error = "Something went wrong while handling full group error.";
                f_reset_select();
            }
        }
        else {
            selected_group = std::get<std::string>(input);
            
            auto it = std::find_if(guil->roles_available.begin(), guil->roles_available.end(), [&](const guild_info::category& c){ return c.name == selected_group; });
            if (it == guil->roles_available.end()) {
                guild_info::category cat;
                cat.name = selected_group;
                guil->roles_available.push_back(cat);
                offset_group = guil->roles_available.size() - 1;
                is_empty = false;
            }
            else {
                emplaced_error = "Group already exists!";

                if (guil->roles_available.size()){
                    offset_group = f_find_ret_sizet(selected_group);

                    if (offset_group > guil->roles_available.size()){
                        emplaced_error = "Something went wrong while listing internal list of groups.";
                        f_reset_select();
                    }
                }
                else { // already exists, but empty group? HOW?
                    emplaced_error = "Something went wrong while listing internal list of groups.";
                    f_reset_select();
                }
            }
        }
    }
        break;
    case roleguild_tasks::GROUP_REMOVE:
    {
        const std::string& todel = std::get<std::string>(input);

        auto it = std::find_if(guil->roles_available.begin(), guil->roles_available.end(), [&](const guild_info::category& c){ return c.name == todel; });

        if (it != guil->roles_available.end()) {
            guil->roles_available.erase(it);
            f_reset_select();
        }
        else {
            emplaced_error = "Cannot find this group to delete.";

            selected_group = get_label(origmsg.components, "guildconf-roles_command-selected");
            offset_group = f_find_ret_sizet(selected_group);

            if (offset_group > guil->roles_available.size()) {
                emplaced_error = "Something went wrong while listing internal list of groups.";
                f_reset_select();
            }
        }
    }
        break;
    case roleguild_tasks::ROLE_ADD:
    case roleguild_tasks::ROLE_REMOVE:
    {
        if (!guil->roles_available.empty()) {

            // SELECT AND OFFSET ARE SET HERE:
            selected_group = get_label(origmsg.components, "guildconf-roles_command-selected");

            // OFFSET HERE, NO WORRIES LOL:
            if ((offset_group = f_find_ret_sizet(selected_group)) < guil->roles_available.size()) // true if found
            {
                is_empty = false;
                if (task == roleguild_tasks::ROLE_REMOVE) 
                {
                    const std::string key_task = std::get<std::string>(input); // add or remove what?

                    if (key_task == "*") {
                        guil->roles_available[offset_group].list.clear();
                    }
                    else {
                        const unsigned long long transl_val = dpp::from_string<dpp::snowflake>(key_task); // may be 0, no prob tho, it fails later

                        auto it = std::find_if(guil->roles_available[offset_group].list.begin(), guil->roles_available[offset_group].list.end(),
                            [&](const guild_info::pair_id_name& p) { return p.id == transl_val;});

                        if (it != guil->roles_available[offset_group].list.end()) guil->roles_available[offset_group].list.erase(it);
                    }
                }
                else 
                {
                    if (guil->roles_available[offset_group].list.size() >= guild_props::max_role_group_each) {
                        emplaced_error = "Already full of roles on this group!";
                    }
                    else {
                        const guild_info::pair_id_name addin = std::get<guild_info::pair_id_name>(input); // add or remove what?

                        auto it = std::find_if(guil->roles_available[offset_group].list.begin(), guil->roles_available[offset_group].list.end(),
                            [&](const guild_info::pair_id_name& p) { return p.id == addin.id;});

                        if (it == guil->roles_available[offset_group].list.end()) {
                            guil->roles_available[offset_group].list.push_back(addin);
                        }
                        else { 
                            it->name = addin.name;
                        }
                    }
                }
            }
            else { // offset is broken / EOF
                emplaced_error = "Something went wrong while listing internal list of groups.";
                f_reset_select();
            }
        }
        else {
            is_empty = true;
        }
    }
        break;
    }

    if (!is_empty && selected_group.empty() && offset_group < guil->roles_available.size()) selected_group = guil->roles_available[offset_group].name;

    const bool tags_failed_once_redo = 
        !change_component(cpy.components, "guildconf-roles_command-select", [&](dpp::component& d) {
            if (guil->roles_available.size()) {
                d.options.clear();
                for(const auto& each : guil->roles_available)
                    d.add_select_option(dpp::select_option(each.name, each.name, "Select to manage it"));
            }
            else {
                for(auto msrc = cpy.components.begin(); msrc != cpy.components.end();)
                {
                    bool erased = false;
                    for(const auto& i : msrc->components){
                        if (i.custom_id == "guildconf-roles_command-select") {
                            cpy.components.erase(msrc);
                            msrc = cpy.components.end();
                            erased = true;
                        }
                    }
                    if (!erased) ++msrc;
                }
            }
        }) || !change_component(cpy.components, "guildconf-roles_command-addgroup", [&](dpp::component& d) {
            d.set_disabled(guil->roles_available.size() >= guild_props::max_role_groups);
        }) || !change_component(cpy.components, "guildconf-roles_command-delgroup", [&](dpp::component& d) {
            d.set_disabled(guil->roles_available.size() == 0);
        }) || !change_component(cpy.components, "guildconf-roles_command-selected", [&](dpp::component& d) {
            d.set_label(selected_group.size() ? selected_group : "<none selected>");
        }) || !change_component(cpy.components, "guildconf-roles_command-add", [&](dpp::component& d) {
            d.set_disabled(is_empty ? true : (guil->roles_available[offset_group].list.size() >= guild_props::max_role_group_each));
        }) || !change_component(cpy.components, "guildconf-roles_command-del", [&](dpp::component& d) {
            d.set_disabled(is_empty ? true : (guil->roles_available[offset_group].list.size() == 0));
        });

    if (tags_failed_once_redo) 
    {
        cpy.components.clear();

        if (guil->roles_available.size()) {
            dpp::component clist;
            clist.set_label("Configurations");
            clist.set_id("guildconf-roles_command-select");
            clist.set_type(dpp::cot_selectmenu);

            for(const auto& each : guil->roles_available) {
                clist.add_select_option(dpp::select_option(each.name, each.name, "Select to manage it"));
            }

            cpy.add_component(dpp::component().add_component(clist));
        }
        cpy.add_component(
            dpp::component()
                .add_component(dpp::component()
                    .set_type(dpp::cot_button)
                    .set_style(dpp::cos_primary)
                    .set_label("New role list")
                    .set_disabled(guil->roles_available.size() >= guild_props::max_role_groups)
                    .set_id("guildconf-roles_command-addgroup")
                    .set_emoji("🌟")
                )
                .add_component(dpp::component()
                    .set_type(dpp::cot_button)
                    .set_style(dpp::cos_danger)
                    .set_label("Remove role list")
                    .set_disabled(guil->roles_available.size() == 0)
                    .set_id("guildconf-roles_command-delgroup")
                    .set_emoji("❌")
                )
                .add_component(dpp::component()
                    .set_type(dpp::cot_button)
                    .set_style(dpp::cos_secondary)
                    .set_label(selected_group.size() ? selected_group : "<none selected>")
                    .set_id("guildconf-roles_command-selected")
                    .set_disabled(true)
                )
                .add_component(dpp::component()
                    .set_type(dpp::cot_button)
                    .set_style(dpp::cos_primary)
                    .set_label("Add a role to the list")
                    .set_disabled(is_empty ? true : (guil->roles_available[offset_group].list.size() >= guild_props::max_role_group_each))
                    .set_id("guildconf-roles_command-add")
                    .set_emoji("🆕")
                )
                .add_component(dpp::component()
                    .set_type(dpp::cot_button)
                    .set_style(dpp::cos_secondary)
                    .set_label("Remove a role from the list")
                    .set_disabled(is_empty ? true : (guil->roles_available[offset_group].list.size() == 0))
                    .set_id("guildconf-roles_command-del")
                    .set_emoji("🗑️")
                )
        );
    }

    if (is_empty) {
        cpy.content = "**Config is empty**\nStart by creating a new role list!" + (emplaced_error.size() ? ("\n\n**ERROR:** " + emplaced_error) : "");
        return cpy;
    }

    const auto& selectd = guil->roles_available[offset_group];

    cpy.content = "**Current list [" + 
        std::to_string(selectd.list.size()) + "/" + std::to_string(guild_props::max_role_group_each) + " roles, " +
        std::to_string(guil->roles_available.size()) + "/" + std::to_string(guild_props::max_role_groups) + " groups]:**\n```cs\n";

    cpy.content += "Group selected: " + selected_group + "\n";

    if (selectd.list.size()){
        dpp::cache<dpp::role>* cach = dpp::get_role_cache();
        {
            std::shared_lock<std::shared_mutex> lu(cach->get_mutex());
            auto& rols = cach->get_container();

            for(const auto& it : selectd.list) {
                cpy.content += " " + std::to_string(it.id) + " [" + it.name + "]: #";
                auto found = std::find_if(rols.begin(), rols.end(), [&](const std::pair<dpp::snowflake, dpp::role*>& s){ return s.first == it.id;});
                if (found != rols.end()) cpy.content += found->second->name;
                cpy.content += "\n";
            }
        }
    }
    else {
        cpy.content += " <empty>";
    }

    if (emplaced_error.size()) cpy.content += "\n\n**ERROR:** " + emplaced_error;

    cpy.content += "\n```\n**Select another group below, if you want to:**";

    return cpy;
}

//dpp::component make_selectable_list(const std::string& listkey, const size_t page, std::vector<dpp::select_option> lst)
//{
//    dpp::component selector;
//    for(const auto& it : lst) selector.add_select_option(it);
//    selector.set_label("Select item");
//    selector.set_id(listkey);
//
//    dpp::component d;
//    
//    d.add_component(dpp::component().add_component(selector));
//    d.add_component(dpp::component()
//        .add_component(dpp::component()
//            .set_type(dpp::cot_button)
//            .set_label("previous")
//            .set_emoji(navigation_emojis[0])
//            .set_style(dpp::cos_secondary)
//            .set_id(listkey + "_prev")
//        )
//        .add_component(dpp::component()
//            .set_type(dpp::cot_button)
//            .set_label("page " + std::to_string(page))
//            .set_emoji(navigation_emojis[1])
//            .set_style(dpp::cos_secondary)
//            .set_id(listkey + "_page")
//        )
//        .add_component(dpp::component()
//            .set_type(dpp::cot_button)
//            .set_label("next")
//            .set_emoji(navigation_emojis[2])
//            .set_style(dpp::cos_secondary)
//            .set_id(listkey + "_next")
//        )
//    );
//    
//    return d;
//}
//
//dpp::component& update_selectable_list(dpp::component& d, const size_t page, std::vector<dpp::select_option> lst)
//{
//    return (d = make_selectable_list(d.custom_id, page, lst));
//}

bool auto_handle_button_switch(const dpp::interaction_create_t& ev, const std::string& key, std::function<void(dpp::component&)> f)
{
    dpp::message cpy = ev.command.msg;
    const bool gud = change_component(cpy.components, key, f);
    if (!gud) { ev.reply(dpp::ir_update_message, "This interaction failed. Please try again later!", error_autoprint); return false; }
    ev.reply(dpp::ir_update_message, cpy, error_autoprint);
    return true;
}

int64_t interpret_color(const std::string& str)
{
    if (str.empty()) return -1;

    else if (str.find("0x") == 0) { // HEX
        if (str.length() <= 2) return -1;
        char* got_on = nullptr;
        return std::strtoll(str.c_str() + 2, &got_on, 16);
    }
    else if (str[0] <= '9' && str[0] >= '0') { // DEC
        char* got_on = nullptr;
        return std::strtoll(str.c_str(), &got_on, 10);
    }
    else { // literal
        if (str == "red") return 0xFF0000;
        if (str == "green") return 0x00FF00;
        if (str == "blue") return 0x0000FF;
        if (str == "yellow") return 0xFFFF00;
        if (str == "cyan") return 0x00FFFF;
        if (str == "magenta") return 0xFF00FF;
        if (str == "white") return 0xFFFFFF;
        if (str == "black") return 0x000000;
        if (str == "default") return -1;
    }
    return -1;
}

std::string print_hex(const int64_t v)
{
    std::stringstream stream;
    stream << std::hex << v;
    return "0x" + stream.str();
}

discord_slashcommands slash_to_discord_command(const std::string& str)
{
    if (str == "botstatus") return discord_slashcommands::BOTSTATUS;
    if (str == "configuration") return discord_slashcommands::CONFIGURATION;
    if (str == "paste") return discord_slashcommands::PASTE;
    if (str == "ping") return discord_slashcommands::PING;
    if (str == "poll") return discord_slashcommands::POLL;
    if (str == "roles") return discord_slashcommands::ROLES;
    if (str == "self") return discord_slashcommands::SELF;
    if (str == "Show info of this user") return discord_slashcommands::RC_SHOWINFO;
    if (str == "Copy to clipboard") return discord_slashcommands::RC_COPY;
    return discord_slashcommands::UNKNOWN;
}

bool is_member_admin(const dpp::guild_member& memb)
{
    dpp::cache<dpp::role>* cached = dpp::get_role_cache();
    std::shared_lock<std::shared_mutex> lu(cached->get_mutex());
    const auto& container = cached->get_container();

    for(const auto& rols : memb.roles) {
        const auto it = container.find(rols);
        if (it == container.end()) {
            cout << console::color::YELLOW << "[CACHE] Can't find role #" << rols << "! Discarded.";
            continue;
        }
        if (it->second->has_administrator()) return true;
    }
    return false;
}

// user pts, current level, points needed for next
void calc_user_level(const unsigned long long usrpts, unsigned long long& currlevel, unsigned long long& nextlvl)
{
    using mull = unsigned long long;

    currlevel = 1;
    while(static_cast<mull>(pow(leveling::calc_level_div, currlevel + leveling::threshold_points_level_0 - 1)) <= usrpts) ++currlevel;

    const mull next_level_raw = static_cast<mull>(pow(leveling::calc_level_div, currlevel + (leveling::threshold_points_level_0 - 1)));
    nextlvl = (next_level_raw - usrpts);
}

bool run_botstatus(const dpp::interaction_create_t& ev, const dpp::command_interaction& cmd)
{
    dpp::message replying;
    replying.id = ev.command.id;
    replying.channel_id = ev.command.channel_id;
    replying.set_type(dpp::message_type::mt_application_command);
    replying.set_flags(64);
    replying.nonce = std::to_string(ev.command.usr.id);

    ev.reply(dpp::interaction_response_type::ir_deferred_channel_message_with_source, replying);

    double memuse_mb = 0.0;
    double resident_mb = 0.0;
    int num_threads = 0;

    {
        process_info proc;
        proc.generate();
        num_threads                  = std::stoi(proc.get(process_info::data::NUM_THREADS));
        long long raw_rss            = std::stoll(proc.get(process_info::data::RSS));
        unsigned long long raw_vsize = std::stoull(proc.get(process_info::data::VSIZE));

        memuse_mb = raw_vsize * 1.0 / 1048576; // 1024*1024
        resident_mb = raw_rss * (1.0 * sysconf(_SC_PAGE_SIZE) / 1048576);
    }


    dpp::embed autoembed = dpp::embed()
        .set_author(
            dpp::embed_author{
                .name = ev.from->creator->me.format_username(),//(core.me.username + "#" + std::to_string(core.me.discriminator)),
                .url = ev.from->creator->me.get_avatar_url(256),
                .icon_url = ev.from->creator->me.get_avatar_url(256)
            })
        .set_title("**__Bot information__**")
        //.set_description("**―――――――――――――――――――――**") // 21 lines
        //.set_footer(dpp::embed_footer().set_text(DPP_VERSION_TEXT).set_icon(images::botstatus_image_dpp_url))
        .set_color(random() % 0xFFFFFF)
        .set_thumbnail(images::botstatus_image_url)
        .add_field(
            "Online for", (u8"⏲️ " + ev.from->creator->uptime().to_string()), true
        )
        .add_field(
            "This shard ID", (u8"🧭 " + std::to_string(ev.from->shard_id)), true
        )
        .add_field(
            "Total shards", (u8"🕹️ " + std::to_string(ev.from->creator->get_shards().size())), true
        )
        .add_field(
            "Max shard amount", (u8"🗺️ " + std::to_string(ev.from->max_shards)), true
        )
        .add_field(
            "Guilds in this shard", (u8"🪀 " + std::to_string(ev.from->get_guild_count())), true
        )
        .add_field(
            "Bot CPU threads", (u8"🧵 " + std::to_string(num_threads)), true
        )
        .add_field(
            "Total memory usage", (u8"🧠 " + std::to_string(memuse_mb) + " MB"), true
        )
        .add_field(
            "Memory usage now", (u8"🧠 " + std::to_string(resident_mb) + " MB"), true
        )
        .add_field(
            "Users/Guilds in cache", (u8"🐱 " + std::to_string(tf_user_info.size()) + u8" — " + std::to_string(tf_guild_info.size()) + u8" 🐏"), true
        )
        .add_field(
            "Current DPP version", (u8"🤖 " + std::string(DPP_VERSION_TEXT)), false
        );
        
    replying.embeds.push_back(autoembed);
    replying.set_content("");
    replying.set_flags(64);

    ev.edit_response(replying, error_autoprint);

    return true;
}

bool run_self(const dpp::interaction_create_t& ev)
{
    const auto you = tf_user_info[ev.command.usr.id];
    if (!you) {
        ev.reply(make_ephemeral_message("Something went wrong! You do not exist?! Please report error! I'm so sorry."));
        return true;
    }

    dpp::message msg(ev.command.channel_id, "**Your configuration**");
    msg.add_component(
        dpp::component()
            .add_component(
                make_boolean_button(you->show_level_up_messages)
                .set_label("Show level up messages?")
                .set_id("user-show_level_up_messages")
            )
    );
    msg.add_component(
        dpp::component()
            .add_component(dpp::component()
                .set_type(dpp::cot_button)
                .set_label("Profile color: " + (you->pref_color < 0 ? "DEFAULT" : print_hex(you->pref_color)))
                .set_style(dpp::cos_secondary)
                .set_id("user-pref_color")
            )
    );
    msg.add_component(
        dpp::component()
            .add_component(dpp::component()
                .set_type(dpp::cot_button)
                .set_label("Get your user data")
                .set_style(dpp::cos_danger)
                .set_id("user-download_user_data")
            )
    );

    msg.set_flags(64);
    ev.reply(msg, error_autoprint);
    return true;
}

bool run_poll(const dpp::interaction_create_t& ev, const dpp::command_interaction& cmd)
{
    dpp::interaction_modal_response modal("poll-create", "Create a poll");
    modal
    .add_component(
        dpp::component()
            .set_label("Title of the poll (required)")
            .set_id("title")
            .set_type(dpp::cot_text)
            .set_required(true)
            .set_placeholder("Best food worldwide?")
            .set_min_length(1)
            .set_max_length(60)
            .set_text_style(dpp::text_short)
    ).add_row().add_component(
        dpp::component()
            .set_label("Describe your question (optional)")
            .set_id("desc")
            .set_type(dpp::cot_text)
            .set_placeholder("Bananas are way cooler than tomatoes. Aren't they? Sure they are. Do you agree?")
            .set_min_length(0)
            .set_max_length(2000)
            .set_text_style(dpp::text_paragraph)
    ).add_row().add_component(
        dpp::component()
            .set_label("Emojis to add as reaction (optional)")
            .set_id("emojis")
            .set_type(dpp::cot_text)
            .set_placeholder("👍;👎;... (blank for 👍;👎)")
            .set_min_length(0)
            .set_max_length(60)
            .set_text_style(dpp::text_short)
    ).add_row().add_component(
        dpp::component()
            .set_label("Image link (optional)")
            .set_id("imglink")
            .set_type(dpp::cot_text)
            .set_placeholder("https://myimage.notarealurl/image.png (blank for \"?\")")
            .set_min_length(0)
            .set_max_length(120)
            .set_text_style(dpp::text_short)
    ).add_row().add_component(
        dpp::component()
            .set_label("Color (optional)")
            .set_id("color")
            .set_type(dpp::cot_text)
            .set_placeholder("(blank for user color) red, green, ..., black, default, 0xHEX or DECIMAL")
            .set_min_length(0)
            .set_max_length(20)
            .set_text_style(dpp::text_short)
    );
    ev.dialog(modal, error_autoprint);
    return true;
}

bool run_ping(const dpp::interaction_create_t& ev)
{
    dpp::message replying;
    replying.id = ev.command.id;
    replying.channel_id = ev.command.channel_id;
    replying.set_type(dpp::message_type::mt_application_command);
    replying.set_flags(64);
    replying.content = "Current REST ping is: `" + std::to_string(static_cast<int>(ev.from->creator->rest_ping * 1000.0)) + " ms`";

    ev.reply(dpp::interaction_response_type::ir_channel_message_with_source, replying);
    
    return true;
}

bool run_config_server(const dpp::interaction_create_t& ev, const dpp::command_interaction& cmd)
{
    if (!is_member_admin(ev.command.member)) {
        ev.reply(make_ephemeral_message("I think you're not an admin or cache is not up to date :("));
        return true;
    }

    dpp::message msg(ev.command.channel_id, "**Guild configuration menu**");
    msg.add_component(
        dpp::component()
            .add_component(
                dpp::component()
                .set_label("Select configuration to setup")
                .set_id("guild-generate_menu")
                .set_type(dpp::cot_selectmenu)
                .add_select_option(dpp::select_option("Export config",  "guildconf-export",         "Export guild configuration"))                 // create a message with json there lol
                .add_select_option(dpp::select_option("/paste",         "guildconf-paste",          "Global configurations about /paste"))         // Show button enable/disable external copy/paste
                .add_select_option(dpp::select_option("User points",    "guildconf-member_points",  "Select and manage a user's points"))          // Show buttons "select user", "set value/`$current_value`"
                .add_select_option(dpp::select_option("Role command",   "guildconf-roles_command",  "Manage user roles system"))                   // selectable list "guild-roles_command-list" + buttons: "new" (list), "replace" (list) and "trashcan" (select)
                .add_select_option(dpp::select_option("Autorole",       "guildconf-auto_roles",     "Manage roles given on user's first message")) // selectable list "guild-auto_roles" + buttons: "new" (list) and "trashcan"
                .add_select_option(dpp::select_option("Leveling",       "guildconf-leveling_roles", "Manage leveling settings"))                   // Show buttons for "messages? (true/false)" "where? (modal w/ chat name or id)"
            )
    );

    msg.set_flags(64);
    ev.reply(msg, error_autoprint);
    return true;
}

bool run_copy(const dpp::interaction_create_t& ev, const dpp::command_interaction& cmd)
{
    const auto you = tf_user_info[ev.command.usr.id];
    if (!you) {
        ev.reply(make_ephemeral_message("Something went wrong! You do not exist?! Please report error! I'm so sorry."));
        return true;
    }

    you->clipboard.message_id = cmd.target_id;
    you->clipboard.channel_id = ev.command.channel_id;
    you->clipboard.guild_id = ev.command.guild_id;

    ev.reply(make_ephemeral_message("Copied message to clipboard! Use /paste somewhere to paste a reference to it!"));
    return true;
}

bool run_paste(const dpp::interaction_create_t& ev, const dpp::command_interaction& cmd)
{
    const auto you = tf_user_info[ev.command.usr.id];
    if (!you) {
        ev.reply(make_ephemeral_message("Something went wrong! You do not exist?! Please report error! I'm so sorry."));
        return true;
    }
    const auto guil = tf_guild_info[ev.command.guild_id];
    if (!guil) {
        ev.reply(make_ephemeral_message("Something went wrong! Guild do not exist?! Please report error! I'm so sorry."));
        return true;
    }

    if (!you->clipboard.has_data()) {
        ev.reply(make_ephemeral_message("You haven't copy a message yet! Try right click on one and Apps -> Copy to clipboard!"));
        return true;
    }

    if (you->clipboard.guild_id != ev.command.guild_id && !guil->allow_external_paste)
    {
        ev.reply(make_ephemeral_message("This server doesn't allow paste from external sources."));
        return true;
    }

    dpp::interaction_modal_response modal("paste-comment", "Paste options");
    modal.add_component(
        dpp::component()
            .set_label("Comment? (optional)")
            .set_id("comment")
            .set_type(dpp::cot_text)
            .set_placeholder("KEKW look at that dude doing poggers")
            .set_min_length(0)
            .set_max_length(280)
            .set_text_style(dpp::text_paragraph)
    );
    ev.dialog(modal, error_autoprint);
    return true;
}

bool run_showinfo(const dpp::interaction_create_t& ev)
{
    dpp::message replying;
    replying.id = ev.command.id;
    replying.channel_id = ev.command.channel_id;
    replying.set_type(dpp::message_type::mt_application_command);
    replying.set_flags(64);

    if (!ev.command.resolved.users.size()) {
        ev.reply(make_ephemeral_message("Failed selecting user"));
        return true;        
    }

    dpp::user currusr = (*ev.command.resolved.users.begin()).second;
   
    const auto you = tf_user_info[currusr.id];
    if (!you) {
        ev.reply(make_ephemeral_message("Something went wrong! You do not exist?! Please report error! I'm so sorry."));
        return true;
    }

    dpp::embed localpt, globalpt, statistics;
    dpp::embed_author common_author{
            .name = currusr.format_username(),
            .url = currusr.get_avatar_url(256),
            .icon_url = currusr.get_avatar_url(256)
        };

    unsigned long long local_level = 0, local_nextlevel = 0;
    unsigned long long global_level = 0, global_nextlevel = 0;

    calc_user_level(you->points_per_guild[ev.command.guild_id], local_level, local_nextlevel);
    calc_user_level(you->points, global_level, global_nextlevel);

    localpt.set_author(common_author);
    localpt.set_title("**__Local points__**");
    localpt.set_thumbnail(images::points_image_url);
    localpt.set_color((you->pref_color < 0 ? random() : you->pref_color) % 0xFFFFFF);
    localpt.add_field("Current level", (u8"✨ " + std::to_string(local_level)), true );
    localpt.add_field("Points", (u8"🧬 " + std::to_string(you->points_per_guild[ev.command.guild_id])), true );
    localpt.add_field("Next level in", (u8"📈 " + std::to_string(local_nextlevel)), true );

    globalpt.set_author(common_author);
    globalpt.set_title("**__Global points__**");
    globalpt.set_thumbnail(images::points_image_url);
    globalpt.set_color((you->pref_color < 0 ? random() : you->pref_color) % 0xFFFFFF);
    globalpt.add_field("Current level", (u8"✨ " + std::to_string(global_level)), true );
    globalpt.add_field("Points", (u8"🧬 " + std::to_string(you->points)), true );
    globalpt.add_field("Next level in", (u8"📈 " + std::to_string(global_nextlevel)), true );

    statistics.set_author(common_author);
    statistics.set_title("**__Statistics__**");
    statistics.set_thumbnail(images::statistics_image_url);
    statistics.set_color((you->pref_color < 0 ? random() : you->pref_color) % 0xFFFFFF);
    statistics.add_field("Total messages", (u8"📚 " + std::to_string(you->messages_sent)), true);
    statistics.add_field("Messages here", (u8"📓 " + std::to_string(you->messages_sent_per_guild[ev.command.guild_id])), true);
    statistics.add_field("% messages here", (u8"🔖 " + std::to_string(static_cast<int>(((100 * you->messages_sent_per_guild[ev.command.guild_id])) / (you->messages_sent == 0 ? 1 : you->messages_sent))) + "%"), true);
    statistics.add_field("Total files", (u8"🗂️ " + std::to_string(you->attachments_sent)), true);
    statistics.add_field("Files here", (u8"📁 " + std::to_string(you->attachments_sent_per_guild[ev.command.guild_id])), true);
    statistics.add_field("% files here", (u8"⚙️ " + std::to_string(static_cast<int>((100 * you->attachments_sent_per_guild[ev.command.guild_id]) / (you->attachments_sent == 0 ? 1 : you->attachments_sent))) + "%"), true);
    statistics.add_field("Commands triggered", (u8"⚡ " + std::to_string(you->commands_used)), true);

    replying.embeds.push_back(localpt);
    replying.embeds.push_back(globalpt);
    replying.embeds.push_back(statistics);
    replying.set_content("");
    replying.set_flags(64);
    
    ev.reply(replying);
    return true;
}