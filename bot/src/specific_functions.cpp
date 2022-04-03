#include <specific_functions.hpp>
// SHOULD BE SAFE (MUTEXES)

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
                if (std::find_if(msg.reactions.begin(), msg.reactions.end(), [&](const dpp::reaction& e){ return e.emoji_id == emoj.id && e.emoji_name == emoj.name && e.count < 2; }) != msg.reactions.end()){ // assuming 1 is bot, so 2 must be user + bot (sadly possibly exploitable)
                    boot->message_delete_reaction_emoji(msg, emoj.format());
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

void g_on_new_guild(dpp::cluster& core, const dpp::guild_create_t& ev)
{
    force_const<guild_info> guil = tf_guild_info[ev.created->id];
    std::lock_guard<std::shared_mutex> lg(guil.unsafe().muu);

    dpp::message msg;
    msg.guild_id = guild_backup_id;
    msg.channel_id = channel_logging_jl;
    
    if (guil->guild_was_deleted) {
        msg.content = "✅ Joined guild `" + ev.created->name + "`";
        core.message_create(msg);
    }
    else if (guil->guild_on_outage) {
        msg.content = "👍 Guild back from outage `" + ev.created->name + "`";
        core.message_create(msg);
    }    

    guil.unsafe().guild_was_deleted = false;
    guil.unsafe().guild_on_outage = false;
}

void g_on_left_guild(dpp::cluster& core, const dpp::guild_delete_t& ev)
{
    force_const<guild_info> guil = tf_guild_info[ev.deleted->id];
    std::lock_guard<std::shared_mutex> lg(guil.unsafe().muu);

    dpp::message msg;
    msg.guild_id = guild_backup_id;
    msg.channel_id = channel_logging_jl;

    if (!ev.deleted->is_unavailable()) { // "if unavailable is not set, the user was removed from the guild"
        msg.content = "❌ Left guild `" + ev.deleted->name + "`";
        core.message_create(msg);

        guil.unsafe().guild_was_deleted = true;
        tf_guild_info.free(ev.deleted->id);
    }
    else {
        guil.unsafe().guild_on_outage = true;

        msg.content = "🔻 Guild unavailable (Discord outage)`" + ev.deleted->name + "`";
        core.message_create(msg);
    }
}

void g_on_message(const dpp::message_create_t& ev)
{
    using mull = unsigned long long;

    if (ev.msg.author.is_bot()) return;
    if (ev.msg.author.flags & dpp::gm_pending) return;

    force_const<user_info> youu = tf_user_info[ev.msg.author.id];
    if (!youu) return; // discard
    std::unique_lock<std::shared_mutex> l2(youu.unsafe().muu);

    auto& you = youu.unsafe(); // it is safe.

    you.messages_sent++;
    you.messages_sent_per_guild[ev.msg.guild_id]++;

    you.attachments_sent += ev.msg.attachments.size();
    you.attachments_sent_per_guild[ev.msg.guild_id] += ev.msg.attachments.size();
    
    force_const<guild_info> guill = tf_guild_info[ev.msg.guild_id];
    if (!guill) return; // abort :(
    std::unique_lock<std::shared_mutex> l1(guill.unsafe().muu);

    auto& guil = guill.unsafe();

    if (get_time_ms() <= guil.last_user_earn_points) return;

    // patch user autorole on join if valid
    {
        const auto& vecref = guil.roles_when_join;

        if (vecref.size() > 0) {
            dpp::guild_member member;
            member.guild_id = ev.msg.guild_id;
            member.user_id 	= ev.msg.author.id;
            member.roles 	= ev.msg.member.roles;

            bool had_update = false;

            for (const auto& i : vecref) {
                if (std::find(member.roles.begin(), member.roles.end(), i) == member.roles.end()) {
                    member.roles.push_back(i);
                    had_update = true;
                }
            }

            if (had_update) {
                ev.from->creator->guild_edit_member(member, error_autoprint);
            }
        }
    }

    if (get_time_ms() <= you.last_points_earned) return;
    

    mull old_pts_g = you.points;
    mull old_pts_l = you.points_per_guild[ev.msg.guild_id];

    int boost_final = 0;
    const bool had_boost = (random() % leveling::range_boost_chances == 0);

    if (had_boost) boost_final = static_cast<int>(rand() % leveling::range_boost_total) + leveling::range_boost_low;
    else boost_final = static_cast<int>(rand() % leveling::range_total) + leveling::range_low;

    you.last_points_earned = get_time_ms() + leveling::time_to_earn_points_sameuser_ms;
    guil.last_user_earn_points = get_time_ms() + leveling::time_to_earn_points_sameuser_ms;

    if (boost_final == 0) return; // no changes

    if (boost_final < 0) {
        if (you.points_per_guild[ev.msg.guild_id] < (-boost_final)) you.points_per_guild[ev.msg.guild_id] = 0;
        else you.points_per_guild[ev.msg.guild_id] += boost_final;
        if (you.points < (-boost_final)) you.points = 0;
        else you.points += boost_final;
    }
    else {
        you.points_per_guild[ev.msg.guild_id] += boost_final;
        you.points += boost_final;
    }
    if (boost_final > 0) you.times_they_got_positive_points++;
    if (boost_final < 0) you.times_they_got_negative_points++;
        
    mull old_lvl_g = 1, old_lvl_l = 1;
    mull new_lvl_g = 1, new_lvl_l = 1;

    mull new_pts_g = you.points;
    mull new_pts_l = you.points_per_guild[ev.msg.guild_id];

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
        const auto& vecref = guil.role_per_level;

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
                ev.from->creator->guild_edit_member(member, error_autoprint);
            }
        }
    }

    if (!you.show_level_up_messages || guil.block_levelup_user_event) return;

    dpp::message replying;
    replying.set_type(dpp::message_type::mt_reply);

    if (const auto _temp = guil.fallback_levelup_message_channel; _temp != 0) { // else follow guild rules
        replying.channel_id = _temp;
    }
    else { // else if guild has no chat, the triggered chat it is.
        replying.channel_id = ev.msg.channel_id;
        replying.message_reference.channel_id = ev.msg.channel_id;
        replying.message_reference.message_id = ev.msg.id;
    }

    std::string desc = (was_level_up ? ("📈 **LEVEL UP**\n") : ("📉 **LEVEL DOWN**\n"));
    if (global_level_up || global_level_down) desc += (desc.length() ? "\n" : "") + (u8"✨ **GLOBAL:** " + std::string(was_level_up ? u8"🔺" : u8"🔻") + u8" `LEVEL " + std::to_string(new_lvl_g) + "`");
    if (local_level_up  || local_level_down)  desc += (desc.length() ? "\n" : "") + (u8"✨ **LOCAL:** "  + std::string(was_level_up ? u8"🔺" : u8"🔻") + u8" `LEVEL " + std::to_string(new_lvl_l) + "`");

    dpp::embed autoembed = dpp::embed()
        .set_author(
            dpp::embed_author{
                .name = ev.msg.author.format_username(),
                .url = ev.msg.author.get_avatar_url(256),
                .icon_url = ev.msg.author.get_avatar_url(256)
            })
        .set_description(desc)
        .set_color((you.pref_color < 0 ? random() : you.pref_color))
        .set_thumbnail(images::points_image_url);

    replying.embeds.push_back(autoembed);
    replying.set_content("");

    ev.from->creator->message_create(replying, error_autoprint);
}

