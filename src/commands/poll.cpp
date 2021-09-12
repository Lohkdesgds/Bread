#include "poll.hpp"

void __handle_reaction_poll(dpp::cluster& core, const dpp::message_reaction_add_t& ev, std::shared_ptr<EachLang> lang)
{
    std::string em_name;
    mull em_id = 0;
    mull user_targ = 0;

    nlohmann::json pars = nlohmann::json::parse(ev.raw_event)["d"];

    //Lunaris::cout << "JSON=\n" << pars.dump(2) ;

    if (auto inn = pars.find("emoji"); inn != pars.end()){
        if (auto namm = inn->find("name"); namm != inn->end()) em_name = namm->is_null() ? "" : namm->get<std::string>();
        if (auto namm = inn->find("id"); namm != inn->end()) em_id = namm->is_null() ? 0 : (namm->is_number() ? namm->get<mull>() : std::stoull(namm->get<std::string>()));
    }
    if (auto inn = pars.find("user_id"); inn != pars.end()) user_targ = inn->is_null() ? 0 : (inn->is_number() ? inn->get<mull>() : std::stoull(inn->get<std::string>()));
    
    if ((em_id == 0 && em_name.empty()) || user_targ == 0) return;
    
    core.message_get(ev.message_id, ev.reacting_channel->id, [&core, lang, em_name, em_id, user_targ, guildid = ev.reacting_guild->id](const dpp::confirmation_callback_t& data) {
        if (data.is_error()) {
            auto gconf = get_guild_config(guildid);
            gconf->post_log("Failed handling poll check event -> ERR#" + std::to_string(data.http_info.status) + " BODY: " + data.http_info.body);
            return;
        }

        dpp::message msg = std::get<dpp::message>(data.value);
        dpp::emoji tool;
        tool.name = em_name;
        tool.id = em_id;


        if (msg.interaction.name == lang->get(lang_command::POLL)) {
            if (user_targ == msg.interaction.usr.id){
                if (tool.id == 0 && tool.name == poll_emoji_delete_reactions){
                    core.message_delete_all_reactions(msg);
                }
                else if (tool.id == 0 && tool.name == poll_emoji_delete_message){
                    core.message_delete(msg.id, msg.channel_id, [&core, guildid](const dpp::confirmation_callback_t& data){
                        if (data.is_error()) {
                            auto gconf = get_guild_config(guildid);
                            gconf->post_log("Failed handling poll delete message event -> ERR#" + std::to_string(data.http_info.status) + " BODY: " + data.http_info.body);
                            return;
                        }
                    });
                }
                else {
                    core.message_add_reaction(msg, tool.format());
                }
            }
            else {
                for (const auto& reacts : msg.reactions) {
                    if (reacts.emoji_id == tool.id && reacts.emoji_name == tool.name && reacts.me) {
                        return;
                    }
                }
                core.message_delete_reaction(msg, user_targ, tool.format());
            }
        }
    });
}

void __handle_command_poll(dpp::cluster& core, const dpp::interaction_create_t& src, dpp::command_interaction& cmd, std::shared_ptr<EachLang> lang)
{
    dpp::message replying;
    replying.id = src.command.id;
    replying.channel_id = src.command.channel_id;
    replying.set_type(dpp::message_type::mt_application_command);
    replying.set_flags(0);
    
    src.reply(dpp::interaction_response_type::ir_deferred_channel_message_with_source, replying);

    const auto uconf = get_user_config(src.command.usr.id);

    auto text_raw = get_str_in_command(cmd, lang->get(lang_command::POLL_TEXT));
    auto title_raw = get_str_in_command(cmd, lang->get(lang_command::POLL_TITLE));
    auto link = get_str_in_command(cmd, lang->get(lang_command::POLL_LINK));
    auto emojis_raw = get_str_in_command(cmd, lang->get(lang_command::POLL_EMOJIS));
    auto mode = get_int_in_command(cmd, lang->get(lang_command::POLL_MODE));
    auto colr = get_int_in_command(cmd, lang->get(lang_command::POLL_COLOR));

    std::vector<std::string> emojis;
    std::string text;
    const std::string title = title_raw.is_null() ? ("**" + lang->get(lang_line::COMMAND_POLL_FINAL_TITLE_NAME) + "**") : ("**" + *title_raw + "**");
    const int32_t color = colr.is_null() ? uconf->get_user_color() : ((*colr) % 0xFFFFFF);

    if (!emojis_raw.is_null()){
        std::stringstream ss(*emojis_raw);
        std::string token;
        while (std::getline(ss, token, ';')){
            
            while(token.length() && (token.front() == ' ' || token.front() == '<' || token.front() == 'a' || token.front() == ':'))
                token.erase(token.begin());
            
            while(token.length() && (token.back() == ' ' || token.back() == '>')) 
                token.pop_back();

            if (token.size()) {
                emojis.push_back(token);
            }
        }
    }

    text = *text_raw;
    for(size_t p = 0; p != std::string::npos;)
    {
        if ((p = text.find("\\n")) != std::string::npos){
            text.erase(p, 2);
            text.insert(p, "\n");
        }
    }

    dpp::embed poll_enq;
    dpp::embed_author author;
    author.name = src.command.usr.username + "#" + std::to_string(src.command.usr.discriminator);
    author.icon_url = src.command.usr.get_avatar_url() + "?size=256";

    poll_enq.set_author(author);
    poll_enq.set_title(title);
    poll_enq.set_description(text);
    poll_enq.set_color(color);
    poll_enq.set_thumbnail(poll_image_url);

    if (!link.is_null()) poll_enq.set_image(*link);
    if (emojis.size() == 0){
        switch(mode.is_null() ? 0 : *mode){
        case 0: // ^ v
        {
            emojis.push_back(u8"🔺");
            emojis.push_back(u8"🔻");
        }
            break;
        case 1: // thumbs
        {
            emojis.push_back(u8"👍");
            emojis.push_back(u8"👎");
        }
            break;
        case 2: // 0 a 5
        {
            emojis.push_back(u8"0️⃣");
            emojis.push_back(u8"1️⃣");
            emojis.push_back(u8"2️⃣");
            emojis.push_back(u8"3️⃣");
            emojis.push_back(u8"4️⃣");
            emojis.push_back(u8"5️⃣");
        }
            break;
        case 3: // 1 a 5
        {
            emojis.push_back(u8"1️⃣");
            emojis.push_back(u8"2️⃣");
            emojis.push_back(u8"3️⃣");
            emojis.push_back(u8"4️⃣");
            emojis.push_back(u8"5️⃣");
        }
            break;
        case 4: // confirm
        {
            emojis.push_back(u8"👍");
        }
            break;
        }
    }

    replying.add_embed(poll_enq);

    src.edit_response(replying);

    if (emojis.size()) {

        auto gconf = get_guild_config(src.command.guild_id);

        src.get_original_response([&core, emojis, gconf](const dpp::confirmation_callback_t& data) {
            if (data.is_error()) {
                gconf->post_log("Could not get command message for reaction -> ERR#" + std::to_string(data.http_info.status) + " BODY: " + data.http_info.body);
                return;
            }

            dpp::message msg = std::get<dpp::message>(data.value);
            for(auto& i : emojis) {
                core.message_add_reaction(msg, i);
            }
        });
    }
}