#include "points.hpp"

void __handle_message_points(dpp::cluster& core, const dpp::message_create_t& ev, std::shared_ptr<EachLang> lang)
{
    if (ev.msg.author.is_bot()) return;

    const mull user_id = ev.msg.author.id;
    const mull guild_id = ev.msg.guild_id;

    auto gconf = get_guild_config(guild_id);

    if (!gconf->is_earning_points_time()) return;

    auto uconf = get_user_config(user_id);

    if (!uconf->is_earning_points_time()) return;

    mull old_pts_g = uconf->get_points();
    mull old_pts_l = uconf->get_points_at_guild(guild_id);

    int boost_final = 0;
    const bool had_boost = (random() % range_boost_chances == 0);

    if (had_boost) boost_final = static_cast<int>(rand() % range_boost_total) + range_boost_low;
    else boost_final = static_cast<int>(rand() % range_total) + range_low;

    uconf->add_points(guild_id, boost_final);

    gconf->set_earned_points();

    //Lunaris::cout << ev.msg->author->username << " -> " << std::to_string(boost_final) ;
    
    
    mull old_lvl_g = 1, old_lvl_l = 1;
    mull new_lvl_g = 1, new_lvl_l = 1;

    mull new_pts_g = uconf->get_points();
    mull new_pts_l = uconf->get_points_at_guild(guild_id);

    while(static_cast<mull>(pow(calc_level_div, old_lvl_g + threshold_points_level_0 - 1)) <= old_pts_g) old_lvl_g++;
    while(static_cast<mull>(pow(calc_level_div, old_lvl_l + threshold_points_level_0 - 1)) <= old_pts_l) old_lvl_l++;
    while(static_cast<mull>(pow(calc_level_div, new_lvl_g + threshold_points_level_0 - 1)) <= new_pts_g) new_lvl_g++;
    while(static_cast<mull>(pow(calc_level_div, new_lvl_l + threshold_points_level_0 - 1)) <= new_pts_l) new_lvl_l++;

    const bool global_level_up = (new_lvl_g > old_lvl_g && new_lvl_g >= 1);
    const bool local_level_up =  (new_lvl_l > old_lvl_l && new_lvl_l >= 1);
    const bool global_level_down = (new_lvl_g < old_lvl_g && old_lvl_g >= 1);
    const bool local_level_down =  (new_lvl_l < old_lvl_l && old_lvl_l >= 1);

    if (!global_level_up && !local_level_up && !global_level_down && !local_level_down) return;
    const bool was_level_up = (global_level_up || local_level_up);

    if (local_level_up || local_level_down) {
        const auto& vecref = gconf->get_roles_per_level_map();

        if (vecref.size() > 0) {
            dpp::guild_member member;
            member.guild_id = guild_id;
            member.user_id 	= user_id;
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
                core.guild_edit_member(member, [gconf](const dpp::confirmation_callback_t data){
                    if (data.is_error()) {
                        gconf->post_log("Couldn't set member's new roles (based on points) -> ERR#" + std::to_string(data.http_info.status) + " BODY: " + data.http_info.body);
                        Lunaris::cout << "[POINTS_LEVEL_UP] Error: " << data.http_info.body ;
                    }
                });
            }
        }
    }

    if (uconf->get_show_level_up() && !gconf->is_message_level_blocked()) {

        dpp::message replying;
        replying.set_type(dpp::message_type::mt_reply);
        if (const auto _temp = gconf->get_level_channel_id(); _temp != 0) {
            replying.channel_id = _temp;
        }
        else {
            replying.channel_id = ev.msg.channel_id;
            replying.message_reference.channel_id = ev.msg.channel_id;
            replying.message_reference.message_id = ev.msg.id;
        }
        replying.set_flags(64);

        std::string desc = (was_level_up ? ("📈 **" + lang->get(lang_line::COMMAND_POINTS_MESSAGE_LEVEL_UP) + "**\n") : ("📉 **" + lang->get(lang_line::COMMAND_POINTS_MESSAGE_LEVEL_DOWN) + "**\n"));
        if (global_level_up || global_level_down) desc += (desc.length() ? "\n" : "") + (u8"✨ **" + lang->get(lang_line::COMMAND_POINTS_MESSAGE_GLOBAL_TAG) + ":** " + std::string(was_level_up ? u8"🔺" : u8"🔻") + u8" `" + lang->get(lang_line::COMMAND_POINTS_MESSAGE_LEVEL) + " " + std::to_string(new_lvl_g) + "`");
        if (local_level_up  || local_level_down)  desc += (desc.length() ? "\n" : "") + (u8"✨ **" + lang->get(lang_line::COMMAND_POINTS_MESSAGE_LOCAL_TAG) + ":** "  + std::string(was_level_up ? u8"🔺" : u8"🔻") + u8" `" + lang->get(lang_line::COMMAND_POINTS_MESSAGE_LEVEL) + " " + std::to_string(new_lvl_l) + "`");

        //Lunaris::cout << "Level up stuff:\nTITLE=" << title << "\nDESC=" << desc ;

        dpp::embed autoembed = dpp::embed()
            .set_author(
                dpp::embed_author{
                    .name = format_user(ev.msg.author),
                    .url = ev.msg.author.get_avatar_url(256),
                    .icon_url = ev.msg.author.get_avatar_url(256)
                })
            .set_description(desc)
            .set_color(uconf->get_user_color())
            .set_thumbnail(points_image_url);

        replying.embeds.push_back(autoembed);
        replying.set_content("");


        core.message_create(replying);
    }
}

