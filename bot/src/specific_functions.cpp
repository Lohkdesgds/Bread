#include <specific_functions.hpp>

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
            const std::string val = std::get<std::string>(ev.components[0].components[0].value);
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
    else if (ev.custom_id == "guildconf-member_points-select_userid")
    {
        try {
            const std::string val = std::get<std::string>(ev.components[0].components[0].value);
            unsigned long long literal = 0;
            const bool guuuuud = (val.find("-") == std::string::npos) && (sscanf(val.c_str(), "%llu", &literal) == 1);

            dpp::message cpy = ev.command.msg;
            dpp::snowflake target_user = 0;

            change_component(cpy.components, "guildconf-member_points-select_userid",  [&](dpp::component& it){
                if (guuuuud) {
                    std::string unam = "<name not found>";
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
            const std::string val = std::get<std::string>(ev.components[0].components[0].value);
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
            const std::string val = std::get<std::string>(ev.components[0].components[0].value);

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
            const std::string val = std::get<std::string>(ev.components[0].components[0].value);

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
            const std::string val = std::get<std::string>(ev.components[0].components[0].value);
            if (val.empty()) {
                ev.reply(make_ephemeral_message("Hmm your string was empty? Error."));
                return;
            }

            const auto guil = tf_guild_info[ev.command.guild_id];
            if (!guil) {
                ev.reply(make_ephemeral_message("Something went wrong! Guild do not exist?! Please report error! I'm so sorry."));
                return;
            }

            if (guil->roles_available.size() >= guild_props::max_role_groups) {
                ev.reply(make_ephemeral_message("Can't add more!."));
                return;
            }
            
            {
                auto it = std::find_if(guil->roles_available.begin(), guil->roles_available.end(), [&](const guild_info::category& c){ return c.name == val; });
                if (it != guil->roles_available.end()) {
                    ev.reply(make_ephemeral_message("This group already exists!"));
                    return;
                }

                guild_info::category cat;
                cat.name = val;
                guil->roles_available.push_back(cat);
            }

            dpp::message cpy = ev.command.msg;
            size_t offset = 0;

            change_component(cpy.components, "guildconf-roles_command-selected", [&](dpp::component& d){
                d.set_label(val);
                for (size_t ff = 0; ff < guil->roles_available.size(); ++ff) {
                    if (guil->roles_available[ff].name == d.label) {
                        offset = ff;
                        break;
                    }
                }
            });


            if (guil->roles_available.size() == 0) {
                cpy.content = "**Empty config**\nStart by creating a new role list!";
            }
            else {
                const auto& selectd = guil->roles_available[offset];

                cpy.content = "**Current list [" + 
                    std::to_string(selectd.list.size()) + "/" + std::to_string(guild_props::max_role_group_each) + " roles, " +
                    std::to_string(guil->roles_available.size()) + "/" + std::to_string(guild_props::max_role_groups) + " groups]:**\n```cs\n";

                cpy.content += "Group selected: " + selectd.name + "\n";

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

                cpy.content += "\n```\n**Select another group below, if you want to:**";
            }

            cpy.components.clear();
            if (guil->roles_available.size()) {
                dpp::component clist;
                clist.set_label("Configurations");
                clist.set_id("guildconf-roles_command-select");
                clist.set_type(dpp::cot_selectmenu);

                for(const auto& each : guil->roles_available) {
                    clist.add_select_option(dpp::select_option(each.name, each.name, "Select to manage it"));
                }

                cpy.add_component(
                    dpp::component().add_component(clist)
                );
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
                        .set_label(guil->roles_available.size() ? (guil->roles_available[offset].name) : "<none selected>")
                        .set_id("guildconf-roles_command-selected")
                        .set_disabled(true)
                    )
                    .add_component(dpp::component()
                        .set_type(dpp::cot_button)
                        .set_style(dpp::cos_primary)
                        .set_label("Add a role to the list")
                        .set_disabled(guil->roles_available.size() ? (guil->roles_available[offset].list.size() >= guild_props::max_role_group_each) : true)
                        .set_id("guildconf-roles_command-add")
                        .set_emoji("🆕")
                    )
                    .add_component(dpp::component()
                        .set_type(dpp::cot_button)
                        .set_style(dpp::cos_secondary)
                        .set_label("Remove a role from the list")
                        .set_disabled(guil->roles_available.size() ? (guil->roles_available[offset].list.size() == 0) : true)
                        .set_id("guildconf-roles_command-del")
                        .set_emoji("🗑️")
                    )
            );

            cpy.set_flags(64);
            ev.reply(dpp::ir_update_message, cpy, error_autoprint);

        }
        catch(...) {
            ev.reply(make_ephemeral_message("Sorry, something went wrong! I'm so sorry."));
        }
        return;
    }
    else if (ev.custom_id == "guildconf-roles_command-delgroup")
    {
        try {
            const std::string val = std::get<std::string>(ev.components[0].components[0].value);
            if (val.empty()) {
                ev.reply(make_ephemeral_message("Hmm your string was empty? Error."));
                return;
            }

            const auto guil = tf_guild_info[ev.command.guild_id];
            if (!guil) {
                ev.reply(make_ephemeral_message("Something went wrong! Guild do not exist?! Please report error! I'm so sorry."));
                return;
            }

            {
                auto it = std::find_if(guil->roles_available.begin(), guil->roles_available.end(), [&](const guild_info::category& c){ return c.name == val; });
                if (it == guil->roles_available.end()) {
                    ev.reply(make_ephemeral_message("Could not find one named like that."));
                    return;
                }

                guil->roles_available.erase(it);
            }

            dpp::message cpy = ev.command.msg;
            size_t offset = 0;

            change_component(cpy.components, "guildconf-roles_command-selected", [&](dpp::component& d){
                if (val == d.label) {
                    d.set_label(guil->roles_available.size() ? (guil->roles_available[0].name) : "<none selected>");
                }
                for (size_t ff = 0; ff < guil->roles_available.size(); ++ff) {
                    if (guil->roles_available[ff].name == d.label) {
                        offset = ff;
                        break;
                    }
                }
            });


            if (guil->roles_available.size() == 0) {
                cpy.content = "**Empty config**\nStart by creating a new role list!";
            }
            else {
                const auto& selectd = guil->roles_available[offset];

                cpy.content = "**Current list [" + 
                    std::to_string(selectd.list.size()) + "/" + std::to_string(guild_props::max_role_group_each) + " roles, " +
                    std::to_string(guil->roles_available.size()) + "/" + std::to_string(guild_props::max_role_groups) + " groups]:**\n```cs\n";

                cpy.content += "Group selected: " + selectd.name + "\n";

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

                cpy.content += "\n```\n**Select another group below, if you want to:**";
            }

            cpy.components.clear();
            if (guil->roles_available.size()) {
                dpp::component clist;
                clist.set_label("Configurations");
                clist.set_id("guildconf-roles_command-select");
                clist.set_type(dpp::cot_selectmenu);

                for(const auto& each : guil->roles_available) {
                    clist.add_select_option(dpp::select_option(each.name, each.name, "Select to manage it"));
                }

                cpy.add_component(
                    dpp::component().add_component(clist)
                );
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
                        .set_label(guil->roles_available.size() ? (guil->roles_available[offset].name) : "<none selected>")
                        .set_id("guildconf-roles_command-selected")
                        .set_disabled(true)
                    )
                    .add_component(dpp::component()
                        .set_type(dpp::cot_button)
                        .set_style(dpp::cos_primary)
                        .set_label("Add a role to the list")
                        .set_disabled(guil->roles_available.size() ? (guil->roles_available[offset].list.size() >= guild_props::max_role_group_each) : true)
                        .set_id("guildconf-roles_command-add")
                        .set_emoji("🆕")
                    )
                    .add_component(dpp::component()
                        .set_type(dpp::cot_button)
                        .set_style(dpp::cos_secondary)
                        .set_label("Remove a role from the list")
                        .set_disabled(guil->roles_available.size() ? (guil->roles_available[offset].list.size() == 0) : true)
                        .set_id("guildconf-roles_command-del")
                        .set_emoji("🗑️")
                    )
            );

            cpy.set_flags(64);
            ev.reply(dpp::ir_update_message, cpy, error_autoprint);
        }
        catch(...) {
            ev.reply(make_ephemeral_message("Sorry, something went wrong! I'm so sorry."));
        }
        return;
    }
    else if (ev.custom_id == "guildconf-roles_command-add")
    {
        try {
            const std::string val = std::get<std::string>(ev.components[0].components[0].value); // Role ID
            const std::string val2 = std::get<std::string>(ev.components[1].components[0].value); // Role naming

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

            dpp::message cpy = ev.command.msg;
            size_t offset = 0;

            change_component(cpy.components, "guildconf-roles_command-selected", [&](dpp::component& d){
                for (size_t ff = 0; ff < guil->roles_available.size(); ++ff) {
                    if (guil->roles_available[ff].name == d.label) {
                        offset = ff;
                        break;
                    }
                }
            });

            auto& thelst = guil->roles_available[offset];

            
            if (thelst.list.size() >= guild_props::max_role_group_each) {
                ev.reply(make_ephemeral_message("Can't add more!."));
                return;
            }

            auto it = std::find_if(thelst.list.begin(), thelst.list.end(), [&](const guild_info::pair_id_name& p) { return p.id == transl_val;});
            if (it == thelst.list.end()) thelst.list.push_back(guild_info::pair_id_name{ transl_val, val2 });
            else { it->name = val2; }
            

            if (guil->roles_available.size() == 0) {
                cpy.content = "**Empty config**\nStart by creating a new role list!";
            }
            else {
                const auto& selectd = guil->roles_available[offset];

                cpy.content = "**Current list [" + 
                    std::to_string(selectd.list.size()) + "/" + std::to_string(guild_props::max_role_group_each) + " roles, " +
                    std::to_string(guil->roles_available.size()) + "/" + std::to_string(guild_props::max_role_groups) + " groups]:**\n```cs\n";

                cpy.content += "Group selected: " + selectd.name + "\n";

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

                cpy.content += "\n```\n**Select another group below, if you want to:**";
            }

            cpy.components.clear();
            if (guil->roles_available.size()) {
                dpp::component clist;
                clist.set_label("Configurations");
                clist.set_id("guildconf-roles_command-select");
                clist.set_type(dpp::cot_selectmenu);

                for(const auto& each : guil->roles_available) {
                    clist.add_select_option(dpp::select_option(each.name, each.name, "Select to manage it"));
                }

                cpy.add_component(
                    dpp::component().add_component(clist)
                );
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
                        .set_label(guil->roles_available.size() ? (guil->roles_available[offset].name) : "<none selected>")
                        .set_id("guildconf-roles_command-selected")
                        .set_disabled(true)
                    )
                    .add_component(dpp::component()
                        .set_type(dpp::cot_button)
                        .set_style(dpp::cos_primary)
                        .set_label("Add a role to the list")
                        .set_disabled(guil->roles_available.size() ? (guil->roles_available[offset].list.size() >= guild_props::max_role_group_each) : true)
                        .set_id("guildconf-roles_command-add")
                        .set_emoji("🆕")
                    )
                    .add_component(dpp::component()
                        .set_type(dpp::cot_button)
                        .set_style(dpp::cos_secondary)
                        .set_label("Remove a role from the list")
                        .set_disabled(guil->roles_available.size() ? (guil->roles_available[offset].list.size() == 0) : true)
                        .set_id("guildconf-roles_command-del")
                        .set_emoji("🗑️")
                    )
            );

            cpy.set_flags(64);
            ev.reply(dpp::ir_update_message, cpy, error_autoprint);

        }
        catch(...) {
            ev.reply(make_ephemeral_message("Sorry, something went wrong! I'm so sorry."));
        }
        return;
    }
    else if (ev.custom_id == "guildconf-roles_command-del")
    {
        try {
            const std::string val = std::get<std::string>(ev.components[0].components[0].value); // Role ID

            const auto guil = tf_guild_info[ev.command.guild_id];
            if (!guil) {
                ev.reply(make_ephemeral_message("Something went wrong! Guild do not exist?! Please report error! I'm so sorry."));
                return;
            }

            if (guil->roles_available.size() == 0) {
                ev.reply(make_ephemeral_message("You have no groups yet! Please create one first!"));
                return;
            }

            const unsigned long long transl_val = val == "*" ? 0 : dpp::from_string<dpp::snowflake>(val);
            if (transl_val == 0 && val != "*") {
                ev.reply(make_ephemeral_message("Invalid input, my friend!"));
                return;
            }

            dpp::message cpy = ev.command.msg;
            size_t offset = 0;

            change_component(cpy.components, "guildconf-roles_command-selected", [&](dpp::component& d){
                for (size_t ff = 0; ff < guil->roles_available.size(); ++ff) {
                    if (guil->roles_available[ff].name == d.label) {
                        offset = ff;
                        break;
                    }
                }
            });

            auto& thelst = guil->roles_available[offset];


            if (val != "*") {
                auto it = std::find_if(thelst.list.begin(), thelst.list.end(), [&](const guild_info::pair_id_name& p) { return p.id == transl_val;});
                if (it != thelst.list.end()) thelst.list.erase(it);
            }
            else {
                thelst.list.clear();
            }


            if (guil->roles_available.size() == 0) {
                cpy.content = "**Empty config**\nStart by creating a new role list!";
            }
            else {
                const auto& selectd = guil->roles_available[offset];

                cpy.content = "**Current list [" + 
                    std::to_string(selectd.list.size()) + "/" + std::to_string(guild_props::max_role_group_each) + " roles, " +
                    std::to_string(guil->roles_available.size()) + "/" + std::to_string(guild_props::max_role_groups) + " groups]:**\n```cs\n";

                cpy.content += "Group selected: " + selectd.name + "\n";

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

                cpy.content += "\n```\n**Select another group below, if you want to:**";
            }

            cpy.components.clear();
            if (guil->roles_available.size()) {
                dpp::component clist;
                clist.set_label("Configurations");
                clist.set_id("guildconf-roles_command-select");
                clist.set_type(dpp::cot_selectmenu);

                for(const auto& each : guil->roles_available) {
                    clist.add_select_option(dpp::select_option(each.name, each.name, "Select to manage it"));
                }

                cpy.add_component(
                    dpp::component().add_component(clist)
                );
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
                        .set_label(guil->roles_available.size() ? (guil->roles_available[offset].name) : "<none selected>")
                        .set_id("guildconf-roles_command-selected")
                        .set_disabled(true)
                    )
                    .add_component(dpp::component()
                        .set_type(dpp::cot_button)
                        .set_style(dpp::cos_primary)
                        .set_label("Add a role to the list")
                        .set_disabled(guil->roles_available.size() ? (guil->roles_available[offset].list.size() >= guild_props::max_role_group_each) : true)
                        .set_id("guildconf-roles_command-add")
                        .set_emoji("🆕")
                    )
                    .add_component(dpp::component()
                        .set_type(dpp::cot_button)
                        .set_style(dpp::cos_secondary)
                        .set_label("Remove a role from the list")
                        .set_disabled(guil->roles_available.size() ? (guil->roles_available[offset].list.size() == 0) : true)
                        .set_id("guildconf-roles_command-del")
                        .set_emoji("🗑️")
                    )
            );

            cpy.set_flags(64);
            ev.reply(dpp::ir_update_message, cpy, error_autoprint);


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
                .set_id("string")
                .set_type(dpp::cot_text)
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
                .set_id("string")
                .set_type(dpp::cot_text)
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
                .set_id("string1")
                .set_type(dpp::cot_text)
                .set_placeholder("123456...")
                .set_min_length(1)
                .set_max_length(20)
                .set_text_style(dpp::text_short)
        )
        .add_row()
        .add_component(
            dpp::component()
                .set_label("Name to show")
                .set_id("string2")
                .set_type(dpp::cot_text)
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
                .set_id("string")
                .set_type(dpp::cot_text)
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
        else if (selected == "guildconf-roles_command"){ // selectable list "guild-roles_command-list" + buttons: "new" (list), "replace" (list) and "trashcan" (select)

            const auto guil = tf_guild_info[ev.command.guild_id];
            if (!guil) {
                ev.reply(make_ephemeral_message("Something went wrong! Guild do not exist?! Please report error! I'm so sorry."));
                return;
            }

            size_t offset = 0;

            dpp::message msg(ev.command.channel_id, "**Role commands**");

            if (guil->roles_available.size()) {
                dpp::component clist;
                clist.set_label("Configurations");
                clist.set_id("guildconf-roles_command-select");
                clist.set_type(dpp::cot_selectmenu);

                for(const auto& each : guil->roles_available) {
                    clist.add_select_option(dpp::select_option(each.name, each.name, "Select to manage it"));
                }

                msg.add_component(
                    dpp::component().add_component(clist)
                );
            }
            msg.add_component(
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
                        .set_label(guil->roles_available.size() ? (guil->roles_available[offset].name) : "<none selected>")
                        .set_id("guildconf-roles_command-selected")
                        .set_disabled(true)
                    )
                    .add_component(dpp::component()
                        .set_type(dpp::cot_button)
                        .set_style(dpp::cos_primary)
                        .set_label("Add a role to the list")
                        .set_disabled(guil->roles_available.size() ? (guil->roles_available[offset].list.size() >= guild_props::max_role_group_each) : true)
                        .set_id("guildconf-roles_command-add")
                        .set_emoji("🆕")
                    )
                    .add_component(dpp::component()
                        .set_type(dpp::cot_button)
                        .set_style(dpp::cos_secondary)
                        .set_label("Remove a role from the list")
                        .set_disabled(guil->roles_available.size() ? (guil->roles_available[offset].list.size() == 0) : true)
                        .set_id("guildconf-roles_command-del")
                        .set_emoji("🗑️")
                    )
            );

            if (guil->roles_available.size() == 0) {
                msg.content = "**Empty config**\nStart by creating a new role list!";
            }
            else {
                const auto& selectd = guil->roles_available[offset];

                msg.content = "**Current list [" + 
                    std::to_string(selectd.list.size()) + "/" + std::to_string(guild_props::max_role_group_each) + " roles, " +
                    std::to_string(guil->roles_available.size()) + "/" + std::to_string(guild_props::max_role_groups) + " groups]:**\n```cs\n";

                msg.content += "Group selected: " + selectd.name + "\n";

                if (selectd.list.size()){
                    dpp::cache<dpp::role>* cach = dpp::get_role_cache();
                    {
                        std::shared_lock<std::shared_mutex> lu(cach->get_mutex());
                        auto& rols = cach->get_container();

                        for(const auto& it : selectd.list) {
                            msg.content += " " + std::to_string(it.id) + " [" + it.name + "]: #";
                            auto found = std::find_if(rols.begin(), rols.end(), [&](const std::pair<dpp::snowflake, dpp::role*>& s){ return s.first == it.id;});
                            if (found != rols.end()) msg.content += found->second->name;
                            msg.content += "\n";
                        }
                    }
                }
                else {
                    msg.content += " <empty>";
                }

                msg.content += "\n```\n**Select another group below, if you want to:**";
            }

            msg.set_flags(64);
            ev.reply(dpp::ir_update_message, msg, error_autoprint);
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

        size_t offset = 0;
        for(size_t ff = 0; ff < guil->roles_available.size(); ++ff) {
            if (guil->roles_available[ff].name == name_sel) {
                offset = ff;
                break;
            }
        }

        dpp::component clist;
        clist.set_label("Configurations");
        clist.set_id("guildconf-roles_command-select");
        clist.set_type(dpp::cot_selectmenu);

        for(const auto& each : guil->roles_available) {
            clist.add_select_option(dpp::select_option(each.name, each.name, "Select to manage it"));
        }

        dpp::message msg(ev.command.channel_id, "**Role commands**");
        msg.add_component(
            dpp::component().add_component(clist)
        )
        .add_component(
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
                    .set_label(guil->roles_available.size() ? (guil->roles_available[offset].name) : "<none selected>")
                    .set_id("guildconf-roles_command-selected")
                    .set_disabled(true)
                )
                .add_component(dpp::component()
                    .set_type(dpp::cot_button)
                    .set_style(dpp::cos_primary)
                    .set_label("Add a role to the list")
                    .set_disabled(guil->roles_available.size() ? (guil->roles_available[offset].list.size() >= guild_props::max_role_group_each) : true)
                    .set_id("guildconf-roles_command-add")
                    .set_emoji("🆕")
                )
                .add_component(dpp::component()
                    .set_type(dpp::cot_button)
                    .set_style(dpp::cos_secondary)
                    .set_label("Remove a role from the list")
                    .set_disabled(guil->roles_available.size() ? (guil->roles_available[offset].list.size() == 0) : true)
                    .set_id("guildconf-roles_command-del")
                    .set_emoji("🗑️")
                )
        );

        if (guil->roles_available.size() == 0) {
                msg.content = "**Empty config**\nStart by creating a new role list!";
        }
        else {
            const auto& selectd = guil->roles_available[offset];

            msg.content = "**Current list [" + 
                std::to_string(selectd.list.size()) + "/" + std::to_string(guild_props::max_role_group_each) + " roles, " +
                std::to_string(guil->roles_available.size()) + "/" + std::to_string(guild_props::max_role_groups) + " groups]:**\n```cs\n";

            msg.content += "Group selected: " + selectd.name + "\n";

            if (selectd.list.size()){
                dpp::cache<dpp::role>* cach = dpp::get_role_cache();
                {
                    std::shared_lock<std::shared_mutex> lu(cach->get_mutex());
                    auto& rols = cach->get_container();

                    for(const auto& it : selectd.list) {
                        msg.content += " " + std::to_string(it.id) + " [" + it.name + "]: #";
                        auto found = std::find_if(rols.begin(), rols.end(), [&](const std::pair<dpp::snowflake, dpp::role*>& s){ return s.first == it.id;});
                        if (found != rols.end()) msg.content += found->second->name;
                        msg.content += "\n";
                    }
                }
            }
            else {
                msg.content += " <empty>";
            }

            msg.content += "\n```\n**Select another group below, if you want to:**";
        }

        msg.set_flags(64);
        ev.reply(dpp::ir_update_message, msg, error_autoprint);
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

    bool went_good = false;

    switch(slash_to_discord_command(cmd.name)) {
        case discord_slashcommands::BOTSTATUS:
            went_good = run_botstatus(ev, cmd);
            break;
        case discord_slashcommands::CONFIGURATION:
            went_good = run_config_server(ev, cmd);
            break;
        case discord_slashcommands::PASTE:
            break;
        case discord_slashcommands::PING:
            went_good = run_ping(ev);
            break;
        case discord_slashcommands::POLL:
            break;
        case discord_slashcommands::ROLES:
            break;
        case discord_slashcommands::SELF:
            went_good = run_self(ev);
            break;
        case discord_slashcommands::RC_SHOWINFO:
            break;
        case discord_slashcommands::RC_COPY:
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
    // color
    // link
    // emojis
    // title
    // text
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
                .add_select_option(dpp::select_option("Points",         "guildconf-member_points",  "Select and manage a user's points"))          // Show buttons "select user", "set value/`$current_value`"
                .add_select_option(dpp::select_option("Role command",   "guildconf-roles_command",  "Manage user roles system"))                   // selectable list "guild-roles_command-list" + buttons: "new" (list), "replace" (list) and "trashcan" (select)
                .add_select_option(dpp::select_option("Autorole",       "guildconf-auto_roles",     "Manage roles given on user's first message")) // selectable list "guild-auto_roles" + buttons: "new" (list) and "trashcan"
                .add_select_option(dpp::select_option("Leveling",       "guildconf-leveling_roles", "Manage leveling settings"))                   // Show buttons for "messages? (true/false)" "where? (modal w/ chat name or id)"
            )
    );

    msg.set_flags(64);
    ev.reply(msg, error_autoprint);
    return true;
}