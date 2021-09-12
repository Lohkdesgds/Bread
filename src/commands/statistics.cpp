#include "statistics.hpp"

void __handle_message_statistics(dpp::cluster& core, const dpp::message_create_t& ev)
{
    auto uconf = get_user_config(ev.msg->author->id);
    uconf->add_message_sent_once(ev.msg->guild_id);
    if (ev.msg->attachments.size()) uconf->add_attachment_sent_once(ev.msg->guild_id);
}

void __handle_command_statistics_any(dpp::cluster& core, const dpp::interaction_create_t& src, dpp::command_interaction& cmd)
{
    auto uconf = get_user_config(src.command.usr.id);
    uconf->add_command_sent_once();
}

void __handle_command_statistics(dpp::cluster& core, const dpp::interaction_create_t& src, dpp::command_interaction& cmd, std::shared_ptr<EachLang> lang)
{
    dpp::message replying;
    replying.id = src.command.id;
    replying.channel_id = src.command.channel_id;
    replying.set_type(dpp::message_type::mt_application_command);
    replying.set_flags(64);
    replying.nonce = std::to_string(src.command.usr.id);

    src.reply(dpp::interaction_response_type::ir_deferred_channel_message_with_source, replying);
    
    mull user_target = src.command.usr.id;

    auto user_raw = get_mull_in_command(cmd, lang->get(lang_command::STATS_USER));
    if (!user_raw.is_null()) user_target = *user_raw;


    core.user_get(user_target, [src, &core, replying, lang](const dpp::confirmation_callback_t& data) mutable {
        if (!data.is_error()){
            dpp::user user = std::get<dpp::user>(data.value);

            const auto uconf = get_user_config(user.id);

            const mull messages_total = uconf->get_total_messages();
            const mull messages_guild_total = uconf->get_total_messages_at_guild(src.command.guild_id);

            const mull attachments_total = uconf->get_total_attachments();
            const mull attachments_guild_total = uconf->get_total_attachments_at_guild(src.command.guild_id);
            
            dpp::embed autoembed = dpp::embed()
                .set_author(
                    dpp::embed_author{
                        .name = (user.username + "#" + std::to_string(user.discriminator)),
                        .url = user.get_avatar_url(),
                        .icon_url = user.get_avatar_url() + "?size=256"
                    })
                .set_title("**__" + lang->get(lang_line::COMMAND_STATS_FINAL_TITLE_NAME) + "__**")
                //.set_description("**―――――――――――――――――――――**") // 21 lines
                //.set_footer(dpp::embed_footer().set_text(u8"A barra de progresso indica quanto avançou no nível atual."))
                .set_color(uconf->get_user_color())
                .set_thumbnail(statistics_image_url)
                .add_field(
                    lang->get(lang_line::COMMAND_STATS_FINAL_TOTAL_MESSAGES), (u8"📚 " + std::to_string(messages_total)), true
                )
                .add_field(
                    lang->get(lang_line::COMMAND_STATS_FINAL_MESSAGES_HERE), (u8"📓 " + std::to_string(messages_guild_total)), true
                )
                .add_field(
                    lang->get(lang_line::COMMAND_STATS_FINAL_MESSAGES_PERC), (u8"🔖 " + std::to_string(static_cast<int>((100 * messages_guild_total) / (messages_total == 0 ? 1 : messages_total))) + "%"), true
                )
                .add_field(
                    lang->get(lang_line::COMMAND_STATS_FINAL_TOTAL_FILES), (u8"🗂️ " + std::to_string(attachments_total)), true
                )
                .add_field(
                    lang->get(lang_line::COMMAND_STATS_FINAL_FILES_HERE), (u8"📁 " + std::to_string(attachments_guild_total)), true
                )
                .add_field(
                    lang->get(lang_line::COMMAND_STATS_FINAL_FILES_PERC), (u8"⚙️ " + std::to_string(static_cast<int>((100 * attachments_guild_total) / (attachments_total == 0 ? 1 : attachments_total))) + "%"), true
                )
                .add_field(
                    lang->get(lang_line::COMMAND_STATS_FINAL_TOTAL_COMMANDS), (u8"⚡ " + std::to_string(uconf->get_total_commands())), true
                );

            replying.embeds.push_back(autoembed);
            replying.set_content("");
            replying.set_flags(64);

            src.edit_response(replying);
        }
        else {
            auto gconf = get_guild_config(src.command.guild_id);
            gconf->post_log("Can't configure this guild -> ERR#" + std::to_string(data.http_info.status) + " BODY: " + data.http_info.body);
            src.edit_response(lang->get(lang_line::GENERIC_BOT_FAILED_DOING_TASK));
        }
    });
}