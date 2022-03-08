#include "copy_paste.hpp"

void __handle_command_clipboard_copy(dpp::cluster& core, const dpp::interaction_create_t& src, dpp::command_interaction& cmd, std::shared_ptr<EachLang> lang)
{
    dpp::message replying;
    replying.id = src.command.id;
    replying.channel_id = src.command.channel_id;
    replying.set_type(dpp::message_type::mt_application_command);
    replying.set_flags(64);

    src.reply(dpp::interaction_response_type::ir_deferred_channel_message_with_source, replying);

    auto uconf = get_user_config(src.command.usr.id);

    unsigned long long adapted_target_msgid = 0; // cmd.options[0].target_id is not available somehow
    dpp::message res;    

    try {
        nlohmann::json parsd = nlohmann::json::parse(src.raw_event);
        adapted_target_msgid = std::stoull(parsd["d"]["data"]["target_id"].get<std::string>());
        auto rnjson = parsd["d"]["data"]["resolved"]["messages"][std::to_string(adapted_target_msgid)];
        res.fill_from_json(&rnjson);
    }
    catch(...){
        replying.set_content(lang->get(lang_line::GENERIC_BOT_FAILED_DOING_TASK));
        src.edit_response(replying);
        return;
    }    

    uconf->set_clipboard(res.guild_id, res.channel_id, adapted_target_msgid);

    replying.set_content(lang->get(lang_line::COMMAND_COPY_SUCCESS_SPOILER_PASTE));

    src.edit_response(replying);
}

void __handle_command_clipboard_paste(dpp::cluster& core, const dpp::interaction_create_t& src, dpp::command_interaction& cmd, std::shared_ptr<EachLang> lang)
{
    const auto uconf = get_user_config(src.command.usr.id);
    const auto gconf = get_guild_config(src.command.guild_id);
    auto& clipboard = uconf->get_clipboard();

    dpp::message replying;
    replying.id = src.command.id;
    replying.channel_id = src.command.channel_id;
    replying.set_type(dpp::message_type::mt_application_command);

    if (!clipboard.has_data()){
        replying.set_flags(64);
        replying.set_content(lang->get(lang_line::COMMAND_PASTE_CLIPBOARD_EMPTY));
        src.reply(dpp::interaction_response_type::ir_channel_message_with_source, replying);
    }
    else if (clipboard.guild_id != src.command.guild_id && !gconf->get_can_paste_external_content())
    {
        replying.set_flags(64);
        replying.set_content(lang->get(lang_line::GENERIC_BOT_NOT_ALLOWED_BECAUSE_GUILD_SETTINGS));
        src.reply(dpp::interaction_response_type::ir_channel_message_with_source, replying);
    }
    else {
        replying.set_flags(0);
        src.reply(dpp::interaction_response_type::ir_deferred_channel_message_with_source, replying);

        //Lunaris::cout << "[DEBUG] MESSAGE_GET B4: {MSGID=" << clipboard.message_id << ";CHANNELID=" << clipboard.channel_id << "}";

        core.message_get(clipboard.message_id, clipboard.channel_id, [&core, clipboard, src, uconf, cmd, lang, replying, gconf](const dpp::confirmation_callback_t data) mutable {
            if (data.is_error()) {
                replying.set_flags(64);
                replying.set_content(lang->get(lang_line::GENERIC_BOT_FAILED_DOING_TASK));
                src.edit_response(replying);
                uconf->reset_clipboard(); // safe thing
                gconf->post_log("Could not read message -> ERR#" + std::to_string(data.http_info.status) + " BODY: " + data.http_info.body);
                return;
            }
            replying.set_flags(0);

            dpp::message sourcemsg = std::get<dpp::message>(data.value);

            //Lunaris::cout << "[DEBUG] MESSAGE_GET: {content=" << sourcemsg.content << ";attachments:" << sourcemsg.attachments.size() << ";author=" << sourcemsg.author.format_username() << "}";
            //Lunaris::cout << "[DEBUG] HTTP Body: " << data.http_info.body;

            auto paste_content = get_str_in_command(cmd, lang->get(lang_command::PASTE_TEXT));

            dpp::embed emb;
            dpp::embed_author authr;
            authr.icon_url = url_author_icon_clipboard;
            authr.name = lang->get(lang_line::COMMAND_PASTE_FINAL_ACCESS_DIRECTLY);
            authr.url = clipboard.generate_url();

            dpp::embed_footer emb_footer;
            emb_footer.set_text(lang->get(lang_line::COMMAND_PASTE_FINAL_REFERENCED_BY) + " " + format_user(src.command.usr) +
                ", " + lang->get(lang_line::COMMAND_PASTE_FINAL_REFERENCED_SOURCE) + ": " + format_user(sourcemsg.author) +
                ", " + lang->get(lang_line::COMMAND_PASTE_FINAL_REFERENCED_GUILD) + " #" + std::to_string(clipboard.guild_id));
            emb_footer.set_icon(src.command.usr.get_avatar_url(256));

            emb.set_author(authr);
            emb.set_color(uconf->get_user_color());
            emb.set_footer(emb_footer);

            if (!paste_content.is_null()){
                emb.description += "`" + lang->get(lang_line::COMMAND_PASTE_FINAL_USER_COMMENTED) + ":`\n```\n";
                emb.description += paste_content.get().substr(0, 512);
                emb.description += "```\n";
            }

            if (!sourcemsg.content.empty()) {
                emb.description += "`" + lang->get(lang_line::COMMAND_PASTE_FINAL_ORIGINAL_TEXT) + ":`\n```\n";
                for (const auto& i : sourcemsg.content) { if (i != '`') emb.description += i; }
                emb.description += "```\n";
            }

            if (sourcemsg.attachments.size()) {
                emb.set_image(sourcemsg.attachments[0].url);
            }

            if (sourcemsg.embeds.size()) emb.description += "`" + lang->get(lang_line::COMMAND_PASTE_FINAL_ORIGINAL_EMBED_BELOW) + ":`";

            replying.add_embed(emb);

            if (sourcemsg.embeds.size()) replying.add_embed(sourcemsg.embeds[0]);

            src.edit_response(replying);
        });
    }    
}