#include "tiem.hpp"

void __handle_command_thetime(dpp::cluster& core, const dpp::interaction_create_t& src, dpp::command_interaction& cmd, std::shared_ptr<EachLang> lang)
{
    dpp::message replying;
    replying.id = src.command.id;
    replying.channel_id = src.command.channel_id;
    replying.set_type(dpp::message_type::mt_application_command);
    replying.set_flags(0);

    auto days_off = get_int_in_command(cmd, lang->get(lang_command::TIME_DAY_OFFSET));
    auto hour_off = get_int_in_command(cmd, lang->get(lang_command::TIME_HOUR_OFFSET));
    auto mins_off = get_int_in_command(cmd, lang->get(lang_command::TIME_MINUTE_OFFSET));

    mull finalt = get_time_ms() / 1000;

    if (!days_off.is_null()) {
        finalt += (*days_off) * 86400;
    }
    if (!hour_off.is_null()) {
        finalt += (*hour_off) * 3600;
    }
    if (!mins_off.is_null()) {
        finalt += (*mins_off) * 60;
    }

    //replying.nonce = ;
    replying.content = "<t:" + std::to_string(finalt) + ":F>\n||EPOCH:" + std::to_string(finalt) + "||";

    src.reply(dpp::interaction_response_type::ir_channel_message_with_source, replying);
}