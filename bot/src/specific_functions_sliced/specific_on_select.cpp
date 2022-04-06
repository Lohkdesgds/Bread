#include <specific_functions.hpp>
// SHOULD BE SAFE (MUTEXES)

// Detected all stuff readonly for guild!
void g_on_select(const dpp::select_click_t& ev)
{
    if (ev.command.usr.id != ev.command.msg.interaction.usr.id && ev.command.msg.interaction.usr.id != 0) {
        ev.reply(make_ephemeral_message("If you want to use this command, please trigger it yourself. This is not yours.")); 
        return;
    }
    
    transl_button_event wrk(ev);

    force_const<guild_info> guil = tf_guild_info[ev.command.guild_id];
    if (!guil) { ev.reply(make_ephemeral_message("Something went wrong! Guild do not exist?! Please report error! I'm so sorry.")); return; }

    std::shared_lock<std::shared_mutex> guilmtx(guil.unsafe().muu); // as guild is readonly

    if (!wrk.has_valid_ref()) { ev.reply(make_ephemeral_message("Something went wrong! Internal reference got LOST!")); return; }

    auto& trigg = wrk.get_trigger();

    if (trigg.group_name == "TMPgetrolegroup") { // switch role in list of getrole

        //std::shared_lock<Lunaris::shared_recursive_mutex> guilmtx(guil->mu);

        const std::string groupnam = (std::holds_alternative<std::string>(trigg.value) ? std::get<std::string>(trigg.value) : "");
        const dpp::snowflake roleid = trigg.item_name.empty() ? 0 : dpp::from_string<dpp::snowflake>(trigg.item_name);

        if (groupnam.empty() || roleid == 0) {
            ev.reply(make_ephemeral_message("Something went wrong! Item was empty or invalid. Track: on_button_click > TMPgetrolegroup > ?"));
            return;
        }

        auto it = std::find_if(guil->roles_available.begin(), guil->roles_available.end(), [&](const guild_info::category& c){ return c.name == groupnam; });

        if (it != guil->roles_available.end()) 
        {
            if (std::find_if(it->list.begin(), it->list.end(), [&](const guild_info::pair_id_name& p){return p.id == roleid; }) == it->list.end()) {
                ev.reply(make_ephemeral_message("Something went wrong! Item was not found in list. Track: on_button_click > TMPgetrolegroup > ?"));
                return;
            }

            dpp::guild_member member;
            member.guild_id = ev.command.guild_id;
            member.user_id 	= ev.command.usr.id;
            member.roles 	= ev.command.member.roles;            

            if (!it->can_combine) {
                for (const auto& inn : it->list) {
                    auto it2 = std::find(member.roles.begin(), member.roles.end(), inn.id);
                    if (it2 != member.roles.end() && *it2 != roleid) {
                        member.roles.erase(it2);
                    }
                }
            }
            
            auto it2 = std::find(member.roles.begin(), member.roles.end(), roleid);

            if (it2 == member.roles.end()) {
                member.roles.push_back(roleid);
            }
            else{
                member.roles.erase(it2);
            }

            select_row group;
            group.set_group_name("TMPgetrolegroup");

            for(const auto& i : it->list) {
                const bool has = (std::find(member.roles.begin(), member.roles.end(), i.id) != member.roles.end());
                group.push_item(item<std::string>(i.name, std::to_string(i.id), (has ? "Click to remove role" : "Click to add role")).set_emoji(has ? "✅" : "🔆").set_custom(it->name));
            }

            wrk.push_or_replace(group); // replace

            ev.from->creator->guild_edit_member(member, [ev](const dpp::confirmation_callback_t data) mutable {
                if (data.is_error()){
                    dpp::message msg = make_ephemeral_message("Something went wrong! Data can be wrong! Could not properly set role stuff! Track: on_button_click > TMPgetrolegroup > guild_member_edit > X");
                    msg.channel_id = ev.command.channel_id;
                    ev.from->creator->message_create(msg);
                }
            });

            wrk.reply(!guil->commands_public);
            return;
        }
        else {
            ev.reply(make_ephemeral_message("Something went wrong! Item unavailable. Track: on_button_click > TMPgetrolegroup > ?"));
            return;
        }

    }
    if (trigg.group_name == "getrolegroup") { // user selecting role group
        
        //std::shared_lock<Lunaris::shared_recursive_mutex> guilmtx(guil->mu);

        wrk.remove_group_named_all("TMP");
        
        auto it = std::find_if(guil->roles_available.begin(), guil->roles_available.end(), [&](const guild_info::category& c){ return c.name == trigg.item_name; });
        
        if (it != guil->roles_available.end()) 
        {
            wrk.set_content("**Selected**: `" + it->name + "` " + (std::string(it->can_combine ? "[combinable]" : "[one only]")));

            const auto& user_roles = ev.command.member.roles;

            select_row group;
            group.set_group_name("TMPgetrolegroup");

            for(const auto& i : it->list) {
                const bool has = (std::find(user_roles.begin(), user_roles.end(), i.id) != user_roles.end());
                group.push_item(item<std::string>(i.name, std::to_string(i.id), (has ? "Click to remove role" : "Click to add role")).set_emoji(has ? "✅" : "🔆").set_custom(it->name));
            }

            wrk.push_or_replace(group);
        }
        else {
            ev.reply(make_ephemeral_message("Something went wrong! Item unavailable. Track: on_button_click > getrolegroup > ?"));
            return;
        }

        wrk.reply(!guil->commands_public);
        return;
    }
    if (trigg.group_name == "TMProlesconfgroup") // part of guildconf, selecting roles omg
    {
        //std::shared_lock<Lunaris::shared_recursive_mutex> guilmtx(guil->mu);

        std::string selected_group_name = trigg.item_name;
        size_t selected_group = 0;

        auto it = std::find_if(guil->roles_available.begin(), guil->roles_available.end(), [&](const guild_info::category& c){ return c.name == selected_group_name; });
        
        if (it != guil->roles_available.end()) 
        {
            wrk.find_button_do("TMProlesconf", "selectedgroup", [&](item<button_props>& i){
                i.set_custom(selected_group_name);
            });
            selected_group = std::distance(guil->roles_available.begin(), it);
        }
        else if (guil->roles_available.size()) {
            selected_group = 0;
            selected_group_name = guil->roles_available[selected_group].name;
        }
        else {
            selected_group = 0;
            selected_group_name.clear();
        }

        wrk.set_content(
            std::string("**__Role command setup__**\n") +
            "*Current groups: " + std::to_string(guil->roles_available.size()) + " of " + std::to_string(guild_props::max_role_groups) + "*\n" +
            ((guil->roles_available.size() > 0) ? ("*Roles in this group: " + std::to_string(guil->roles_available[selected_group].list.size()) + " of " + std::to_string(guild_props::max_role_group_each) + "*\n") : "") +
            std::string((guil->roles_available.size() > selected_group) ? 
                guil->roles_available[selected_group].render_string_block() :
                "```cs\nCreate a role group to render a group view\n```"
            )
        );
        
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
    if (trigg.group_name == "guildconf") 
    {
        if (trigg.item_name == "export") // by default embeds/files won't stay
        {
            //std::shared_lock<Lunaris::shared_recursive_mutex> guilmtx(guil->mu);

            wrk.remove_group_named_all("TMP");

            wrk.set_content("**Export config selected**");
            wrk.reply(!guil->commands_public, true, [&](dpp::message& msg){ msg.add_file("guild_data.json", guil->to_json().dump(2)); });
            return;
        }
        if (trigg.item_name == "comm")
        {
            //std::shared_lock<Lunaris::shared_recursive_mutex> guilmtx(guil->mu);

            wrk.remove_group_named_all("TMP");

            wrk.push_or_replace(button_row()
                .push_item(item<button_props>("Allow external paste", "ext", { guil->allow_external_paste ? dpp::cos_success : dpp::cos_danger, false }).set_custom(guil->allow_external_paste))
                .push_item(item<button_props>("Make most commands public", "pub", { guil->commands_public ? dpp::cos_success : dpp::cos_danger, false }).set_custom(guil->commands_public))
                .push_item(item<button_props>("Show level up messages", "levelmsg", { !guil->block_levelup_user_event ? dpp::cos_success : dpp::cos_danger, false }).set_custom(guil->block_levelup_user_event))
                .push_item(item<button_props>("Level up chat: " + std::string(guil->fallback_levelup_message_channel != 0 ? std::to_string(guil->fallback_levelup_message_channel) : "NONE"), "levelch", { dpp::cos_secondary, false }))
                .set_group_name("TMPcommconf"), 0
            );

            wrk.set_content("**__User commands configuration__**");
            wrk.reply(!guil->commands_public);
            return;
        }
        if (trigg.item_name == "points")
        {
            //std::shared_lock<Lunaris::shared_recursive_mutex> guilmtx(guil->mu);

            wrk.remove_group_named_all("TMP");
            
            wrk.push_or_replace(button_row()
                .push_item(item<button_props>("Select user ID", "select", { dpp::cos_primary, false }).set_custom(dpp::snowflake(0)))
                .push_item(item<button_props>("Guild points: nan", "prtpts", { dpp::cos_secondary, true }))
                .push_item(item<button_props>("Set user points", "setpts", { dpp::cos_secondary, true }))
                .set_group_name("TMPpointsconf"), 0
            );

            wrk.set_content("**__Per user points manager__**");
            wrk.reply(!guil->commands_public);
            return;
        }
        if (trigg.item_name == "roles")
        {
            //std::shared_lock<Lunaris::shared_recursive_mutex> guilmtx(guil->mu);

            wrk.remove_group_named_all("TMP");

            std::string selected_group_name;
            if (guil->roles_available.size()) {
                selected_group_name = guil->roles_available[0].name;
            }
            const size_t selected_group = 0; // must be 0 here
            const bool can_addg = guil->roles_available.size() < guild_props::max_role_groups;
            const bool can_delg = guil->roles_available.size() > 0;
            const bool can_add = guil->roles_available.size() > selected_group ? (guil->roles_available[selected_group].list.size() < guild_props::max_role_group_each) : false;
            const bool can_del = guil->roles_available.size() > selected_group ? (guil->roles_available[selected_group].list.size() > 0) : false;
            
            wrk.push_or_replace(button_row()
                .push_item(item<button_props>("Add group", "addg", { can_addg ? dpp::cos_primary : dpp::cos_secondary, !can_addg }).set_emoji("🌟"))
                .push_item(item<button_props>("Remove group", "delg", { dpp::cos_danger, !can_delg }).set_emoji("❌"))
                .push_item(item<button_props>("", "selectedgroup", { dpp::cos_secondary, true }).set_emoji("#️⃣").set_custom(selected_group_name))
                .push_item(item<button_props>("Add role", "add", { can_add ? dpp::cos_primary : dpp::cos_secondary, !can_add }).set_emoji("🌟"))
                .push_item(item<button_props>("Remove role", "del", { dpp::cos_danger, !can_del }).set_emoji("❌"))
                .set_group_name("TMProlesconf"), 0
            );


            select_row group;
            group.set_group_name("TMProlesconfgroup");
            for(const auto& i : guil->roles_available) {
                group.push_item(item<std::string>(i.name, i.name, "Select to configure or check"));
            }
            wrk.push_or_replace(group, 0); // group of groups


            wrk.set_content(
                std::string("**__Role command setup__**\n") +
                "*Current groups: " + std::to_string(guil->roles_available.size()) + " of " + std::to_string(guild_props::max_role_groups) + "*\n" +
                ((guil->roles_available.size() > 0) ? ("*Roles in this group: " + std::to_string(guil->roles_available[selected_group].list.size()) + " of " + std::to_string(guild_props::max_role_group_each) + "*\n") : "") +
                std::string((guil->roles_available.size() > selected_group) ? 
                    guil->roles_available[selected_group].render_string_block() :
                    "```cs\nCreate a role group to render a group view\n```"
                )
            );

            wrk.reply(!guil->commands_public);
            return;            
        }
        if (trigg.item_name == "aroles") // auto roles
        {
            //std::shared_lock<Lunaris::shared_recursive_mutex> guilmtx(guil->mu);

            wrk.remove_group_named_all("TMP");

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
            
            wrk.push_or_replace(button_row()
                .push_item(item<button_props>("Add role", "add", { can_add ? dpp::cos_primary : dpp::cos_secondary, !can_add }).set_emoji("🌟"))
                .push_item(item<button_props>("Remove role", "del", { dpp::cos_danger, !can_del }).set_emoji("❌"))
                .set_group_name("TMParolesconf"), 0
            );

            wrk.reply(!guil->commands_public);
            return;
        }
        if (trigg.item_name == "lroles") // level roles
        {
            //std::shared_lock<Lunaris::shared_recursive_mutex> guilmtx(guil->mu);

            wrk.remove_group_named_all("TMP");

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

            wrk.push_or_replace(button_row()
                .push_item(item<button_props>("Add level role", "add", { can_add ? dpp::cos_primary : dpp::cos_secondary, !can_add }).set_emoji("🌟"))
                .push_item(item<button_props>("Remove level role", "del", { dpp::cos_danger, !can_del }).set_emoji("❌"))
                .set_group_name("TMPlrolesconf"), 0
            );

            wrk.reply(!guil->commands_public);
            return;
        }
    }
    if (trigg.group_name == "showinfo") {
        //std::shared_lock<Lunaris::shared_recursive_mutex> guilmtx(guil->mu);

        // ==================== SHARED STUFF GOING ON ==================== //
        dpp::snowflake usrid = 0;
        wrk.find_button_do("showinfotarget", "select", [&](item<button_props>& i){
            usrid = i.get_custom_as_snowflake();
        });

        if (usrid == 0) {
            ev.reply(make_ephemeral_message("Something went wrong! User not found?"));
            return;
        }

        dpp::user currusr;
        {
            dpp::cache<dpp::user>* cach = dpp::get_user_cache();
            std::shared_lock<std::shared_mutex> lu(cach->get_mutex());
            const auto& vec = cach->get_container();

            auto it = std::find_if(vec.begin(), vec.end(), [&](const std::pair<dpp::snowflake, dpp::user*>& u){ return u.first == usrid; });
            if (it == vec.end()) {
                ev.reply(make_ephemeral_message("Something went wrong! Cannot find user somehow. Cache is not up to date?"));
                return;
            }

            currusr = *it->second;
        }
    
        force_const<user_info> you = tf_user_info[currusr.id];
        if (!you) { ev.reply(make_ephemeral_message("Something went wrong! You do not exist?! Please report error! I'm so sorry.")); return; }

        std::shared_lock<std::shared_mutex> lu(you.unsafe().muu);

        dpp::embed targetemb;
        targetemb.set_author(dpp::embed_author{
                .name = currusr.format_username(),
                .url = currusr.get_avatar_url(256),
                .icon_url = currusr.get_avatar_url(256)
            });
        targetemb.set_color((you->pref_color < 0 ? random() : you->pref_color));
        targetemb.set_thumbnail(images::points_image_url);

        wrk.set_content("");

        // ==================== ENDOF SHARED STUFF GOING ON ==================== //

        if (trigg.item_name == "localpt") {
            unsigned long long local_level = 0, local_nextlevel = 0;
            calc_user_level(you->get_points_on_guild(ev.command.guild_id), local_level, local_nextlevel);

            targetemb.set_title("**__Local points__**");
            targetemb.add_field("Current level", (u8"✨ " + std::to_string(local_level)), true );
            targetemb.add_field("Points", (u8"🧬 " + std::to_string(you->get_points_on_guild(ev.command.guild_id))), true );
            targetemb.add_field("Next level in", (u8"📈 " + std::to_string(local_nextlevel)), true );

            wrk.reply(!guil->commands_public, true, [&](dpp::message& msg) {
                msg.embeds.push_back(targetemb);
            });
            return;
        }
        if (trigg.item_name == "globalpt") {            
            unsigned long long global_level = 0, global_nextlevel = 0;
            calc_user_level(you->points, global_level, global_nextlevel);

            targetemb.set_title("**__Global points__**");
            targetemb.add_field("Current level", (u8"✨ " + std::to_string(global_level)), true );
            targetemb.add_field("Points", (u8"🧬 " + std::to_string(you->points)), true );
            targetemb.add_field("Next level in", (u8"📈 " + std::to_string(global_nextlevel)), true );

            wrk.reply(!guil->commands_public, true, [&](dpp::message& msg) {
                msg.embeds.push_back(targetemb);
            });
            return;

        }
        if (trigg.item_name == "statistics") {
            targetemb.set_title("**__Statistics__**");
            targetemb.add_field("Total messages", (u8"📚 " + std::to_string(you->messages_sent)), true);
            targetemb.add_field("Messages here", (u8"📓 " + std::to_string(you->get_messages_on_guild(ev.command.guild_id))), true);
            targetemb.add_field("% messages here", (u8"🔖 " + std::to_string(static_cast<int>(((100 * you->get_messages_on_guild(ev.command.guild_id))) / (you->messages_sent == 0 ? 1 : you->messages_sent))) + "%"), true);
            targetemb.add_field("Total files", (u8"🗂️ " + std::to_string(you->attachments_sent)), true);
            targetemb.add_field("Files here", (u8"📁 " + std::to_string(you->get_attachments_on_guild(ev.command.guild_id))), true);
            targetemb.add_field("% files here", (u8"⚙️ " + std::to_string(static_cast<int>((100 * you->get_attachments_on_guild(ev.command.guild_id)) / (you->attachments_sent == 0 ? 1 : you->attachments_sent))) + "%"), true);
            targetemb.add_field("Commands triggered", (u8"⚡ " + std::to_string(you->commands_used)), true);

            wrk.reply(!guil->commands_public, true, [&](dpp::message& msg) {
                msg.embeds.push_back(targetemb);
            });
            return;
        }        

        ev.reply(make_ephemeral_message("Something went wrong! Track: on_select > showinfo > ?"));
        return;
    }

    ev.reply(make_ephemeral_message("Something went wrong! Track: on_select > ?"));
    return;
}