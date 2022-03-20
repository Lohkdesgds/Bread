#include <general_functions.hpp>

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
    const auto you = tf_user_info[ev.command.usr.id];
    if (!you) {
        ev.reply("Something went wrong! You do not exist?! Please report error! I'm so sorry.");
        return;
    }
    
    if (ev.custom_id == "user-pref_color") {
        try {
            const std::string val = std::get<std::string>(ev.components[0].components[0].value);
            const int64_t sel = interpret_color(val);
            
            auto_handle_button_switch(ev, ev.custom_id, [&](dpp::component& it){
                you->pref_color = sel;
                it.set_label("Profile color: " + (you->pref_color < 0 ? "DEFAULT" : print_hex(you->pref_color)));
            });
        }
        catch(...) {
            ev.reply("Sorry, something went wrong! I'm so sorry.");
        }
    }
}

void g_on_button_click(const dpp::button_click_t& ev)
{
    const auto you = tf_user_info[ev.command.usr.id];
    if (!you) {
        ev.reply("Something went wrong! You do not exist?! Please report error! I'm so sorry.");
        return;
    }

    if (ev.custom_id == "user-show_level_up_messages")
    {
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
                .set_placeholder("red, green, blue, yellow, magenta, cyan, white, black, default, 0xHEXAHERE or DECIMAL")
                .set_min_length(1)
                .set_max_length(20)
                .set_text_style(dpp::text_short)
        );
        ev.dialog(modal, error_autoprint);
    }
    else if (ev.custom_id == "user-download_user_data")
    {
        dpp::message msg;
        msg.set_content("Your user data:");
        msg.add_file("user_data.json", you->to_json().dump(2));
        msg.set_flags(64);
        ev.reply(msg, error_autoprint);
    }
    else {
        ev.reply(dpp::ir_update_message, "This interaction is unknown. Please try again later!", error_autoprint);
    }
}

void g_on_select(const dpp::select_click_t& ev)
{
    const auto you = tf_user_info[ev.command.usr.id];
    if (!you) {
        ev.reply("Something went wrong! You do not exist?! Please report error! I'm so sorry.");
        return;
    }    

    ev.reply("Unexpected command. How is this possible? Command id that failed: `" + ev.custom_id + "`.", error_autoprint);

}

void g_on_interaction(const dpp::interaction_create_t& ev)
{
    //ev.thinking();

    const auto you = tf_user_info[ev.command.usr.id];
    if (!you) {
        ev.reply("Something went wrong! You do not exist?! Please report error! I'm so sorry.");
        return;
    }

    dpp::message msg(ev.command.channel_id, "Your configuration");
    msg.add_component(
        dpp::component()
            .add_component(
                make_boolean_button(you->show_level_up_messages)
                .set_label("Level up messages?")
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
                .set_label("Download your data (DM)")
                .set_style(dpp::cos_danger)
                .set_id("user-download_user_data")
            )
    );

    ev.reply(msg, error_autoprint);
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
    }

}

void lock_indefinitely()
{
    while(1) std::this_thread::sleep_for(std::chrono::seconds(3600));
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
    return stream.str();
}