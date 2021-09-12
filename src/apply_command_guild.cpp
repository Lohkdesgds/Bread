#include "apply_command_guild.hpp"

std::string language_based_on_location(const dpp::guild& guild)
{
    switch (guild.voice_region) {
    case dpp::region::r_brazil:
        return "pt.BR";
    default:
        return "en.US";
    }
}

void __apply_commands_at_guild(dpp::cluster& core, const mull guild, dpp::guild* opt_guild)
{
    if (guild == 0) return;
    auto gconf = get_guild_config(guild);

    const auto func_work = [&core, guild](dpp::guild& got, ComplexSharedPtr<GuildSelf> gconf)
    {
        std::string __report;
        auto commands_array = std::make_shared<_dummy_commands>();
        dpp::slashcommand& ping = commands_array->cmds[0].second; commands_array->cmds[0].first = "ping";
        dpp::slashcommand& ptts = commands_array->cmds[1].second; commands_array->cmds[1].first = "points";
        dpp::slashcommand& stat = commands_array->cmds[2].second; commands_array->cmds[2].first = "stats";
        dpp::slashcommand& post = commands_array->cmds[3].second; commands_array->cmds[3].first = "paste";
        dpp::slashcommand& self = commands_array->cmds[4].second; commands_array->cmds[4].first = "self";
        dpp::slashcommand& tags = commands_array->cmds[5].second; commands_array->cmds[5].first = "tags";
        dpp::slashcommand& copy = commands_array->cmds[6].second; commands_array->cmds[6].first = "copy";
        dpp::slashcommand& poll = commands_array->cmds[7].second; commands_array->cmds[7].first = "poll";
        dpp::slashcommand& bsts = commands_array->cmds[8].second; commands_array->cmds[8].first = "botstats";
        dpp::slashcommand& mngr = commands_array->cmds[9].second; commands_array->cmds[9].first = "manager"; // last one please! (so a new reapply can be done only after full reload!)
        // IF ADD NEW COMMANDS, UPDATE COMMANDS_AMOUNT!

        const auto lang = get_lang(gconf->get_language());

        const auto generic_f = [guild,gconf](const dpp::confirmation_callback_t& data) {
            if (data.is_error()) {
                gconf->post_log("Failed to apply commands -> ERR#" + std::to_string(data.http_info.status) + " BODY: " + data.http_info.body);

                Lunaris::cout << "[__apply_commands_at_guild] Failed to apply commands at guild #" << guild << "." ;
                Lunaris::cout << "[__apply_commands_at_guild] Response #" << data.http_info.status << ": " << data.http_info.body ;
            }
            return;
        };

        // CLEANING UP INVALID STUFF
        {
            // CLEANUP ADMIN ROLES NOT EXISTENT 
            for(const auto& i : gconf->get_roles_admin())
            {
                if (std::find(got.roles.begin(), got.roles.end(), i) == got.roles.end()) { // not a role anymore.
                    __report += "Removed now invalid admin role `" + std::to_string(i) + "`\n";
                    gconf->remove_role_admin(i);
                    continue;
                }
            }
            
            // CLEANUP ROLES COMMAND ROLES NOT EXISTENT
            for(auto j = gconf->get_roles_map().begin(); j != gconf->get_roles_map().end();)
            {
                for(auto k = j->list.begin(); k != j->list.end();) {
                    if (std::find(got.roles.begin(), got.roles.end(), k->id) == got.roles.end()){
                        __report += "Removed now invalid role from role command category `" + j->name + "` named `" + k->name + "` id `" + std::to_string(k->id) + "`\n";
                        k = j->list.erase(k);
                    }
                    else ++k;
                }

                if (j->list.empty()) {
                    __report += "Removed now empty category role named `" + j->name + "\n";
                    j = gconf->get_roles_map().erase(j);
                }
                else ++j;
            }

            // CLEANUP LEVELUP ROLES NOT EXISTENT
            for(auto j = gconf->get_roles_per_level_map().begin(); j != gconf->get_roles_per_level_map().end();)
            {
                if (std::find(got.roles.begin(), got.roles.end(), j->id) == got.roles.end()){
                    __report += "Removed now invalid role from level roles level `" + std::to_string(j->level) + "` id `" + std::to_string(j->id) + "`\n";
                    j = gconf->get_roles_per_level_map().erase(j);
                }
                else ++j;
            }
            gconf->sort_role_per_level();

            // CLEANUP AUTOROLE ROLES NOT EXISTENT
            for(auto j = gconf->get_roles_joined().begin(); j != gconf->get_roles_joined().end();)
            {
                if (std::find(got.roles.begin(), got.roles.end(), *j) == got.roles.end()){
                    __report += "Removed now invalid role from autorole id `" + std::to_string(*j) + "`\n";
                    j = gconf->get_roles_joined().erase(j);
                }
                else ++j;
            }
        }

        bool enable_tags = false; // only if set

        {
            self.set_name(lang->get(lang_command::SELFCONF));
            self.set_description(lang->get(lang_command::SELFCONF_DESC));
            self.set_application_id(core.me.id);
            
            poll.set_name(lang->get(lang_command::POLL)); // done
            poll.set_description(lang->get(lang_command::POLL_DESC));
            poll.set_application_id(core.me.id);

            ptts.set_name(lang->get(lang_command::POINTS)); // done
            ptts.set_description(lang->get(lang_command::POINTS_DESC));
            ptts.set_application_id(core.me.id);

            stat.set_name(lang->get(lang_command::STATS)); // done
            stat.set_description(lang->get(lang_command::STATS_DESC));
            stat.set_application_id(core.me.id);

            ping.set_name(lang->get(lang_command::PING)); // done
            ping.set_description(lang->get(lang_command::PING_DESC));
            ping.set_application_id(core.me.id);

            bsts.set_name(lang->get(lang_command::BOTSTATUS)); // done
            bsts.set_description(lang->get(lang_command::BOTSTATUS_DESC));
            bsts.set_application_id(core.me.id);

            copy.set_name(lang->get(lang_command::COPY)); // done
            copy.set_description(lang->get(lang_command::COPY_DESC));
            copy.set_type(dpp::ctxm_message);
            copy.set_application_id(core.me.id);

            post.set_name(lang->get(lang_command::PASTE)); // done
            post.set_description(lang->get(lang_command::PASTE_DESC));
            post.set_application_id(core.me.id);

            tags.set_name(lang->get(lang_command::ROLES));
            tags.set_description(lang->get(lang_command::ROLES_DESC));
            tags.set_application_id(core.me.id);
            tags.set_type(dpp::slashcommand_contextmenu_type::ctxm_chat_input);

            mngr.set_name(lang->get(lang_command::CONFIG));
            mngr.set_description(lang->get(lang_command::CONFIG_DESC));
            mngr.set_application_id(core.me.id);
            mngr.default_permission = false; // admin command only

            using opt = dpp::command_option;
            using opt_t = dpp::command_option_type;

            // MANAGE
            {
                // OWNER IS ALWAYS OWNER
                {
                    dpp::command_permission cmdperm;
                    cmdperm.id = got.owner_id;
                    cmdperm.permission = true;
                    cmdperm.type = dpp::command_permission_type::cpt_user;

                    mngr.add_permission(cmdperm);
                }

                // CLEANUP ADMIN ROLES NOT EXISTENT 
                for(const auto& i : gconf->get_roles_admin())
                {
                    dpp::command_permission cmdperm;
                    cmdperm.id = i;
                    cmdperm.permission = true;
                    cmdperm.type = dpp::command_permission_type::cpt_role;
                    mngr.add_permission(cmdperm);
                }

                // ----------------------------------------- //
                // >  MAIN COMMANDS
                // ----------------------------------------- //

                opt mngr_appl = opt(opt_t::co_sub_command,       lang->get(lang_command::CONFIG_APPLY),              lang->get(lang_command::CONFIG_APPLY_DESC));
                opt mngr_logs = opt(opt_t::co_sub_command,       lang->get(lang_command::CONFIG_LOGS),               lang->get(lang_command::CONFIG_LOGS_DESC));
                opt mngr_lang = opt(opt_t::co_sub_command,       lang->get(lang_command::CONFIG_LANGUAGE),           lang->get(lang_command::CONFIG_LANGUAGE_DESC));
                opt mngr_extn = opt(opt_t::co_sub_command_group, lang->get(lang_command::CONFIG_EXTERNAL),           lang->get(lang_command::CONFIG_EXTERNAL_DESC));
                opt mngr_admn = opt(opt_t::co_sub_command_group, lang->get(lang_command::CONFIG_ADMIN),              lang->get(lang_command::CONFIG_ADMIN_DESC));
                opt mngr_ptts = opt(opt_t::co_sub_command,       lang->get(lang_command::CONFIG_POINTS),             lang->get(lang_command::CONFIG_POINTS_DESC));
                opt mngr_role = opt(opt_t::co_sub_command_group, lang->get(lang_command::CONFIG_ROLES),              lang->get(lang_command::CONFIG_ROLES_DESC));
                opt mngr_arle = opt(opt_t::co_sub_command_group, lang->get(lang_command::CONFIG_AUTOROLE),           lang->get(lang_command::CONFIG_AUTOROLE_DESC));
                opt mngr_levl = opt(opt_t::co_sub_command_group, lang->get(lang_command::CONFIG_LEVELS),             lang->get(lang_command::CONFIG_LEVELS_DESC));



                // ----------------------------------------- //
                // >  MAIN SUBCOMMANDS
                // ----------------------------------------- //

                // <appl has no options>

                opt mngr_lang_strg = opt(opt_t::co_string, lang->get(lang_command::CONFIG_LANGUAGE_STRING), lang->get(lang_command::CONFIG_LANGUAGE_STRING_DESC), true);

                opt mngr_extn_past = opt(opt_t::co_sub_command, lang->get(lang_command::CONFIG_EXTERNAL_CANPASTE), lang->get(lang_command::CONFIG_EXTERNAL_CANPASTE_DESC));
                opt mngr_extn_past_bool = opt(opt_t::co_boolean, lang->get(lang_command::CONFIG_EXTERNAL_CANPASTE_ALLOW), lang->get(lang_command::CONFIG_EXTERNAL_CANPASTE_ALLOW_DESC));

                opt mngr_admn_addd = opt(opt_t::co_sub_command, lang->get(lang_command::CONFIG_ADMIN_ADD), lang->get(lang_command::CONFIG_ADMIN_ADD_DESC));
                opt mngr_admn_dell = opt(opt_t::co_sub_command, lang->get(lang_command::CONFIG_ADMIN_REMOVE), lang->get(lang_command::CONFIG_ADMIN_REMOVE_DESC));
                opt mngr_admn_chkk = opt(opt_t::co_sub_command, lang->get(lang_command::CONFIG_ADMIN_VERIFY), lang->get(lang_command::CONFIG_ADMIN_VERIFY_DESC));
                opt mngr_admn______role = opt(opt_t::co_role,   lang->get(lang_command::CONFIG_ADMIN_COMMON_ROLE), lang->get(lang_command::CONFIG_ADMIN_COMMON_ROLE_DESC), true);

                opt mngr_ptts_whoo = opt(opt_t::co_user,    lang->get(lang_command::CONFIG_POINTS_WHO), lang->get(lang_command::CONFIG_POINTS_WHO_DESC), true);
                opt mngr_ptts_vall = opt(opt_t::co_integer, lang->get(lang_command::CONFIG_POINTS_VALUE), lang->get(lang_command::CONFIG_POINTS_VALUE_DESC), true);

                opt mngr_role_addd = opt(opt_t::co_sub_command, lang->get(lang_command::CONFIG_ROLES_ADD), lang->get(lang_command::CONFIG_ROLES_ADD_DESC));
                opt mngr_role_dell = opt(opt_t::co_sub_command, lang->get(lang_command::CONFIG_ROLES_REMOVE), lang->get(lang_command::CONFIG_ROLES_REMOVE_DESC));
                opt mngr_role_clrr = opt(opt_t::co_sub_command, lang->get(lang_command::CONFIG_ROLES_CLEANUP), lang->get(lang_command::CONFIG_ROLES_CLEANUP_DESC));
                opt mngr_role_comb = opt(opt_t::co_sub_command, lang->get(lang_command::CONFIG_ROLES_COMBO), lang->get(lang_command::CONFIG_ROLES_COMBO_DESC));
                opt mngr_role_addd_catg = opt(opt_t::co_string, lang->get(lang_command::CONFIG_ROLES_ADD_CATEGORY), lang->get(lang_command::CONFIG_ROLES_ADD_CATEGORY_DESC), true);
                opt mngr_role_addd_role = opt(opt_t::co_role,   lang->get(lang_command::CONFIG_ROLES_ADD_ROLE), lang->get(lang_command::CONFIG_ROLES_ADD_ROLE_DESC), true);
                opt mngr_role_addd_name = opt(opt_t::co_string, lang->get(lang_command::CONFIG_ROLES_ADD_NAME), lang->get(lang_command::CONFIG_ROLES_ADD_NAME_DESC), true);
                opt mngr_role_dell_catg = opt(opt_t::co_string, lang->get(lang_command::CONFIG_ROLES_REMOVE_CATEGORY), lang->get(lang_command::CONFIG_ROLES_REMOVE_CATEGORY_DESC));
                opt mngr_role_dell_role = opt(opt_t::co_role,   lang->get(lang_command::CONFIG_ROLES_REMOVE_ROLE), lang->get(lang_command::CONFIG_ROLES_REMOVE_ROLE_DESC));
                opt mngr_role_comb_bool = opt(opt_t::co_boolean,lang->get(lang_command::CONFIG_ROLES_COMBO_COMBINABLE), lang->get(lang_command::CONFIG_ROLES_COMBO_COMBINABLE_DESC), true);
                opt mngr_role_comb_catg = opt(opt_t::co_string, lang->get(lang_command::CONFIG_ROLES_COMBO_CATEGORY), lang->get(lang_command::CONFIG_ROLES_COMBO_CATEGORY_DESC), true);
                
                opt mngr_arle_addd = opt(opt_t::co_sub_command, lang->get(lang_command::CONFIG_AUTOROLE_ADD), lang->get(lang_command::CONFIG_AUTOROLE_ADD_DESC));
                opt mngr_arle_dell = opt(opt_t::co_sub_command, lang->get(lang_command::CONFIG_AUTOROLE_REMOVE), lang->get(lang_command::CONFIG_AUTOROLE_REMOVE_DESC));
                opt mngr_arle_chkk = opt(opt_t::co_sub_command, lang->get(lang_command::CONFIG_AUTOROLE_VERIFY), lang->get(lang_command::CONFIG_AUTOROLE_VERIFY_DESC));
                opt mngr_arle______role = opt(opt_t::co_role,   lang->get(lang_command::CONFIG_AUTOROLE_COMMON_ROLE), lang->get(lang_command::CONFIG_AUTOROLE_COMMON_ROLE_DESC), true);

                opt mngr_levl_addd = opt(opt_t::co_sub_command, lang->get(lang_command::CONFIG_LEVELS_ADD), lang->get(lang_command::CONFIG_LEVELS_ADD_DESC));
                opt mngr_levl_dell = opt(opt_t::co_sub_command, lang->get(lang_command::CONFIG_LEVELS_REMOVE), lang->get(lang_command::CONFIG_LEVELS_REMOVE_DESC));
                opt mngr_levl_redr = opt(opt_t::co_sub_command, lang->get(lang_command::CONFIG_LEVELS_REDIRECT), lang->get(lang_command::CONFIG_LEVELS_REDIRECT_DESC));
                opt mngr_levl_msgs = opt(opt_t::co_sub_command, lang->get(lang_command::CONFIG_LEVELS_MESSAGES), lang->get(lang_command::CONFIG_LEVELS_MESSAGES_DESC));
                opt mngr_levl_addd_role = opt(opt_t::co_role,   lang->get(lang_command::CONFIG_LEVELS_ADD_ROLE), lang->get(lang_command::CONFIG_LEVELS_ADD_ROLE_DESC), true);
                opt mngr_levl_addd_levl = opt(opt_t::co_integer,lang->get(lang_command::CONFIG_LEVELS_ADD_LEVEL), lang->get(lang_command::CONFIG_LEVELS_ADD_LEVEL_DESC), true);
                opt mngr_levl_dell_role = opt(opt_t::co_role,   lang->get(lang_command::CONFIG_LEVELS_REMOVE_ROLE), lang->get(lang_command::CONFIG_LEVELS_REMOVE_ROLE_DESC));
                opt mngr_levl_dell_levl = opt(opt_t::co_integer,lang->get(lang_command::CONFIG_LEVELS_REMOVE_LEVEL), lang->get(lang_command::CONFIG_LEVELS_REMOVE_LEVEL_DESC));
                opt mngr_levl_redr_chid = opt(opt_t::co_channel,lang->get(lang_command::CONFIG_LEVELS_REDIRECT_CHANNELID), lang->get(lang_command::CONFIG_LEVELS_REDIRECT_CHANNELID_DESC));
                opt mngr_levl_msgs_blck = opt(opt_t::co_boolean,lang->get(lang_command::CONFIG_LEVELS_MESSAGES_BLOCK), lang->get(lang_command::CONFIG_LEVELS_MESSAGES_BLOCK_DESC), true);

                
                // ----------------------------------------- //
                // >  LINKING SUBSUBCOMMANDS TO SUBCOMMANDS
                // ----------------------------------------- //

                for(const auto& i : langctrl.language_list()) mngr_lang_strg.add_choice(dpp::command_option_choice(i, i)); // up to 25 afaik

                mngr_extn_past.add_option(mngr_extn_past_bool);

                mngr_admn_addd.add_option(mngr_admn______role);
                mngr_admn_dell.add_option(mngr_admn______role);
                mngr_admn_chkk.add_option(mngr_admn______role);

                mngr_role_addd.add_option(mngr_role_addd_catg);
                mngr_role_addd.add_option(mngr_role_addd_name);
                mngr_role_addd.add_option(mngr_role_addd_role);
                mngr_role_dell.add_option(mngr_role_dell_catg);
                mngr_role_dell.add_option(mngr_role_dell_role);
                mngr_role_comb.add_option(mngr_role_comb_bool);
                mngr_role_comb.add_option(mngr_role_comb_catg);
                
                mngr_arle_addd.add_option(mngr_arle______role);
                mngr_arle_dell.add_option(mngr_arle______role);
                mngr_arle_chkk.add_option(mngr_arle______role);

                mngr_levl_addd.add_option(mngr_levl_addd_role);
                mngr_levl_addd.add_option(mngr_levl_addd_levl);
                mngr_levl_dell.add_option(mngr_levl_dell_role);
                mngr_levl_dell.add_option(mngr_levl_dell_levl);
                mngr_levl_redr.add_option(mngr_levl_redr_chid);
                mngr_levl_msgs.add_option(mngr_levl_msgs_blck);


                // ----------------------------------------- //
                // >  LINKING SUBCOMMANDS TO MAIN COMMANDS
                // ----------------------------------------- //

                //<appl has no options>

                mngr_extn.add_option(mngr_extn_past);

                mngr_lang.add_option(mngr_lang_strg);

                mngr_admn.add_option(mngr_admn_addd);
                mngr_admn.add_option(mngr_admn_dell);
                mngr_admn.add_option(mngr_admn_chkk);

                mngr_ptts.add_option(mngr_ptts_whoo);
                mngr_ptts.add_option(mngr_ptts_vall);

                mngr_role.add_option(mngr_role_addd);
                mngr_role.add_option(mngr_role_dell);
                mngr_role.add_option(mngr_role_clrr);
                mngr_role.add_option(mngr_role_comb);

                mngr_arle.add_option(mngr_arle_addd);
                mngr_arle.add_option(mngr_arle_dell);
                mngr_arle.add_option(mngr_arle_chkk);

                mngr_levl.add_option(mngr_levl_addd);
                mngr_levl.add_option(mngr_levl_dell);
                mngr_levl.add_option(mngr_levl_redr);
                mngr_levl.add_option(mngr_levl_msgs);                



                // ----------------------------------------- //
                // >  FINAL LINK TO ROOT COMMAND
                // ----------------------------------------- //

                mngr.add_option(mngr_appl);
                mngr.add_option(mngr_logs);
                mngr.add_option(mngr_lang);
                mngr.add_option(mngr_extn);
                mngr.add_option(mngr_admn);
                mngr.add_option(mngr_ptts);
                mngr.add_option(mngr_role);
                mngr.add_option(mngr_arle);
                mngr.add_option(mngr_levl);
            }

            // EU
            {
                opt self_levl = opt(opt_t::co_sub_command,  lang->get(lang_command::SELFCONF_LEVELNOTIF), lang->get(lang_command::SELFCONF_LEVELNOTIF_DESC));
                opt self_levl_bool = opt(opt_t::co_boolean, lang->get(lang_command::SELFCONF_LEVELNOTIF_ENABLE), lang->get(lang_command::SELFCONF_LEVELNOTIF_ENABLE_DESC), true);

                opt self_colr = opt(opt_t::co_sub_command,  lang->get(lang_command::SELFCONF_COLOR), lang->get(lang_command::SELFCONF_COLOR_DESC));
                opt self_colr_vall = opt(opt_t::co_integer, lang->get(lang_command::SELFCONF_COLOR_VALUE), lang->get(lang_command::SELFCONF_COLOR_VALUE_DESC), true);

                self_levl.add_option(self_levl_bool);
                self_colr.add_option(self_colr_vall);

                self.add_option(self_levl);
                self.add_option(self_colr);
            }
            // POLL
            {
                opt poll_text = opt(opt_t::co_string,   lang->get(lang_command::POLL_TEXT),  lang->get(lang_command::POLL_TEXT_DESC), true);
                opt poll_titl = opt(opt_t::co_string,   lang->get(lang_command::POLL_TITLE),  lang->get(lang_command::POLL_TITLE_DESC));
                opt poll_mode = opt(opt_t::co_integer,  lang->get(lang_command::POLL_MODE),  lang->get(lang_command::POLL_MODE_DESC));
                opt poll_emoj = opt(opt_t::co_string,   lang->get(lang_command::POLL_EMOJIS),lang->get(lang_command::POLL_EMOJIS_DESC));
                opt poll_link = opt(opt_t::co_string,   lang->get(lang_command::POLL_LINK),  lang->get(lang_command::POLL_LINK_DESC));
                opt poll_colr = opt(opt_t::co_integer,  lang->get(lang_command::POLL_COLOR), lang->get(lang_command::POLL_COLOR_DESC));

                // MODES
                poll_mode.add_choice(dpp::command_option_choice(lang->get(lang_command::POLL_MODE_OPTION_0_DESC), static_cast<int32_t>(0)));
                poll_mode.add_choice(dpp::command_option_choice(lang->get(lang_command::POLL_MODE_OPTION_1_DESC), static_cast<int32_t>(1)));
                poll_mode.add_choice(dpp::command_option_choice(lang->get(lang_command::POLL_MODE_OPTION_2_DESC), static_cast<int32_t>(2)));
                poll_mode.add_choice(dpp::command_option_choice(lang->get(lang_command::POLL_MODE_OPTION_3_DESC), static_cast<int32_t>(3)));
                poll_mode.add_choice(dpp::command_option_choice(lang->get(lang_command::POLL_MODE_OPTION_4_DESC), static_cast<int32_t>(4)));

                poll.add_option(poll_text);
                poll.add_option(poll_titl);
                poll.add_option(poll_emoj);
                poll.add_option(poll_link);
                poll.add_option(poll_mode);
                poll.add_option(poll_colr);
            }
            // DNA
            {
                opt ptts_user = opt(opt_t::co_user,    lang->get(lang_command::POINTS_USER), lang->get(lang_command::POINTS_USER_DESC));
                opt ptts_glbl = opt(opt_t::co_boolean, lang->get(lang_command::POINTS_GLOBAL), lang->get(lang_command::POINTS_GLOBAL));

                ptts.add_option(ptts_user);
                ptts.add_option(ptts_glbl);
            }
            // STATS
            {
                opt stats_user = opt(opt_t::co_user, lang->get(lang_command::STATS_USER), lang->get(lang_command::STATS_USER_DESC), false);

                stat.add_option(stats_user);
            }
            // TAGS
            {
                if (gconf->get_roles_map().empty()) { // no roles, no command.
                    core.guild_commands_get(guild, [&core, guild, gconf, lang](const dpp::confirmation_callback_t& data){
                        if (data.is_error()) {
                            gconf->post_log("Failed to get commands -> ERR#" + std::to_string(data.http_info.status) + " BODY: " + data.http_info.body);

                            Lunaris::cout << "[__apply_commands_at_guild] Failed to clean roles command from guild #" << guild << "." ;
                            Lunaris::cout << "[__apply_commands_at_guild] Response #" << data.http_info.status << ": " << data.http_info.body ;
                            return;
                        }
                        dpp::slashcommand_map mapp = std::get<dpp::slashcommand_map>(data.value);

                        for(const auto& i : mapp) {
                            if (i.second.name == lang->get(lang_command::ROLES)){
                                //Lunaris::cout << "FOUND_DELETE_COMMAND_SUCCESS\n";
                                core.guild_command_delete(i.second.id, guild, [guild, gconf, nam = i.second.name](const dpp::confirmation_callback_t& data) {
                                    if (data.is_error()) {
                                        gconf->post_log("Failed to clear command '" + nam + "' -> ERR#" + std::to_string(data.http_info.status) + " BODY: " + data.http_info.body);
                                        Lunaris::cout << "[__apply_commands_at_guild] Failed to clean roles command from guild #" << guild << "." ;
                                        Lunaris::cout << "[__apply_commands_at_guild] Response #" << data.http_info.status << ": " << data.http_info.body ;
                                    }
                                });
                            }
                        }
                    });
                }
                else { // has roles to set up
                    bool has_one = false;

                    for (const auto& each : gconf->get_roles_map()) {
                        bool good = true;

                        const std::string cat_fixed = fix_name_for_cmd(each.name);

                        if (each.list.size() == 0 || cat_fixed.empty()) continue;
                        
                        dpp::command_option tags_each = dpp::command_option(dpp::command_option_type::co_sub_command, cat_fixed, lang->get(lang_command::ROLES_EACH_DESC));
                        dpp::command_option tags_each_list = dpp::command_option(dpp::command_option_type::co_string, lang->get(lang_command::ROLES_EACH), lang->get(lang_command::ROLES_EACH_DESC), true);

                        for(const auto& one : each.list) {                            
                            const std::string one_fixed = fix_name_for_cmd(one.name);

                            if (one_fixed.empty() || one.id == 0) {
                                good = false;
                                break;
                            }
                            tags_each_list.add_choice(dpp::command_option_choice(one_fixed, std::to_string(one.id)));
                        }
                        if (!good) continue;

                        tags_each.add_option(tags_each_list);
                        tags.add_option(tags_each);
                        has_one = true;
                    }

                    if (has_one) enable_tags = true;
                }
            }
            // PASTE
            {
                opt post_text = opt(opt_t::co_string, lang->get(lang_command::PASTE_TEXT), lang->get(lang_command::PASTE_TEXT_DESC));

                post.add_option(post_text);
            }
        }

        /* * * * * * * * > APPLYING COMMANDS < * * * * * * * */

        if (!__report.empty()) {
            if (__report.back() = '\n') __report.pop_back();
            gconf->post_log(__report);
        }

        gconf->set_config_locked(true);
        gconf->post_log("Guild config is now locked because slash commands are being updated.");

        core.guild_commands_get(guild, [&core, commands_array, gconf, guild, lang](const dpp::confirmation_callback_t& data) mutable {

            DelayedTasker& delay_tasker = get_default_tasker(); // global reference, no worries

            if(data.is_error()) {
                gconf->post_log("Failed to clear any command -> ERR#" + std::to_string(data.http_info.status) + " BODY: " + data.http_info.body);
                gconf->post_log("Failed to clear guild commands for new command setup. Setting up new commands anyway (so you have a way to setup your stuff if half broken).");
            }
            else {
                dpp::slashcommand_map mapp = std::get<dpp::slashcommand_map>(data.value);

                // remove
                for(const auto& i : mapp){
                    delay_tasker.push_back([&core, firsto = i.first, guild]() -> bool{
                        core.guild_command_delete(firsto, guild);
                        return true;
                    });
                }                
            }

            // then add
            delay_tasker.push_back([&core, commands_array, gconf, guild, lang](const size_t rn)
            {
                const auto handle = [&core, commands_array, gconf, guild, lang, rn](const dpp::confirmation_callback_t& data) {
                    if (data.is_error()){
                        gconf->post_log("Failed while applying new commands (at '" + commands_array->cmds[rn].first + "') -> ERR#" + std::to_string(data.http_info.status) + " BODY: " + data.http_info.body);

                        Lunaris::cout << "[__apply_commands_at_guild](recursive_set) Failed to apply command '" << commands_array->cmds[rn].first << "' at guild #" << guild << ".";
                        Lunaris::cout << "[__apply_commands_at_guild](recursive_set) Response #" << data.http_info.status << ": " << data.http_info.body;
                        return;
                    }
                    //Lunaris::cout << "[__apply_commands_at_guild] Applying commands... (" << (static_cast<int>(rn * 100.0f / commands_amount)) << "%) - Added '" << commands_array->cmds[rn].first << "'" ;

                    if ((rn + 1) >= commands_amount) {
                        //Lunaris::cout << "[__apply_commands_at_guild] Applied all commands." ;
                        return;
                    }
                };

                core.guild_command_create(commands_array->cmds[rn].second, guild, handle);
            }, 0, commands_amount);

            delay_tasker.push_back([gconf]{ gconf->set_config_locked(false); gconf->post_log("Guild config is now unlocked! Slash commands tasks ended!"); return true;}); // UNLOCK GUILD CONFIG AFTER ALL OTHER TASKS
        });

    };

    if (opt_guild != nullptr){
        func_work(*opt_guild, gconf);
    }
    else {
        core.guild_get(guild, [&core, guild, func_work, gconf](const dpp::confirmation_callback_t& data){
            if (data.is_error()) {
                gconf->post_log("Can't configure this guild -> ERR#" + std::to_string(data.http_info.status) + " BODY: " + data.http_info.body);
                Lunaris::cout << "Cannot configure guild #" << guild << "!" ;
                return;
            }
            dpp::guild got = std::get<dpp::guild>(data.value);            
            func_work(got, gconf);
        });
    }
}