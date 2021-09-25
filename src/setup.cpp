#include "setup.hpp"

void set_presence(dpp::cluster& core, ConfigSetting& conf)
{
    //dpp::activity act = dpp::activity(dpp::activity_type::at_streaming, u8"terças, quintas e sábados 20h30!", u8"BORA LÁ", "https://www.twitch.tv/Lohk");
    dpp::activity act = dpp::activity(static_cast<dpp::activity_type>(conf.qgi("status_mode")), conf.qgs("status_text"), "", conf.qgs("status_link"));    
	dpp::presence pres(dpp::presence_status::ps_dnd, act);
	core.set_presence(pres);
	Lunaris::cout << "[set_presence] Presence updated successfully." ;
}

void setup_bot(ConfigSetting& c, bool& skip)
{
    if (mkdir(guild_config_path_default.c_str(), 0777) && errno != EEXIST){ Lunaris::cout << "Cannot create directory (GUILD)!" ; exit(1); }
    if (mkdir(user_config_path_default.c_str(), 0777)  && errno != EEXIST){ Lunaris::cout << "Cannot create directory (USER)!" ; exit(1); }
    if (mkdir(language_config_path.c_str(), 0777)  && errno != EEXIST)    { Lunaris::cout << "Cannot create directory (LANG)!" ; exit(1); }

    if (!langctrl.try_reload()){
        Lunaris::cout << Lunaris::console::color::AQUA << "[SETUP]" << Lunaris::console::color::RED << " Please do something about the errors. I would NOT continue if there are errors.";
        while(1) std::this_thread::sleep_for(std::chrono::seconds(30));
    }

    // perfect 10 setup
    c.do_safe([](nlohmann::json& json){_fix_json_missing_vals(json);});
    c.flush(); // save

    // maybe in the future add shards and clusters? (more Raspberrys?)
    __global_cluster_sad_times = std::make_unique<dpp::cluster>(c.qgs("token"), c.qgi("intents"), c.qgi("shards"));

    __global_cluster_sad_times->on_ready                        ([d = __global_cluster_sad_times.get(),&c,&skip](const auto& ev){ if(!skip) try{_hook_ev_ready(*d, c, ev);                  }catch(const std::exception& e){Lunaris::cout << Lunaris::console::color::RED << "Unexpected error: " << e.what();}catch(...){Lunaris::cout << Lunaris::console::color::DARK_RED << "Unexpected error: UNCAUGHT";}}); // pointer should not change
    __global_cluster_sad_times->on_interaction_create           ([d = __global_cluster_sad_times.get(),&c,&skip](const auto& ev){ if(!skip) try{_hook_ev_interaction(*d, c, ev);            }catch(const std::exception& e){Lunaris::cout << Lunaris::console::color::RED << "Unexpected error: " << e.what();}catch(...){Lunaris::cout << Lunaris::console::color::DARK_RED << "Unexpected error: UNCAUGHT";}});
    __global_cluster_sad_times->on_message_create               ([d = __global_cluster_sad_times.get(),&c,&skip](const auto& ev){ if(!skip) try{_hook_ev_message_create(*d, c, ev);         }catch(const std::exception& e){Lunaris::cout << Lunaris::console::color::RED << "Unexpected error: " << e.what();}catch(...){Lunaris::cout << Lunaris::console::color::DARK_RED << "Unexpected error: UNCAUGHT";}});
    __global_cluster_sad_times->on_guild_create                 ([d = __global_cluster_sad_times.get(),&c,&skip](const auto& ev){ if(!skip) try{_hook_ev_guild_create(*d, c, ev);           }catch(const std::exception& e){Lunaris::cout << Lunaris::console::color::RED << "Unexpected error: " << e.what();}catch(...){Lunaris::cout << Lunaris::console::color::DARK_RED << "Unexpected error: UNCAUGHT";}});
    __global_cluster_sad_times->on_guild_delete                 ([d = __global_cluster_sad_times.get(),&c,&skip](const auto& ev){ if(!skip) try{_hook_ev_guild_delete(*d, c, ev);           }catch(const std::exception& e){Lunaris::cout << Lunaris::console::color::RED << "Unexpected error: " << e.what();}catch(...){Lunaris::cout << Lunaris::console::color::DARK_RED << "Unexpected error: UNCAUGHT";}});
    __global_cluster_sad_times->on_guild_member_update          ([d = __global_cluster_sad_times.get(),&c,&skip](const auto& ev){ if(!skip) try{_hook_ev_guild_member_update(*d, c, ev);    }catch(const std::exception& e){Lunaris::cout << Lunaris::console::color::RED << "Unexpected error: " << e.what();}catch(...){Lunaris::cout << Lunaris::console::color::DARK_RED << "Unexpected error: UNCAUGHT";}});
    __global_cluster_sad_times->on_message_reaction_add         ([d = __global_cluster_sad_times.get(),&c,&skip](const auto& ev){ if(!skip) try{_hook_ev_message_reaction_add(*d, c, ev);   }catch(const std::exception& e){Lunaris::cout << Lunaris::console::color::RED << "Unexpected error: " << e.what();}catch(...){Lunaris::cout << Lunaris::console::color::DARK_RED << "Unexpected error: UNCAUGHT";}});
    __global_cluster_sad_times->on_log                          ([](const auto& ev){if (ev.severity > dpp::ll_trace) { Lunaris::cout << Lunaris::console::color::DARK_PURPLE << "[DPP] " << ev.message; }});
}