void g_on_ready(const dpp::ready_t& ev, safe_data<slash_global>& sg)
{
    sg.safe<void>([&ev](slash_global& s){ s.update_bot_id(*ev.from->creator); });
}

void g_on_interaction(const dpp::interaction_create_t& ev)
{
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
    
    {
        force_const<user_info> you = tf_user_info[ev.command.usr.id];
        if (!you) {
            ev.reply(make_ephemeral_message("Something went wrong! You do not exist?! Please report error! I'm so sorry."));
            return;
        }
        std::lock_guard<std::shared_mutex> l(you.unsafe().muu);
        you.unsafe().commands_used++;
    }

    bool went_good = false;

    switch(slash_to_discord_command(cmd.name)) {
        case discord_slashcommands::BOTSTATUS:
            went_good = run_botstatus(ev, cmd); // all good
            break;
        case discord_slashcommands::CONFIGURATION:
            went_good = run_config_server(ev, cmd);
            break;
        case discord_slashcommands::PASTE:
            went_good = run_paste(ev, cmd); // all good
            break;
        case discord_slashcommands::PING:
            went_good = run_ping(ev); // all good
            break;
        case discord_slashcommands::POLL:
            went_good = run_poll(ev, cmd); // all good
            break;
        case discord_slashcommands::ROLES:
            went_good = run_roles(ev);
            break;
        case discord_slashcommands::SELF:
            went_good = run_self(ev); // all good
            break;
        case discord_slashcommands::USERINFO:
        case discord_slashcommands::RC_SHOWINFO:
            went_good = run_showinfo(ev); // all good
            break;
        case discord_slashcommands::RC_COPY:
            went_good = run_copy(ev, cmd); // all good
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

void input_handler_cmd(dpp::cluster& bot, bool& _keep, safe_data<general_config>& config, /*safe_data<std::vector<slash_local>>& lslashes,*/ const safe_data<slash_global>& gslash, const std::string& cmd)
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

        auto thebomb = std::shared_ptr<bomb>(new Lunaris::bomb([&bot]{
            cout << console::color::GREEN << "[MAIN] (Last task) All local guild commands will be available sometime soon.";
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
        cout << console::color::GREEN << "[MAIN] Data in memory (custom data):";
        cout << console::color::DARK_GREEN << "[MAIN] Users: " << tf_user_info.size();
        cout << console::color::DARK_GREEN << "[MAIN] Guilds: " << tf_guild_info.size();
        cout << console::color::DARK_GREEN << "[MAIN] Users (dpp): " << dpp::get_user_cache()->get_container().size();
        cout << console::color::DARK_GREEN << "[MAIN] Guilds (dpp): " << dpp::get_guild_cache()->get_container().size();
        cout << console::color::DARK_GREEN << "[MAIN] Roles (dpp): " << dpp::get_role_cache()->get_container().size();
        cout << console::color::DARK_GREEN << "[MAIN] Channels (dpp): " << dpp::get_channel_cache()->get_container().size();
        cout << console::color::DARK_GREEN << "[MAIN] Emojis (dpp): " << dpp::get_emoji_cache()->get_container().size();
        cout << console::color::DARK_GREEN << "[MAIN] Memory use [TOTAL, MB]: " << memuse_mb;
        cout << console::color::DARK_GREEN << "[MAIN] Memory use [RESIDENT, MB]: " << resident_mb;
        cout << console::color::DARK_GREEN << "[MAIN] Threads: " << num_threads;
        /*
user, fi
guild, f
role, fi
channel,
emoji, f
        */
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
    bot.on_ready([&sg](const dpp::ready_t& arg){ SUPERSAFE(g_on_ready(arg, sg)); });
    bot.on_form_submit([&](const dpp::form_submit_t& arg){ SUPERSAFE(g_on_modal(arg)); });
    bot.on_button_click([&](const dpp::button_click_t& arg) { SUPERSAFE(g_on_button_click(arg)); });
    bot.on_select_click([&](const dpp::select_click_t& arg) { SUPERSAFE(g_on_select(arg)); });
    bot.on_interaction_create([&](const dpp::interaction_create_t& arg) { SUPERSAFE(g_on_interaction(arg)); });
    bot.on_message_reaction_add([&](const dpp::message_reaction_add_t& arg) { SUPERSAFE(g_on_react(arg)); });
    bot.on_message_create([&](const dpp::message_create_t& arg) { SUPERSAFE(g_on_message(arg)); });
    bot.on_guild_create([&](const dpp::guild_create_t& arg){ SUPERSAFE(g_on_new_guild(bot, arg)); });
    bot.on_guild_delete([&](const dpp::guild_delete_t& arg){ SUPERSAFE(g_on_left_guild(bot, arg)); });
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
    try {
        if (str.find("0x") == 0) { // HEX
            if (str.length() <= 2) return -1;
            char* got_on = nullptr;
            return std::strtoll(str.c_str() + 2, &got_on, 16) & 0xFFFFFF;
        }
        else if (str[0] <= '9' && str[0] >= '0') { // DEC
            char* got_on = nullptr;
            return std::strtoll(str.c_str(), &got_on, 10) & 0xFFFFFF;
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
    }
    catch(const std::exception& e) { Lunaris::cout << Lunaris::console::color::DARK_RED << "Exception: " << __FILE__ << " @ " << __LINE__ << ": " << e.what(); }
    catch(...) { Lunaris::cout << Lunaris::console::color::DARK_RED << "Exception: " << __FILE__ << " @ " << __LINE__ << ": UNCAUGHT"; }
    return -1;
}

std::string print_hex_color_auto(const int64_t v)
{
    //std::stringstream stream;
    //stream << std::hex << v;
    //return "0x" + stream.str();
    if (v < 0) return "DEFAULT";
    char _tmp[16]{};
    snprintf(_tmp, 16, "0x%06llX", static_cast<unsigned long long>(v));
    return _tmp;
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
    if (str == "info") return discord_slashcommands::USERINFO;
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
    const auto guil = tf_guild_info[ev.command.guild_id];
    if (!guil) { ev.reply(make_ephemeral_message("Something went wrong! Guild do not exist?! Please report error! I'm so sorry.")); return true; }

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
        .set_color(random())
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
        
    dpp::message msg;
    if (!guil->commands_public) msg.set_flags(dpp::m_ephemeral);
    msg.embeds.push_back(autoembed);

    ev.reply(msg);

    return true;
}

bool run_self(const dpp::interaction_create_t& ev)
{
    force_const<user_info> you = tf_user_info[ev.command.usr.id];
    if (!you) { ev.reply(make_ephemeral_message("Something went wrong! You do not exist?! Please report error! I'm so sorry.")); return true; }
    force_const<guild_info> guil = tf_guild_info[ev.command.guild_id];
    if (!guil) { ev.reply(make_ephemeral_message("Something went wrong! Guild do not exist?! Please report error! I'm so sorry.")); return true; }

    std::shared_lock<std::shared_mutex> lg(guil.unsafe().muu);
    std::shared_lock<std::shared_mutex> lu(you.unsafe().muu);

    transl_button_event wrk;

    wrk.push_or_replace(button_row()
        .push_item(item<button_props>("Show level up messages to everyone", "ptspublic", { you->show_level_up_messages ? dpp::cos_success : dpp::cos_danger, false }))
        .push_item(item<button_props>("Profile color: " + print_hex_color_auto(you->pref_color), "setcolor", { dpp::cos_secondary, false }))
        .push_item(item<button_props>("Get your user data", "getdata", { dpp::cos_primary, false }))
        .set_group_name("selfconf")
    );

    dpp::message msg = wrk.generate_message(!guil->commands_public);
    msg.channel_id = ev.command.channel_id;
    msg.set_content("**__User configuration menu__**");

    ev.reply(msg, error_autoprint);
    return true;
}

bool run_poll(const dpp::interaction_create_t& ev, const dpp::command_interaction& cmd)
{
    auto modal = modal_generate("poll", "create", "Create a poll");

    modal_add_component(modal, "Title of the poll", "title", "Best food worldwide?", dpp::text_style_type::text_short, true, 1, 60);
    modal_add_component(modal, "Describe your question", "desc", "Bananas are way cooler than tomatoes. Aren't they? Sure they are. Do you agree?", dpp::text_style_type::text_paragraph, false, 0, 2000);
    modal_add_component(modal, "Emojis to add as reaction", "emojis", "👍;👎;... (blank results 👍;👎)", dpp::text_style_type::text_short, false, 0, 100);
    modal_add_component(modal, "Image link", "imglink", "https://myimage.notarealurl/image.png (blank for \"?\")", dpp::text_style_type::text_short, false, 0, 150);
    modal_add_component(modal, "Color", "color", "(blank for user color) red, green, ..., black, 0xHEX or DECIMAL", dpp::text_style_type::text_short, false, 0, 20);

    ev.dialog(modal, error_autoprint);
    return true;
}

bool run_ping(const dpp::interaction_create_t& ev)
{
    force_const<guild_info> guil = tf_guild_info[ev.command.guild_id];
    if (!guil) { ev.reply(make_ephemeral_message("Something went wrong! Guild do not exist?! Please report error! I'm so sorry.")); return true; }

    std::shared_lock<std::shared_mutex> lg(guil.unsafe().muu);

    dpp::message msg;
    if (!guil->commands_public) msg.set_flags(dpp::m_ephemeral);
    msg.set_content("Current REST ping is: `" + std::to_string(static_cast<int>(ev.from->creator->rest_ping * 1000.0)) + " ms`");

    ev.reply(dpp::interaction_response_type::ir_channel_message_with_source, msg);
    
    return true;
}

bool run_config_server(const dpp::interaction_create_t& ev, const dpp::command_interaction& cmd)
{
    if (!is_member_admin(ev.command.member)) { ev.reply(make_ephemeral_message("I think you're not an admin or cache is not up to date :(")); return true; }

    force_const<guild_info> guil = tf_guild_info[ev.command.guild_id];
    if (!guil) { ev.reply(make_ephemeral_message("Something went wrong! Guild do not exist?! Please report error! I'm so sorry.")); return true; }

    std::shared_lock<std::shared_mutex> lg(guil.unsafe().muu);
    
    transl_button_event wrk;

    wrk.push_or_replace(select_row()
        .push_item(item<std::string>("Export config",  "export",    "Export guild configuration"                ).set_emoji("📦"))
        .push_item(item<std::string>("Commands",       "comm",      "Configurations of commands in this guild"  ).set_emoji("⚙️"))
        .push_item(item<std::string>("User points",    "points",    "Select and manage a user's points"         ).set_emoji("💵"))
        .push_item(item<std::string>("Role command",   "roles",     "Manage user /roles command system"         ).set_emoji("🙂"))
        .push_item(item<std::string>("Autorole",       "aroles",    "Manage roles given on user's first message").set_emoji("📨"))
        .push_item(item<std::string>("Leveling",       "lroles",    "Manage leveling settings"                  ).set_emoji("📊"))
        .set_group_name("guildconf")
    );

    dpp::message msg = wrk.generate_message(!guil->commands_public);
    msg.channel_id = ev.command.channel_id;
    msg.set_content("**__Guild configuration menu__**");

    ev.reply(msg, error_autoprint);
    return true;
}

bool run_copy(const dpp::interaction_create_t& ev, const dpp::command_interaction& cmd)
{
    force_const<user_info> you = tf_user_info[ev.command.usr.id];
    if (!you) { ev.reply(make_ephemeral_message("Something went wrong! You do not exist?! Please report error! I'm so sorry.")); return true; }

    {
        std::lock_guard<std::shared_mutex> l(you.unsafe().muu);
        you.unsafe().clipboard.message_id = cmd.target_id;
        you.unsafe().clipboard.channel_id = ev.command.channel_id;
        you.unsafe().clipboard.guild_id = ev.command.guild_id;
    }

    ev.reply(make_ephemeral_message("Copied message reference to clipboard! Use /paste somewhere to paste a reference to it! (bot must be able to see it to reference it!)"));
    return true;
}

bool run_paste(const dpp::interaction_create_t& ev, const dpp::command_interaction& cmd)
{
    force_const<user_info> you = tf_user_info[ev.command.usr.id];
    if (!you) { ev.reply(make_ephemeral_message("Something went wrong! You do not exist?! Please report error! I'm so sorry.")); return true; }
    force_const<guild_info> guil = tf_guild_info[ev.command.guild_id];
    if (!guil) { ev.reply(make_ephemeral_message("Something went wrong! Guild do not exist?! Please report error! I'm so sorry.")); return true; }

    std::shared_lock<std::shared_mutex> lg(guil.unsafe().muu);
    std::shared_lock<std::shared_mutex> lu(you.unsafe().muu);

    if (!you->clipboard.has_data()) { ev.reply(make_ephemeral_message("You haven't copy a message yet! Try right click on one and Apps -> Copy to clipboard!")); return true; }

    if (you->clipboard.guild_id != ev.command.guild_id && !guil->allow_external_paste) {
        ev.reply(make_ephemeral_message("This server doesn't allow paste from external sources."));
        return true;
    }

    auto modal = modal_generate("paste", "post", "Paste options");
    modal_add_component(modal, "Your take on this message", "comment", "KEKW look at that dude doing poggers", dpp::text_style_type::text_paragraph, false, 0, 280);
    ev.dialog(modal, error_autoprint);
    return true;
}

bool run_showinfo(const dpp::interaction_create_t& ev)
{
    dpp::snowflake usrid = ev.command.usr.id;
    bool was_right_click = false;

    if (std::holds_alternative<dpp::command_interaction>(ev.command.data))
    {
        const dpp::command_interaction& inter = std::get<dpp::command_interaction>(ev.command.data);
        if (inter.target_id != 0) {
            usrid = inter.target_id;
            was_right_click = true;
        }
    }

    dpp::user currusr;
    bool replied_already = false;

    const auto do_showinfo_intern = [ev](const dpp::interaction_create_t& mev, dpp::user musr, bool replied_al){
        force_const<guild_info> guil = tf_guild_info[ev.command.guild_id];
        if (!guil) { ev.reply(make_ephemeral_message("Something went wrong! Guild do not exist?! Please report error! I'm so sorry.")); return; }
        
        std::shared_lock<std::shared_mutex> lg(guil.unsafe().muu);

        transl_button_event wrk;

        wrk.push_or_replace(select_row()
            .push_item(item<std::string>("Local points",   "localpt",     "The user points in this guild"                              ).set_emoji("📊"))
            .push_item(item<std::string>("Global points",  "globalpt",    "The global points (ranking)"                                ).set_emoji("🌐"))
            .push_item(item<std::string>("Statistics",     "statistics",  "User stats, like messages sent, commands, attachments..."   ).set_emoji("🗂️"))
            .set_group_name("showinfo")
        );

        wrk.push_or_replace(button_row()
            .push_item(item<button_props>("You've selected: " + musr.format_username(), "select", { dpp::cos_secondary, true }).set_custom(musr.id))
            .set_group_name("showinfotarget")
        );

        dpp::message msg = wrk.generate_message(!guil->commands_public);
        msg.channel_id = mev.command.channel_id;
        msg.set_content("**__About user__**");

        if (replied_al) mev.edit_response(msg);
        else mev.reply(msg);
    };

    {
        dpp::cache<dpp::user>* cach = dpp::get_user_cache();
        std::shared_lock<std::shared_mutex> lu(cach->get_mutex());
        const auto& vec = cach->get_container();

        auto it = std::find_if(vec.begin(), vec.end(), [&](const std::pair<dpp::snowflake, dpp::user*>& u){ return u.first == usrid; });
        if (it == vec.end()) {
            force_const<guild_info> guil = tf_guild_info[ev.command.guild_id];
            if (!guil) {ev.reply(make_ephemeral_message("Something went wrong! Guild do not exist?! Please report error! I'm so sorry.")); return true;}

            std::shared_lock<std::shared_mutex> lg(guil.unsafe().muu);

            ev.reply(dpp::message().set_content("This user is not on cache. Trying to load it... (please wait some seconds, this message will update soon, hopefully)").set_flags(guil->commands_public ? 0 : dpp::m_ephemeral));
            if (was_right_click) {
                ev.from->creator->user_get(usrid, [ev, do_showinfo_intern](const dpp::confirmation_callback_t& conf){
                    if (conf.is_error()) {
                        ev.reply("Sadly I couldn't get more information about this user. Try again later.");
                        return;
                    }

                    const dpp::user_identified& ui = std::get<dpp::user_identified>(conf.value);

                    // manually add to cache...?
                    if (ui.id != 0 && !dpp::get_user_cache()->find(ui.id)) {                        
                        dpp::get_user_cache()->store(new dpp::user(ui));
                    }

                    do_showinfo_intern(ev, ui, true);
                });
            }
            return true;
        }

        currusr = *it->second;
    }

    do_showinfo_intern(ev, currusr, false);
    return true;
}

bool run_roles(const dpp::interaction_create_t& ev)
{
    force_const<guild_info> guil = tf_guild_info[ev.command.guild_id];
    if (!guil) { ev.reply(make_ephemeral_message("Something went wrong! Guild do not exist?! Please report error! I'm so sorry.")); return true; }

    std::shared_lock<std::shared_mutex> lg(guil.unsafe().muu);
    
    const auto& available_roles = guil->roles_available;

    if (available_roles.empty()) {
        ev.reply(make_ephemeral_message("This guild has no role groups."));
        return true;
    }

    transl_button_event wrk;

    select_row group;
    group.set_group_name("getrolegroup");

    for(const auto& i : available_roles) {
        group.push_item(item<std::string>(i.name, i.name, "Group of roles " + std::string(i.can_combine ? "[combinable]" : "[one only]")));
    }

    wrk.push_or_replace(group); // group of groups

    dpp::message msg = wrk.generate_message(!guil->commands_public);
    msg.channel_id = ev.command.channel_id;
    msg.set_content("**__Roles__**");

    ev.reply(msg, error_autoprint);
    return true;
}