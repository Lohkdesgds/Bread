#include "eu.hpp"

void __handle_command_eu(dpp::cluster& core, const dpp::interaction_create_t& src, dpp::command_interaction& cmd, std::shared_ptr<EachLang> lang)
{
    const mull user_id = src.command.usr.id;

    dpp::message replying;
    replying.id = src.command.id;
    replying.channel_id = src.command.channel_id;
    replying.set_type(dpp::message_type::mt_application_command);
    replying.set_flags(64);
    replying.nonce = std::to_string(src.command.usr.id);

    src.reply(dpp::interaction_response_type::ir_deferred_channel_message_with_source, replying);

    if (cmd.options.size() == 0) {
        replying.set_content(lang->get(lang_line::COMMAND_GENERIC_INVALID_ARGS));
        src.edit_response(replying);
        return;
    }

    static const std::initializer_list<lang_command> self_commands = { lang_command::SELFCONF_COLOR, lang_command::SELFCONF_LEVELNOTIF };

    const auto name = get_first_name(cmd);
    const auto options = get_first_option(cmd);

    //const std::string error_message_invalid_args = u8"Alguma coisa muito errada aconteceu. Acho que os argumentos não batem com o esperado! Não pude completar a tarefa.";
    //const std::string error_message_invalid_args_with_name = u8"Alguma coisa muito errada aconteceu. Algo deu errado com o argumento {0}.";
    //const std::string error_message_cant_find_x = u8"Não consegui encontrar '{0}'.";

    if (name.is_null()){ src.edit_response(lang->get(lang_line::COMMAND_GENERIC_INVALID_ARGS)); return; }

    switch(lang->command_from(*name, self_commands)) {
    case lang_command::SELFCONF_COLOR:
        {
            const auto color = get_int_in_command(options, lang->get(lang_command::SELFCONF_COLOR_VALUE));
            if (!color) { src.edit_response(replaceargformatdirect(lang->get(lang_line::COMMAND_GENERIC_INVALID_ARGS_WITH_NAME), {lang->get(lang_command::SELFCONF_COLOR_VALUE)})); return; }

            auto uconf = get_user_config(user_id);
            uconf->set_user_color(*color);

            src.edit_response(*color < 0 ? lang->get(lang_line::GENERIC_BOT_SUCCESS_ON_RESET) : lang->get(lang_line::GENERIC_BOT_SUCCESS));
        }
        return;
    case lang_command::SELFCONF_LEVELNOTIF:
        {
            const auto enable = get_bool_in_command(options, lang->get(lang_command::SELFCONF_LEVELNOTIF_ENABLE));
            if (!enable) { src.edit_response(replaceargformatdirect(lang->get(lang_line::COMMAND_GENERIC_INVALID_ARGS_WITH_NAME), {lang->get(lang_command::SELFCONF_LEVELNOTIF_ENABLE)})); return; }

            auto uconf = get_user_config(user_id);
            uconf->set_show_level_up(*enable);

            src.edit_response(lang->get(lang_line::GENERIC_BOT_SUCCESS));
        }
        return;
    default:
        break;
    }
    src.edit_response(lang->get(lang_line::GENERIC_BOT_FAILED_DOING_TASK));
}