void _hook_ev_ready(dpp::cluster& core, ConfigSetting& conf, const dpp::ready_t& ev)
{
    set_presence(core, conf);
    _fix_set_slash_commands(core, conf);
}

void _hook_ev_guild_member_update(dpp::cluster& core, ConfigSetting& conf, const dpp::guild_member_update_t& ev) 
{
    const auto lang = get_lang(get_guild_config(ev.updated.guild_id)->get_language());
    __handle_guild_member_update(core, ev, lang);
}

void _hook_ev_interaction(dpp::cluster& core, ConfigSetting& conf, const dpp::interaction_create_t& ev)
{
    const auto lang = get_lang(get_guild_config(ev.command.guild_id)->get_language());

    if (ev.command.type == dpp::interaction_type::it_ping) {
        ev.reply(dpp::interaction_response_type::ir_pong, "");
    }
    else if (ev.command.type == dpp::interaction_type::it_application_command) {
        dpp::command_interaction cmd = std::get<dpp::command_interaction>(ev.command.data);

        dpp::message replying;
        replying.id = ev.command.id;
        replying.channel_id = ev.command.channel_id;
        replying.set_type(dpp::message_type::mt_application_command);
        replying.set_flags(64);
        replying.nonce = std::to_string(ev.command.usr.id);

        try {
            // any
            __handle_command_statistics_any(core, ev, cmd);

            static const std::initializer_list<lang_command> commands_expected = { lang_command::FEEDBACK, lang_command::TIME, lang_command::RGB2DECIMAL, lang_command::POINTS, lang_command::COPY, lang_command::PASTE, lang_command::STATS, lang_command::POLL, lang_command::ROLES, lang_command::CONFIG, lang_command::SELFCONF, lang_command::PING, lang_command::BOTSTATUS };

            switch(lang->command_from(cmd.name, commands_expected)){
            case lang_command::POINTS:
                __handle_command_points(core, ev, cmd, lang);
                break;
            case lang_command::COPY:
                __handle_command_clipboard_copy(core, ev, cmd, lang);
                break;
            case lang_command::PASTE:
                __handle_command_clipboard_paste(core, ev, cmd, lang);
                break;
            case lang_command::STATS:
                __handle_command_statistics(core, ev, cmd, lang);
                break;
            case lang_command::POLL:
                __handle_command_poll(core, ev, cmd, lang);
                break;
            case lang_command::ROLES:
                __handle_command_tags(core, ev, cmd, lang);
                break;
            case lang_command::CONFIG:
                __handle_command_configurar(core, ev, cmd, lang);
                break;
            case lang_command::SELFCONF:
                __handle_command_eu(core, ev, cmd, lang);
                break;
            case lang_command::PING:
                __handle_command_ping(core, ev, cmd, lang);
                break;
            case lang_command::TIME:
                __handle_command_thetime(core, ev, cmd, lang);
                break;
            case lang_command::BOTSTATUS:
                __handle_command_botstatus(core, ev, cmd, lang);
                break;
            case lang_command::RGB2DECIMAL:
                __handle_command_rgb(core, ev, cmd, lang);
                break;
            case lang_command::FEEDBACK:
                __handle_command_feedback(core, ev, cmd, lang);
                break;
            default:
                replying.set_content("Ainda não sou capaz disso. Desculpe.");
                ev.edit_response(replying);
                break;
            }
        }
        catch(const std::exception& e) {
            replying.set_content(std::string("O bot parece ter dado algum problema. Por favor tente novamente daqui alguns minutos.\nInformações do erro:\n") + e.what());
            ev.edit_response(replying);
            Lunaris::cout << "[EXCEPTION] Error at INTERACTION: " << e.what() ;
        }
        catch(...) {
            replying.set_content(std::string("O bot parece ter dado algum problema. Por favor tente novamente daqui alguns minutos."));
            ev.edit_response(replying);
            Lunaris::cout << "[EXCEPTION] Error at INTERACTION: UNCAUGHT" ;
        }
    }
}

