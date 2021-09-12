#include "tags.hpp"

void __handle_guild_member_update(dpp::cluster& core, const dpp::guild_member_update_t& ev, std::shared_ptr<EachLang> lang)
{
    if (ev.updated.is_pending()) return; // not ready

    //auto uconf = get_user_config(ev.updated.user_id);
    auto gconf = get_guild_config(ev.updating_guild->id);

    const auto& roles_ruling = gconf->get_roles_joined();
    if (roles_ruling.empty()) return; // no roles configured

    dpp::guild_member member;
    member.guild_id = ev.updating_guild->id;
    member.user_id 	= ev.updated.user_id;
    member.roles 	= ev.updated.roles;

    bool has_news = false;

    for (const auto& i : roles_ruling) {
        if (std::find(member.roles.begin(), member.roles.end(), i) == member.roles.end()) {
            member.roles.push_back(i);
            has_news = true;
        }
    }

    if (!has_news) return;

    core.guild_edit_member(member, [gconf](const dpp::confirmation_callback_t data) mutable {
        if (data.is_error()){
            gconf->post_log("Can't update member's roles (autorole) -> ERR#" + std::to_string(data.http_info.status) + " BODY: " + data.http_info.body);
            Lunaris::cout << "[UPDATE_TAGS] Failed pending autotag task." ;
            Lunaris::cout << "Response #" << data.http_info.status << ": " << data.http_info.body ;
        }
    });
}

void __handle_command_tags(dpp::cluster& core, const dpp::interaction_create_t& src, dpp::command_interaction& cmd, std::shared_ptr<EachLang> lang)
{
    dpp::message replying;
    replying.id = src.command.id;
    replying.channel_id = src.command.channel_id;
    replying.set_type(dpp::message_type::mt_application_command);
    replying.set_flags(64);
    replying.nonce = std::to_string(src.command.usr.id);

    src.reply(dpp::interaction_response_type::ir_deferred_channel_message_with_source, replying);
    
    mull user_target = src.command.usr.id;

    if (cmd.options.size() != 1 || cmd.options[0].options.size() != 1){
        src.edit_response(lang->get(lang_line::GENERIC_BOT_FAILED_DOING_TASK));
        return;
    }

    const std::string category = fix_name_for_cmd(cmd.options[0].name);
    //Lunaris::cout << category << "=CATEGORY\n";
    const mull roleid = std::stoull(std::get<std::string>(cmd.options[0].options[0].value));
    //Lunaris::cout << roleid << "=ROLEID\n";

    auto gconf = get_guild_config(src.command.guild_id);
    const auto& roles_in_guild = gconf->get_roles_map();

    const auto found = std::find_if(roles_in_guild.begin(), roles_in_guild.end(), [&](const guild_data::category& it){ return fix_name_for_cmd(it.name) == category; });
    if (found == roles_in_guild.end()){
        src.edit_response(lang->get(lang_line::COMMAND_GENERIC_INVALID_ARGS));
        return;
    }

    bool found_in_list = false;
    for(const auto& i : found->list){
        if (i.id == roleid) {
            found_in_list = true;
            break;
        }
    }
    if (!found_in_list){
        src.edit_response(lang->get(lang_line::COMMAND_GENERIC_INVALID_ARGS));
        return;
    }

    // ALL GOOD
    dpp::guild_member member;
    member.guild_id = src.command.guild_id;
    member.user_id 	= src.command.usr.id;
    member.roles 	= src.command.member.roles;

    if (!found->can_combine) {
        for (const auto& inn : found->list) {
            auto it2 = std::find(member.roles.begin(), member.roles.end(), inn.id);
            if (it2 != member.roles.end() && *it2 != roleid) {
                member.roles.erase(it2);
                //Lunaris::cout << "REMOVEID=" << inn.id ;
            }
        }
    }
    
    auto it = std::find(member.roles.begin(), member.roles.end(), roleid);
    bool was_add = false;

    if (it == member.roles.end()) {
        member.roles.push_back(roleid);
        was_add = true;
    }
    else{
        member.roles.erase(it);
        was_add = false;
    }

    core.guild_edit_member(member, [src, lang, replying, was_add, roleid, gconf](const dpp::confirmation_callback_t data) mutable {
        if (data.is_error()){
            replying.set_content(lang->get(lang_line::GENERIC_BOT_FAILED_DISCORD_ERROR));
            gconf->post_log("Can't edit member role (tags) -> ERR#" + std::to_string(data.http_info.status) + " BODY: " + data.http_info.body);
        }
        else {
            replying.set_content(replaceargformatdirect(lang->get(was_add ? lang_line::COMMAND_GENERIC_SUCCESS_ADD_WITH_NAME : lang_line::COMMAND_GENERIC_SUCCESS_REMOVE_WITH_NAME), {"<@&" + std::to_string(roleid) + ">"}));
                //was_add ? u8"Seu cargo foi adicionado com sucesso!" : u8"Seu cargo foi removido com sucesso!");
        }
        src.edit_response(replying);
    });
}