void __handle_command_points(dpp::cluster& core, const dpp::interaction_create_t& src, dpp::command_interaction& cmd, std::shared_ptr<EachLang> lang)
{
    dpp::message replying;
    replying.id = src.command.id;
    replying.channel_id = src.command.channel_id;
    replying.set_type(dpp::message_type::mt_application_command);
    replying.set_flags(64);
    replying.nonce = std::to_string(src.command.usr.id);

    src.reply(dpp::interaction_response_type::ir_deferred_channel_message_with_source, replying);

    mull user_target = src.command.usr.id;
    bool is_global = false;

    if (const auto __temp = get_mull_in_command(cmd, lang->get(lang_command::POINTS_USER)); !__temp.is_null()) user_target = *__temp;
    if (const auto __temp = get_bool_in_command(cmd, lang->get(lang_command::POINTS_GLOBAL)); !__temp.is_null()) is_global = *__temp;

    core.user_get(user_target, [src, &core, is_global, lang, replying](const dpp::confirmation_callback_t& data) mutable {
        if (data.is_error()){
            src.edit_response(lang->get(lang_line::GENERIC_BOT_FAILED_DOING_TASK));
            auto gconf = get_guild_config(src.command.guild_id);
            gconf->post_log("Could not load user information -> ERR#" + std::to_string(data.http_info.status) + " BODY: " + data.http_info.body);
        }

        dpp::user_identified user = std::get<dpp::user_identified>(data.value);

        const auto uconf = get_user_config(user.id);
        
        const mull usrpts = is_global ? uconf->get_points() : uconf->get_points_at_guild(src.command.guild_id);
        const mull threshold = pow(calc_level_div, threshold_points_level_0);

        mull currlevel = 1;
        while(static_cast<mull>(pow(calc_level_div, currlevel + threshold_points_level_0 - 1)) <= usrpts) currlevel++;

        const mull next_level_raw = static_cast<mull>(pow(calc_level_div, currlevel + (threshold_points_level_0 - 1)));
        const mull nxtg = (next_level_raw - usrpts);
        
        int perc_calc = 0;
        {
            if (currlevel <= 1) {
                perc_calc = (100 * usrpts / threshold);
            }
            else{
                const mull past_level = static_cast<mull>(pow(calc_level_div, currlevel + (threshold_points_level_0 - 2)));
                const mull diff_levels = next_level_raw - past_level;
                const mull points_relative = usrpts - past_level;
                perc_calc = points_relative * 100 / diff_levels;
            }
        }

        dpp::embed autoembed = dpp::embed()
            .set_author(
                dpp::embed_author{
                    .name = format_user(user),
                    .url = user.get_avatar_url(256),
                    .icon_url = user.get_avatar_url(256)
                })
            .set_title((is_global ? (" **__" + lang->get(lang_line::COMMAND_POINTS_FINAL_GLOBAL) + "__**") : (" **__" + lang->get(lang_line::COMMAND_POINTS_FINAL_LOCAL) + "__**")))
            .set_color(uconf->get_user_color())
            .set_thumbnail(points_image_url)
            .set_image(__generate_points_url(perc_calc))
            .add_field(
                lang->get(lang_line::COMMAND_POINTS_FINAL_LEVEL_NOW), (u8"✨ " + std::to_string(currlevel)), true
            )
            .add_field(
                lang->get(lang_line::COMMAND_POINTS_FINAL_POINTS_NOW), (u8"🧬 " + std::to_string(usrpts)), true
            )
            .add_field(
                lang->get(lang_line::COMMAND_POINTS_FINAL_NEXT_LEVEL_IN), (u8"📈 " + std::to_string(nxtg > 0 ? nxtg : 1)), true
            );

        replying.embeds.push_back(autoembed);
        replying.set_content("");
        replying.set_flags(64);

        src.edit_response(replying);
    });
}

std::string __generate_points_url(int perc)
{
    if (perc < 0) perc = 0;
    if (perc > 100) perc = 100;
    return std::string(dna_points_percentage_per_percent_begin) + dna_points_percentage_per_percent[perc] + dna_points_percentage_per_percent_end;
}