void _hook_ev_message_create(dpp::cluster& core, ConfigSetting& conf, const dpp::message_create_t& ev)
{
    const auto lang = get_lang(get_guild_config(ev.msg->guild_id)->get_language());
    if (ev.msg->author->is_bot()) return;
    __handle_message_points(core, ev, lang);
    __handle_message_statistics(core, ev);
    //__handle_message_ping(core, ev, lang);
}

void _hook_ev_guild_create(dpp::cluster& core, ConfigSetting& conf, const dpp::guild_create_t& ev)
{
    auto gconf = get_guild_config(ev.created->id);
    gconf->set_config_locked(false); // new guilds must not be locked
    gconf->set_guild_deleted(false);
    
    if (gconf->get_language().empty()){
        gconf->set_language(language_based_on_location(*ev.created));
        __apply_commands_at_guild(core, ev.created->id, ev.created);
        Lunaris::cout << "Joined new guild '" << ev.created->name << "' (#" << std::to_string(ev.created->id) << ")" ;
    }
    else if (gconf->get_current_command_version() < commands_version_val){
        gconf->set_current_command_version(commands_version_val);
        gconf->save(); // force this time
        __apply_commands_at_guild(core, ev.created->id, ev.created);
        Lunaris::cout << "Joined and updated guild '" << ev.created->name << "' (#" << std::to_string(ev.created->id) << ")" ;
    }
    else Lunaris::cout << "Connected to guild '" << ev.created->name << "' (#" << std::to_string(ev.created->id) << ")" ;    
}

void _hook_ev_guild_delete(dpp::cluster& core, ConfigSetting& conf, const dpp::guild_delete_t& ev)
{
    if (!ev.deleted->is_unavailable()){ // not "unavailable" means ban or kick
        Lunaris::cout << "Left and cleared guild '" << ev.deleted->name << "' (#" << std::to_string(ev.deleted->id) << ")" ;

        auto gconf = get_guild_config(ev.deleted->id);
        gconf->set_guild_deleted(true);

        //delete_guild_config(ev.deleted->id);
    }
}

void _hook_ev_message_reaction_add(dpp::cluster& core, ConfigSetting& conf, const dpp::message_reaction_add_t& ev)
{
    const auto lang = get_lang(get_guild_config(ev.reacting_guild->id)->get_language());
    __handle_reaction_poll(core, ev, lang); // this does delete/clear/erase message if command was from user and emoji match X
}

