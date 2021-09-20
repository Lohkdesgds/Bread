#include "feedback.hpp"

void __handle_command_feedback(dpp::cluster& core, const dpp::interaction_create_t& src, dpp::command_interaction& cmd, std::shared_ptr<EachLang> lang)
{
    dpp::message replying;
    replying.id = src.command.id;
    replying.channel_id = src.command.channel_id;
    replying.set_type(dpp::message_type::mt_application_command);
    replying.set_flags(64);

    auto feedbk_text = get_str_in_command(cmd, lang->get(lang_command::FEEDBACK_TEXT));

    if (feedbk_text.is_null()) {
        replying.set_content(lang->get(lang_line::GENERIC_BOT_FAILED_DOING_TASK));
    }
    else {
        replying.set_content(lang->get(lang_line::GENERIC_BOT_SUCCESS));

        dpp::message msg;
        msg.channel_id = channel_feedback_id;
        msg.guild_id = guild_feedback_id;

        dpp::embed_author author;
        author.name = (src.command.usr.username + "#" + std::to_string(src.command.usr.discriminator));
        author.icon_url = src.command.usr.get_avatar_url();

        dpp::embed_footer foot;
        foot.set_text("From guild #" + std::to_string(src.command.guild_id));
        
        dpp::embed emb;
        emb.set_author(author);
        emb.set_title("**/FEEDBACK**");
        emb.set_description(*feedbk_text);
        emb.set_footer(foot);
        emb.set_color(0x77DD22);

        msg.add_embed(emb);

        core.message_create(msg);
    }

    src.reply(dpp::interaction_response_type::ir_channel_message_with_source, replying);
}