#include "tiem.hpp"

void __handle_command_thetime(dpp::cluster& core, const dpp::interaction_create_t& src, dpp::command_interaction& cmd, std::shared_ptr<EachLang> lang)
{
    dpp::message replying;
    replying.id = src.command.id;
    replying.channel_id = src.command.channel_id;
    replying.set_type(dpp::message_type::mt_application_command);
    replying.set_flags(0);
    //replying.nonce = ;
    replying.content = "<t:" + std::to_string(get_time_ms() / 1000) + ":F>";

    src.reply(dpp::interaction_response_type::ir_channel_message_with_source, replying);
}