void _fix_json_missing_vals(nlohmann::json& json)
{
    //json["help"] = u8"INTENTS=Discord intents|CLEANUP=cleans up all slash commands (global slash commands)|CLEANUP_GUILD=same as CLEANUP, but local in a specific guild (!=0 activates)|STATUS_*: current bot status";

    json["__help_intents"] = "The bot's intents";
    json["__help_cleanup"] = "Cleanup clears all global commands (you have to re-set them to use again with setup_slashcmd). Automatically sets itself false after success.";
    json["__help_cleanup_guild"] = "Cleanup clears local guild commands (you may want to use the slash command to reset itself instead of clearing it like this). Automatically sets itself 0 after success.";
    json["__help_setup_slashcmd"] = "Re-set global commands (obsolete, because global commands are not used anymore). Automatically sets itself false after success.";
    json["__help_status_text"] = "Text shown in bot's presence";
    json["__help_status_link"] = "The link related to the activity (streaming link)";
    json["__help_status_mode"] = "The mode itself (gaming, streaming, other)";

    if (!json.contains("intents")       || !json["intents"].is_string())        json["intents"]         = default_app_intents;
    if (!json.contains("shards")        || !json["shards"].is_number())         json["shards"]          = default_shard_count;
    if (!json.contains("cleanup")       || !json["cleanup"].is_boolean())       json["cleanup"]         = false;
    if (!json.contains("cleanup_guild") || !json["cleanup_guild"].is_number())  json["cleanup_guild"]   = 0ULL;
    if (!json.contains("setup_slashcmd")|| !json["setup_slashcmd"].is_boolean())json["setup_slashcmd"]  = true;

    if (!json.contains("status_text")   || !json["status_text"].is_string())    json["status_text"]     = "BreadGlobal_beta V1.0";
    if (!json.contains("status_link")   || !json["status_link"].is_string())    json["status_link"]     = "https://www.twitch.tv/Lohk";
    if (!json.contains("status_mode")   || !json["status_mode"].is_number())    json["status_mode"]     = static_cast<int>(dpp::activity_type::at_streaming);
}

void _fix_set_slash_commands(dpp::cluster& core, ConfigSetting& c)
{
    bool exit_setting_slash = false;
    c.do_safe([&](nlohmann::json& j){
        exit_setting_slash = !j["setup_slashcmd"].get<bool>();
        j["setup_slashcmd"] = false; // once only

        if (j["cleanup_guild"] != 0ULL) {
            mull currguildid = j["cleanup_guild"];

            Lunaris::cout << "CLEANUP detected one guild (#" << currguildid << ") to clean up slash commands. Starting cleanup in 5 seconds..." ;

            std::this_thread::sleep_for(std::chrono::seconds(6));

            Lunaris::cout << "CLEANUP will clean up all local slash commands!" ;

            bool got_return = false;

            core.guild_commands_get(currguildid, [&](const dpp::confirmation_callback_t& data){
                if (!data.is_error()) {
                    dpp::slashcommand_map mapp = std::get<dpp::slashcommand_map>(data.value);
                    for(const auto& i : mapp){
                        Lunaris::cout << "Removing specific guild command #" << i.first << " called '" << i.second.name << "'" ;
                        core.guild_command_delete(i.first, currguildid);
                    }
                }
                got_return = true;
            });

            while(!got_return) std::this_thread::sleep_for(std::chrono::milliseconds(500));

            Lunaris::cout << "CLEANUP of one guild ended." ;

            j["cleanup_guild"] = 0LL;
        }
        if (j["cleanup"].get<bool>()) {
            Lunaris::cout << "CLEANUP was set to be done in config file. This is done ONCE and then reset to FALSE. You have 5 seconds to cancel if you wish." ;

            std::this_thread::sleep_for(std::chrono::seconds(6));

            Lunaris::cout << "CLEANUP will clean up all global slash commands!" ;

            bool got_return = false;

            core.global_commands_get([&](const dpp::confirmation_callback_t& data){
                if (!data.is_error()) {
                    dpp::slashcommand_map mapp = std::get<dpp::slashcommand_map>(data.value);
                    for(const auto& i : mapp){
                        Lunaris::cout << "Removing command #" << i.first << " called '" << i.second.name << "'" ;
                        core.global_command_delete(i.first);
                    }
                }
                got_return = true;
            });

            while(!got_return) std::this_thread::sleep_for(std::chrono::milliseconds(500));

            Lunaris::cout << "CLEANUP is ending its task." ;

            j["cleanup"] = false;
        }
    });
    c.flush();

    if (exit_setting_slash) {
        Lunaris::cout << "[ON_READY] Config file didn't force slash command reset, so assuming it's already good." ;
        return;
    }

    Lunaris::cout << "[ON_READY] GLOBAL COMMANDS ARE NOW DISABLED! CONFIGURATION FILE IS OBSOLETE! Skipping." ;
    return;
}