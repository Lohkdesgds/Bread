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

    if (cmd.options.size() == 0) {
        replying.set_content(lang->get(lang_line::COMMAND_GENERIC_INVALID_ARGS));
        src.edit_response(replying);
        return;
    }

    static const std::initializer_list<lang_command> self_commands = { lang_command::SELFCONF_COLOR, lang_command::SELFCONF_LEVELNOTIF, lang_command::SELFCONF_DATA };

    const auto name = get_first_name(cmd);
    const auto options = get_first_option(cmd);

    //const std::string error_message_invalid_args = u8"Alguma coisa muito errada aconteceu. Acho que os argumentos não batem com o esperado! Não pude completar a tarefa.";
    //const std::string error_message_invalid_args_with_name = u8"Alguma coisa muito errada aconteceu. Algo deu errado com o argumento {0}.";
    //const std::string error_message_cant_find_x = u8"Não consegui encontrar '{0}'.";

    if (name.is_null()){ 
        replying.set_content(lang->get(lang_line::COMMAND_GENERIC_INVALID_ARGS)); 
        src.reply(dpp::interaction_response_type::ir_channel_message_with_source, replying);
        return; 
    }

    switch(lang->command_from(*name, self_commands)) {
    case lang_command::SELFCONF_COLOR:
        {
            const auto color = get_int_in_command(options, lang->get(lang_command::SELFCONF_COLOR_VALUE));
            if (!color) { 
                replying.set_content(replaceargformatdirect(lang->get(lang_line::COMMAND_GENERIC_INVALID_ARGS_WITH_NAME), {lang->get(lang_command::SELFCONF_COLOR_VALUE)}));
                src.reply(dpp::interaction_response_type::ir_channel_message_with_source, replying);
                return;
            }

            auto uconf = get_user_config(user_id);
            uconf->set_user_color(*color);

            replying.set_content(*color < 0 ? lang->get(lang_line::GENERIC_BOT_SUCCESS_ON_RESET) : lang->get(lang_line::GENERIC_BOT_SUCCESS));
            src.reply(dpp::interaction_response_type::ir_channel_message_with_source, replying);
        }
        return;
    case lang_command::SELFCONF_LEVELNOTIF:
        {
            const auto enable = get_bool_in_command(options, lang->get(lang_command::SELFCONF_LEVELNOTIF_ENABLE));
            if (!enable) { 
                replying.set_content(replaceargformatdirect(lang->get(lang_line::COMMAND_GENERIC_INVALID_ARGS_WITH_NAME), {lang->get(lang_command::SELFCONF_LEVELNOTIF_ENABLE)}));
                src.reply(dpp::interaction_response_type::ir_channel_message_with_source, replying);
                return;
            }

            auto uconf = get_user_config(user_id);
            uconf->set_show_level_up(*enable);

            replying.set_content(lang->get(lang_line::GENERIC_BOT_SUCCESS));
            src.reply(dpp::interaction_response_type::ir_channel_message_with_source, replying);
        }
        return;
    case lang_command::SELFCONF_DATA:
    {
        auto uconf = get_user_config(user_id);
        
        const std::string str = uconf->export_json();

        dpp::message alt;
        alt.set_file_content(str);
        alt.set_filename("userdata.json");
        alt.set_content("userdata.json:");

        core.direct_message_create(src.command.usr.id, alt);

        replying.set_content(lang->get(lang_line::GENERIC_BOT_GOOD_SEE_DM));
        src.reply(dpp::interaction_response_type::ir_channel_message_with_source, replying);
    }
        return;
    default:
        break;
    }
    replying.set_content(lang->get(lang_line::GENERIC_BOT_FAILED_DOING_TASK));
    src.reply(dpp::interaction_response_type::ir_channel_message_with_source, replying);
}