#include "ping.hpp"

//void __handle_message_ping(dpp::cluster& core, const dpp::message_create_t& ev, std::shared_ptr<EachLang> lang)
//{
//    if (ev.msg->author->id == core.me.id){
//        std::string ref = ev.msg->content;
//        const mull time_ms_now = get_time_ms();
//
//        if (ref.find(ping_start_text_content) == 0){
//            ref.erase(0, ping_start_text_content.size());
//            const mull last_time = std::stoull(ref);
//            ev.msg->content = 
//                "EDIT:      `" + std::to_string((time_ms_now - last_time)) + " ms`\n"
//                "HTTPS/DPP: `" + std::to_string(static_cast<int>(core.rest_ping * 1000.0)) + " ms`";
//            core.message_edit(*ev.msg);
//        }
//    }
//}
//
//void __handle_command_ping(dpp::cluster& core, const dpp::interaction_create_t& src, dpp::command_interaction& cmd, std::shared_ptr<EachLang> lang)
//{
//    dpp::message replying;
//    replying.id = src.command.id;
//    replying.channel_id = src.command.channel_id;
//    replying.set_type(dpp::message_type::mt_application_command);
//    replying.set_flags(0);
//    //replying.nonce = ;
//    replying.content = ping_start_text_content + std::to_string(get_time_ms()) + "||\n...";
//
//    src.reply(dpp::interaction_response_type::ir_channel_message_with_source, replying);
//}

void __handle_command_ping(dpp::cluster& core, const dpp::interaction_create_t& src, dpp::command_interaction& cmd, std::shared_ptr<EachLang> lang)
{
    dpp::message replying;
    replying.id = src.command.id;
    replying.channel_id = src.command.channel_id;
    replying.set_type(dpp::message_type::mt_application_command);
    replying.set_flags(64);
    replying.content = "`" + std::to_string(static_cast<int>(core.rest_ping * 1000.0)) + " ms`";

    src.reply(dpp::interaction_response_type::ir_channel_message_with_source, replying);
}