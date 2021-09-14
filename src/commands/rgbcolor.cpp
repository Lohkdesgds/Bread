#include "rgbcolor.hpp"

void __handle_command_rgb(dpp::cluster& core, const dpp::interaction_create_t& src, dpp::command_interaction& cmd, std::shared_ptr<EachLang> lang)
{
    dpp::message replying;
    replying.id = src.command.id;
    replying.channel_id = src.command.channel_id;
    replying.set_type(dpp::message_type::mt_application_command);
    replying.set_flags(64);

    auto r = get_int_in_command(cmd, lang->get(lang_command::RGB2DECIMAL_RED));
    auto g = get_int_in_command(cmd, lang->get(lang_command::RGB2DECIMAL_GREEN));
    auto b = get_int_in_command(cmd, lang->get(lang_command::RGB2DECIMAL_BLUE));

    int result = rgb_to_decimal_color(
        r.is_null() ? 0 : *r,
        g.is_null() ? 0 : *g,
        b.is_null() ? 0 : *b
    );

    replying.set_content(replaceargformatdirect(lang->get(lang_line::COMMAND_RGB2DECIMAL_RESULT), {std::to_string(result)}));

    src.reply(dpp::interaction_response_type::ir_channel_message_with_source, replying);
}