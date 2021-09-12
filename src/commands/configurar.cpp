#include "configurar.hpp"

void __handle_command_configurar(dpp::cluster& core, const dpp::interaction_create_t& src, dpp::command_interaction& cmd, std::shared_ptr<EachLang> lang)
{
    dpp::message replying;
    replying.id = src.command.id;
    replying.channel_id = src.command.channel_id;
    replying.set_type(dpp::message_type::mt_application_command);
    replying.set_flags(64);
    replying.nonce = std::to_string(src.command.usr.id);

    src.reply(dpp::interaction_response_type::ir_deferred_channel_message_with_source, replying);
    
    auto gconf = get_guild_config(src.command.guild_id);
    mull user_target = src.command.usr.id;
    const mull user_action = src.command.usr.id;

    // HANDLE

    if (gconf->is_config_locked()){
        src.edit_response(lang->get(lang_line::GENERIC_BOT_NOT_ALLOWED_BECAUSE_GUILD_SETTINGS));
        return;
    }

    const auto name = get_first_name(cmd);
    const auto options = get_first_option(cmd); // configurar <OPT> {the args}
    const auto suboptions = get_first_option(options); // configurar <OPT> <SUBOPT> {the args}

    static const std::initializer_list<lang_command> main_commands = { lang_command::CONFIG_APPLY, lang_command::CONFIG_EXTERNAL, lang_command::CONFIG_LOGS, lang_command::CONFIG_LANGUAGE, lang_command::CONFIG_ROLES, lang_command::CONFIG_POINTS, lang_command::CONFIG_ADMIN, lang_command::CONFIG_AUTOROLE, lang_command::CONFIG_LEVELS };
    static const std::initializer_list<lang_command> external_commands = { lang_command::CONFIG_EXTERNAL_CANPASTE };
    static const std::initializer_list<lang_command> roles_commands = { lang_command::CONFIG_ROLES_ADD, lang_command::CONFIG_ROLES_REMOVE, lang_command::CONFIG_ROLES_CLEANUP, lang_command::CONFIG_ROLES_COMBO };
    static const std::initializer_list<lang_command> admin_commands = { lang_command::CONFIG_ADMIN_ADD, lang_command::CONFIG_ADMIN_REMOVE, lang_command::CONFIG_ADMIN_VERIFY };
    static const std::initializer_list<lang_command> autorole_commands = { lang_command::CONFIG_AUTOROLE_ADD, lang_command::CONFIG_AUTOROLE_REMOVE, lang_command::CONFIG_AUTOROLE_VERIFY };
    static const std::initializer_list<lang_command> levels_commands = { lang_command::CONFIG_LEVELS_ADD, lang_command::CONFIG_LEVELS_REMOVE, lang_command::CONFIG_LEVELS_REDIRECT, lang_command::CONFIG_LEVELS_MESSAGES };

    if (name.is_null()){ src.edit_response(lang->get(lang_line::COMMAND_GENERIC_INVALID_ARGS)); return; }

    switch(lang->command_from(*name, main_commands)) {
    case lang_command::CONFIG_APPLY:
    {
        src.edit_response(lang->get(lang_line::COMMAND_GENERIC_SUCCESS_TAKE_TIME));
        gconf->post_log(u8"<@" + std::to_string(user_action) + "> -> APPLY done.");
        __apply_commands_at_guild(core, src.command.guild_id, nullptr);
    }
        return;
    case lang_command::CONFIG_LOGS:
    {
        std::string gen_me;

        for(size_t pos = 0; pos < gconf->get_log_size(); pos++) {
            gen_me += (gen_me.size() ? "\n" : "") + gconf->get_log(pos);
            if (gen_me.size() > 1500) break;
        }

        if (gen_me.size()) src.edit_response(lang->get(lang_line::COMMAND_CONFIG_LOG_LOGS_TITLE) + "\n\n" + gen_me);
        else src.edit_response(lang->get(lang_line::COMMAND_GENERIC_CANT_FIND));
    }
        return;
    case lang_command::CONFIG_EXTERNAL:
    {
        const auto _arg_name = get_first_name(options);

        switch(lang->command_from(_arg_name.copy_zero(), external_commands)) {
        case lang_command::CONFIG_EXTERNAL_CANPASTE:
        {
            const auto allow = get_bool_in_command(suboptions, lang->get(lang_command::CONFIG_EXTERNAL_CANPASTE_ALLOW));

            if (!allow) { src.edit_response(replaceargformatdirect(lang->get(lang_line::COMMAND_GENERIC_INVALID_ARGS_WITH_NAME), {lang->get(lang_command::CONFIG_EXTERNAL_CANPASTE_ALLOW)})); return; }

            gconf->post_log(u8"<@" + std::to_string(user_action) + "> -> EXTERNAL CANPASTE was set to '" + (*allow ? "TRUE" : "FALSE") + "'");
            gconf->set_can_paste_external_content(*allow);
            src.edit_response(lang->get(lang_line::GENERIC_BOT_SUCCESS));
        }
            break;
        default:
            src.edit_response(lang->get(lang_line::COMMAND_GENERIC_INVALID_ARGS));
            break;
        }
    }
        return;
    case lang_command::CONFIG_LANGUAGE:
    {
        const auto _arg_string = get_str_in_command(options, lang->get(lang_command::CONFIG_LANGUAGE_STRING));

        if (!_arg_string.is_null()){
            if (_arg_string != gconf->get_language()) {
                gconf->set_language(_arg_string.get());
                src.edit_response(replaceargformatdirect(lang->get(lang_line::COMMAND_CONFIG_LANGUAGE_SUCCESS_TAKE_TIME_WITH_NAME), {*_arg_string}));
                __apply_commands_at_guild(core, src.command.guild_id, nullptr);
            }
            else src.edit_response(replaceargformatdirect(lang->get(lang_line::COMMAND_GENERIC_ALREADY_SET_WITH_NAME), {*_arg_string}));

            gconf->post_log(u8"<@" + std::to_string(user_action) + "> -> LANGUAGE updated to '" + _arg_string.get() + "'");

            return;
        }
        else { src.edit_response(lang->get(lang_line::COMMAND_GENERIC_INVALID_ARGS)); return; }
    }
        return;
    case lang_command::CONFIG_ROLES:
    {
        const auto _arg_name = get_first_name(options);

        switch(lang->command_from(_arg_name.copy_zero(), roles_commands)) {
        case lang_command::CONFIG_ROLES_ADD:
        {
            const auto category = get_str_in_command(suboptions, lang->get(lang_command::CONFIG_ROLES_ADD_CATEGORY));
            const auto role = get_mull_in_command(suboptions, lang->get(lang_command::CONFIG_ROLES_ADD_ROLE));
            const auto name = get_str_in_command(suboptions, lang->get(lang_command::CONFIG_ROLES_ADD_NAME));

            if (!category) { src.edit_response(replaceargformatdirect(lang->get(lang_line::COMMAND_GENERIC_INVALID_ARGS_WITH_NAME), {lang->get(lang_command::CONFIG_ROLES_ADD_CATEGORY)})); return; }
            if (!role) { src.edit_response(replaceargformatdirect(lang->get(lang_line::COMMAND_GENERIC_INVALID_ARGS_WITH_NAME), {lang->get(lang_command::CONFIG_ROLES_ADD_ROLE)})); return; }
            if (!name) { src.edit_response(replaceargformatdirect(lang->get(lang_line::COMMAND_GENERIC_INVALID_ARGS_WITH_NAME), {lang->get(lang_command::CONFIG_ROLES_ADD_NAME)})); return; }

            auto& editroles = gconf->get_roles_map();

            gconf->post_log(u8"<@" + std::to_string(user_action) + "> -> ROLES ADD command call -> CATEGORY='" + category.get() + "';ROLE='" + std::to_string(role.get()) + "';NAME='" + name.get() + "'");

            auto it = std::find_if(editroles.begin(), editroles.end(), [&](const guild_data::category& a){ return fix_name_for_cmd(a.name) == fix_name_for_cmd(category.get()); });
            if (it == editroles.end()) {
                guild_data::category cat;
                cat.name = category.get();
                editroles.push_back(cat);
                it = --editroles.end();
            }
            guild_data::category& ref = *it;

            for(auto& inn : ref.list){
                if (role == inn.id){
                    src.edit_response(replaceargformatdirect(lang->get(lang_line::COMMAND_GENERIC_FOUND_SWITCH_FROM_TO_NAMES), {inn.name, name.get()}));
                    inn.name = name.get();
                    return;
                }
            }

            ref.list.push_back(guild_data::pair_id_name{role.get(), name.get()});
            src.edit_response(replaceargformatdirect(lang->get(lang_line::COMMAND_GENERIC_SUCCESS_ADD_WITH_NAME), {name.get()}));
        }
            break;
        case lang_command::CONFIG_ROLES_REMOVE:
        {
            const auto category = get_str_in_command(suboptions, lang->get(lang_command::CONFIG_ROLES_ADD_CATEGORY));
            const auto role = get_mull_in_command(suboptions, lang->get(lang_command::CONFIG_ROLES_ADD_ROLE));

            if (!category && !role) { src.edit_response(lang->get(lang_line::COMMAND_GENERIC_INVALID_ARGS)); return; }

            auto& editroles = gconf->get_roles_map();

            gconf->post_log(u8"<@" + std::to_string(user_action) + "> -> ROLES REMOVE command call -> CATEGORY='" + category.get() + "';ROLE='" + std::to_string(role.get()) + "'");

            if (role != 0){
                if (!category->empty()) {
                    bool got_one = find_and_do(editroles, 
                        [&](const guild_data::category& a){ return fix_name_for_cmd(a.name) == fix_name_for_cmd(*category); },
                        [&](std::vector<guild_data::category>::iterator it){
                            return find_and_do(it->list, 
                                [&](const guild_data::pair_id_name& a){ return role == a.id;},
                                [&](std::vector<guild_data::pair_id_name>::iterator it2){
                                    it->list.erase(it2);
                                    src.edit_response(replaceargformatdirect(lang->get(lang_line::COMMAND_GENERIC_SUCCESS_REMOVE_WITH_NAME), {it->name}));
                                        //u8"Um cargo removido com sucesso da categoria '" + it->name + "'! Use o comando de aplicar quando terminar tudo.");
                                    return true;
                                }
                            );
                        }
                    );

                    if (!got_one){ src.edit_response(replaceargformatdirect(lang->get(lang_line::COMMAND_GENERIC_CANT_FIND_WITH_NAME), {*category})); return; }
                    return;
                }
                else { // roleid full search
                    bool got_one = find_and_do(editroles, 
                        [&](const guild_data::category& a){ return true; }, // any
                        [&](std::vector<guild_data::category>::iterator it){
                            return find_and_do(it->list, 
                                [&](const guild_data::pair_id_name& a){ return role == a.id;},
                                [&](std::vector<guild_data::pair_id_name>::iterator it2){
                                    it->list.erase(it2);
                                    src.edit_response(replaceargformatdirect(lang->get(lang_line::COMMAND_GENERIC_SUCCESS_REMOVE_WITH_NAME), {it->name}));
                                        //u8"Encontrei o cargo na categoria '" + it->name + "'. Ele foi removido com sucesso! Use o comando de aplicar quando terminar tudo.");
                                    return true;
                                }
                            );
                        }
                    );

                    if (!got_one){ src.edit_response(replaceargformatdirect(lang->get(lang_line::COMMAND_GENERIC_CANT_FIND_WITH_NAME), {std::to_string(*role)})); return; }
                    return;
                }
            }
            else { // category delete
                bool got_one = find_and_do(editroles, 
                    [&](const guild_data::category& a){ return fix_name_for_cmd(a.name) == fix_name_for_cmd(*category); },
                    [&](std::vector<guild_data::category>::iterator it){                        
                        editroles.erase(it);
                        src.edit_response(replaceargformatdirect(lang->get(lang_line::COMMAND_GENERIC_SUCCESS_REMOVE_WITH_NAME), {it->name}));
                            //u8"Categoria '" + it->name + "' removida com successo! Use o comando de aplicar quando terminar tudo.");
                        return true;
                    }
                );

                if (!got_one){ src.edit_response(replaceargformatdirect(lang->get(lang_line::COMMAND_GENERIC_CANT_FIND_WITH_NAME), {*category})); return; }
                return;
            }
        }
            break;
        case lang_command::CONFIG_ROLES_CLEANUP:
        {
            gconf->get_roles_map().clear();
            gconf->post_log(u8"<@" + std::to_string(user_action) + "> -> ROLES CLEAR command call.");
            src.edit_response(lang->get(lang_line::GENERIC_BOT_SUCCESS));
        }
            break;
        case lang_command::CONFIG_ROLES_COMBO:
        {
            const auto category = get_str_in_command(suboptions, lang->get(lang_command::CONFIG_ROLES_COMBO_CATEGORY));
            const auto combinable = get_bool_in_command(suboptions, lang->get(lang_command::CONFIG_ROLES_COMBO_COMBINABLE));

            if (!category) { src.edit_response(replaceargformatdirect(lang->get(lang_line::COMMAND_GENERIC_INVALID_ARGS_WITH_NAME), {lang->get(lang_command::CONFIG_ROLES_COMBO_CATEGORY)})); return; }
            if (!combinable) { src.edit_response(replaceargformatdirect(lang->get(lang_line::COMMAND_GENERIC_INVALID_ARGS_WITH_NAME), {lang->get(lang_command::CONFIG_ROLES_COMBO_COMBINABLE)})); return; }

            auto& editroles = gconf->get_roles_map();

            gconf->post_log(u8"<@" + std::to_string(user_action) + "> -> ROLES COMBO command call -> CATEGORY='" + category.get() + "';COMBINABLE='" + (combinable.get() ? "TRUE" : "FALSE") + "'");

            bool got_one = find_and_do(editroles, 
                [&](const guild_data::category& a){ return fix_name_for_cmd(a.name) == fix_name_for_cmd(*category); },
                [&](std::vector<guild_data::category>::iterator it){                        
                    it->can_combine = *combinable;
                    src.edit_response(lang->get(lang_line::GENERIC_BOT_SUCCESS));
                    return true;
                }
            );

            if (!got_one){ src.edit_response(replaceargformatdirect(lang->get(lang_line::COMMAND_GENERIC_CANT_FIND_WITH_NAME), {*category})); return; }
            return;            
        }
            break;
        default:
            src.edit_response(lang->get(lang_line::COMMAND_GENERIC_INVALID_ARGS));
            break;
        }
    }
        return;
    case lang_command::CONFIG_POINTS:
    {
        const auto who = get_mull_in_command(options, lang->get(lang_command::CONFIG_POINTS_WHO));
        const auto value = get_int_in_command(options, lang->get(lang_command::CONFIG_POINTS_VALUE));

        if (!who) { src.edit_response(replaceargformatdirect(lang->get(lang_line::COMMAND_GENERIC_INVALID_ARGS_WITH_NAME), {lang->get(lang_command::CONFIG_POINTS_WHO)})); return; }
        if (!value || value.get() < 0) { src.edit_response(replaceargformatdirect(lang->get(lang_line::COMMAND_GENERIC_INVALID_ARGS_WITH_NAME), {lang->get(lang_command::CONFIG_POINTS_VALUE)})); return; }

        auto uconf = get_user_config(who.get());

        uconf->set_points_at_guild(src.command.guild_id, value.get());

        src.edit_response(lang->get(lang_line::GENERIC_BOT_SUCCESS));
    }
        return;
    case lang_command::CONFIG_ADMIN:
    {
        const auto _arg_name = get_first_name(options);

        switch(lang->command_from(_arg_name.copy_zero(), admin_commands)) {
        case lang_command::CONFIG_ADMIN_ADD:
        {
            const auto role = get_mull_in_command(suboptions, lang->get(lang_command::CONFIG_ADMIN_COMMON_ROLE));

            if (!role) { src.edit_response(replaceargformatdirect(lang->get(lang_line::COMMAND_GENERIC_INVALID_ARGS_WITH_NAME), {lang->get(lang_command::CONFIG_ADMIN_COMMON_ROLE)})); return; }
            
            gconf->add_role_admin(*role);
            src.edit_response(lang->get(lang_line::GENERIC_BOT_SUCCESS));
        }
            break;
        case lang_command::CONFIG_ADMIN_REMOVE:
        {
            const auto role = get_mull_in_command(suboptions, lang->get(lang_command::CONFIG_ADMIN_COMMON_ROLE));
            
            if (!role) { src.edit_response(replaceargformatdirect(lang->get(lang_line::COMMAND_GENERIC_INVALID_ARGS_WITH_NAME), {lang->get(lang_command::CONFIG_ADMIN_COMMON_ROLE)})); return; }
            
            gconf->remove_role_admin(*role);
            src.edit_response(lang->get(lang_line::GENERIC_BOT_SUCCESS));
        }
            break;
        case lang_command::CONFIG_ADMIN_VERIFY:
        {
            const auto role = get_mull_in_command(suboptions, lang->get(lang_command::CONFIG_ADMIN_COMMON_ROLE));
            
            if (!role) { src.edit_response(replaceargformatdirect(lang->get(lang_line::COMMAND_GENERIC_INVALID_ARGS_WITH_NAME), {lang->get(lang_command::CONFIG_ADMIN_COMMON_ROLE)})); return; }
            
            if (gconf->check_role_admin(*role)) {
                src.edit_response(replaceargformatdirect(lang->get(lang_line::COMMAND_GENERIC_IN_LIST_TRUE_WITH_NAME), { "<@&" + std::to_string(*role) + ">" }));
            }
            else {
                src.edit_response(replaceargformatdirect(lang->get(lang_line::COMMAND_GENERIC_IN_LIST_FALSE_WITH_NAME), { "<@&" + std::to_string(*role) + ">" }));
            }
        }
            break;
        default:
            src.edit_response(lang->get(lang_line::COMMAND_GENERIC_INVALID_ARGS));
            break;
        }
    }
        return;
    case lang_command::CONFIG_AUTOROLE:
    {
        const auto _arg_name = get_first_name(options);

        switch(lang->command_from(_arg_name.copy_zero(), autorole_commands)) {
        case lang_command::CONFIG_AUTOROLE_ADD:
        {
            const auto role = get_mull_in_command(suboptions, lang->get(lang_command::CONFIG_AUTOROLE_COMMON_ROLE));

            if (!role) { src.edit_response(replaceargformatdirect(lang->get(lang_line::COMMAND_GENERIC_INVALID_ARGS_WITH_NAME), {lang->get(lang_command::CONFIG_AUTOROLE_COMMON_ROLE)})); return; }
            
            gconf->add_role_joined(*role);
            src.edit_response(lang->get(lang_line::GENERIC_BOT_SUCCESS));
        }
            break;
        case lang_command::CONFIG_AUTOROLE_REMOVE:
        {
            const auto role = get_mull_in_command(suboptions, lang->get(lang_command::CONFIG_AUTOROLE_COMMON_ROLE));
            
            if (!role) { src.edit_response(replaceargformatdirect(lang->get(lang_line::COMMAND_GENERIC_INVALID_ARGS_WITH_NAME), {lang->get(lang_command::CONFIG_AUTOROLE_COMMON_ROLE)})); return; }
            
            gconf->remove_role_joined(*role);
            src.edit_response(lang->get(lang_line::GENERIC_BOT_SUCCESS));
        }
            break;
        case lang_command::CONFIG_AUTOROLE_VERIFY:
        {
            const auto role = get_mull_in_command(suboptions, lang->get(lang_command::CONFIG_AUTOROLE_COMMON_ROLE));
            
            if (!role) { src.edit_response(replaceargformatdirect(lang->get(lang_line::COMMAND_GENERIC_INVALID_ARGS_WITH_NAME), {lang->get(lang_command::CONFIG_AUTOROLE_COMMON_ROLE)})); return; }
            
            if (gconf->check_role_joined(*role)) {
                src.edit_response(replaceargformatdirect(lang->get(lang_line::COMMAND_GENERIC_IN_LIST_TRUE_WITH_NAME), { "<@&" + std::to_string(*role) + ">" }));
            }
            else {
                src.edit_response(replaceargformatdirect(lang->get(lang_line::COMMAND_GENERIC_IN_LIST_FALSE_WITH_NAME), { "<@&" + std::to_string(*role) + ">" }));
            }
        }
            break;
        default:
            src.edit_response(lang->get(lang_line::COMMAND_GENERIC_INVALID_ARGS));
            break;
        }
    }
        return;
    case lang_command::CONFIG_LEVELS:
    {        
        const auto _arg_name = get_first_name(options);

        switch(lang->command_from(_arg_name.copy_zero(), levels_commands)) {
        case lang_command::CONFIG_LEVELS_ADD:
        {
            const auto role = get_mull_in_command(suboptions, lang->get(lang_command::CONFIG_LEVELS_ADD_ROLE));
            const auto level = get_mull_in_command(suboptions, lang->get(lang_command::CONFIG_LEVELS_ADD_LEVEL));
            
            if (!role) { src.edit_response(replaceargformatdirect(lang->get(lang_line::COMMAND_GENERIC_INVALID_ARGS_WITH_NAME), {lang->get(lang_command::CONFIG_LEVELS_ADD_ROLE)})); return; }
            if (!level || level.get() < 0) { src.edit_response(replaceargformatdirect(lang->get(lang_line::COMMAND_GENERIC_INVALID_ARGS_WITH_NAME), {lang->get(lang_command::CONFIG_LEVELS_ADD_LEVEL)})); return; }

            gconf->add_role_level_map(guild_data::pair_id_level{role.get(), level.get()});

            src.edit_response(lang->get(lang_line::GENERIC_BOT_SUCCESS));
        }
            break;
        case lang_command::CONFIG_LEVELS_REMOVE:
        {
            const auto role = get_mull_in_command(suboptions, lang->get(lang_command::CONFIG_LEVELS_REMOVE_ROLE));
            const auto level = get_mull_in_command(suboptions, lang->get(lang_command::CONFIG_LEVELS_REMOVE_LEVEL));
            
            if (!role) { src.edit_response(replaceargformatdirect(lang->get(lang_line::COMMAND_GENERIC_INVALID_ARGS_WITH_NAME), {lang->get(lang_command::CONFIG_LEVELS_REMOVE_ROLE)})); return; }
            if (!level || level.get() < 0) { src.edit_response(replaceargformatdirect(lang->get(lang_line::COMMAND_GENERIC_INVALID_ARGS_WITH_NAME), {lang->get(lang_command::CONFIG_LEVELS_REMOVE_LEVEL)})); return; }

            if (level != 0) {
                const auto& vecref = gconf->get_roles_per_level_map();
                for(const auto& inn : vecref) {
                    if (level == inn.level) {
                        gconf->remove_role_level_map(inn.id);
                        src.edit_response(lang->get(lang_line::GENERIC_BOT_SUCCESS));
                        return;
                    }
                }
                src.edit_response(lang->get(lang_line::COMMAND_GENERIC_CANT_FIND));
                return;
            }
            else {
                gconf->remove_role_level_map(*role);
                src.edit_response(lang->get(lang_line::GENERIC_BOT_SUCCESS));
                return;
            }
        }
            break;
        case lang_command::CONFIG_LEVELS_REDIRECT:
        {
            const auto channel = get_mull_in_command(suboptions, lang->get(lang_command::CONFIG_LEVELS_REDIRECT_CHANNELID));

            if (channel.is_null()){
                gconf->set_level_channel_id(0);
                src.edit_response(lang->get(lang_line::GENERIC_BOT_SUCCESS_ON_RESET));
                return;
            }
            else {
                gconf->set_level_channel_id(*channel);
                src.edit_response(replaceargformatdirect(lang->get(lang_line::COMMAND_GENERIC_SUCCESS_SETTING_WITH_NAME),{"<#" + std::to_string(*channel) + ">"}));
                    //u8"Configurado, o chat <#" + std::to_string(*channel) + "> é agora o chat de mensagens de level up.");
                return;
            }
        }
            break;
        case lang_command::CONFIG_LEVELS_MESSAGES:
        {
            const auto block = get_bool_in_command(suboptions, lang->get(lang_command::CONFIG_LEVELS_MESSAGES_BLOCK));

            if (!block) { src.edit_response(replaceargformatdirect(lang->get(lang_line::COMMAND_GENERIC_INVALID_ARGS_WITH_NAME), {lang->get(lang_command::CONFIG_LEVELS_MESSAGES_BLOCK)})); return; }

            gconf->set_message_level_blocked(*block);

            src.edit_response(lang->get(*block ? lang_line::COMMAND_CONFIG_LEVELS_BLOCKING_TRUE : lang_line::COMMAND_CONFIG_LEVELS_BLOCKING_FALSE));
        }
            break;
        default:
            src.edit_response(lang->get(lang_line::COMMAND_GENERIC_INVALID_ARGS));
            break;
        }
    }
        return;
    default:
        break;
    }
    src.edit_response(lang->get(lang_line::GENERIC_BOT_FAILED_DOING_TASK));
}