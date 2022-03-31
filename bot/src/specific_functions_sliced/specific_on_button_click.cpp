#include <specific_functions.hpp>

void g_on_button_click(const dpp::button_click_t& ev)
{
    transl_button_event wrk(ev);

    const auto guil = tf_guild_info[ev.command.guild_id];
    if (!guil) { ev.reply(make_ephemeral_message("Something went wrong! Guild do not exist?! Please report error! I'm so sorry.")); return; }

    if (!wrk.get_trigger().target_if_button) { ev.reply(make_ephemeral_message("Something went wrong! Internal reference got LOST!")); return; }

    auto& trigg = wrk.get_trigger();

    if (trigg.group_name == "selfconf") {
        const auto you = tf_user_info[ev.command.usr.id];
        if (!you) {
            ev.reply(make_ephemeral_message("Something went wrong! You do not exist?! Please report error! I'm so sorry."));
            return;
        }
        
        if (trigg.item_name == "ptspublic") {
            trigg.target_if_button->extra.style = 
                (trigg.target_if_button->extra.style == dpp::cos_success) ? dpp::cos_danger : dpp::cos_success;
            you->show_level_up_messages = trigg.target_if_button->extra.style == dpp::cos_success;
            wrk.reply(!guil->commands_public);
            return;
        }
        if (trigg.item_name == "setcolor") {
            auto modal = modal_generate("selfconf", "colorpicker", "Select color");
            modal_add_component(modal, "Color name, hex or decimal", "color", "red, green, cyan, white, 0xFFFFFF...", dpp::text_style_type::text_short);
            ev.dialog(modal, error_autoprint);
            return;
        }
        if (trigg.item_name == "getdata") {
            wrk.set_content("**All your data stored**");
            wrk.reply(!guil->commands_public, true, [&](dpp::message& msg) { msg.add_file("user_data.json", you->to_json().dump(2)); });
            return;
        }

        ev.reply(make_ephemeral_message("Something went wrong! Track: on_button_click > selfconf > ?"));
        return;
    }
    if (trigg.group_name == "TMPcommconf") {

        const auto guil = tf_guild_info[ev.command.guild_id];
        if (!guil) { ev.reply(make_ephemeral_message("Something went wrong! Guild do not exist?! Please report error! I'm so sorry.")); return; }

        if (trigg.item_name == "ext") {
            trigg.target_if_button->extra.style = 
                (trigg.target_if_button->extra.style == dpp::cos_success) ? dpp::cos_danger : dpp::cos_success;
            guil->allow_external_paste = trigg.target_if_button->extra.style == dpp::cos_success;
            wrk.reply(!guil->commands_public);
            return;
        }
        if (trigg.item_name == "pub") {
            trigg.target_if_button->extra.style =
                (trigg.target_if_button->extra.style == dpp::cos_success) ? dpp::cos_danger : dpp::cos_success;
            guil->commands_public = trigg.target_if_button->extra.style == dpp::cos_success;
            wrk.reply(!guil->commands_public);
            return;
        }

        ev.reply(make_ephemeral_message("Something went wrong! Track: on_button_click > TMPcommconf > ?"));
        return;
    }
    if (trigg.group_name == "TMPpointsconf") {
        if (trigg.item_name == "select") {
            auto modal = modal_generate("pointsconf", "select", "Select user");
            modal_add_component(modal, "User ID (number)", "userid", "012345...", dpp::text_style_type::text_short);
            ev.dialog(modal, error_autoprint);
            return;
        }
        if (trigg.item_name == "setpts") {
            auto modal = modal_generate("pointsconf", "setpts", "Set user points in guild");
            modal_add_component(modal, "Points (number, positive)", "points", "121314...", dpp::text_style_type::text_short);
            ev.dialog(modal, error_autoprint);
            return;
        }

        ev.reply(make_ephemeral_message("Something went wrong! Track: on_button_click > TMPpointsconf > ?"));
        return;
    }
    if (trigg.group_name == "TMProlesconf") { // GROUP
        if (trigg.item_name == "addg") {
            auto modal = modal_generate("roles_roles", "addg", "Add / update role group");
            modal_add_component(modal, "Group name", "name", "Fancy group", dpp::text_style_type::text_short, true, 0, 20);
            modal_add_component(modal, "One role at a time or any combination?", "limit", "\"ONE\" for one, \"ANY\" for any", dpp::text_style_type::text_short);
            ev.dialog(modal, error_autoprint);
            return;
        }
        if (trigg.item_name == "delg") {
            auto modal = modal_generate("roles_roles", "delg", "Remove role group");
            modal_add_component(modal, "Role group name", "name", "* deletes everything, else perfect match", dpp::text_style_type::text_short);
            ev.dialog(modal, error_autoprint);
            return;
        }
        if (trigg.item_name == "add") {
            auto modal = modal_generate("roles_roles", "add", "Add / update item");
            modal_add_component(modal, "Role ID", "role", "012345...", dpp::text_style_type::text_short, true);
            modal_add_component(modal, "Label", "name", "Crazy role", dpp::text_style_type::text_short, true, 0, 20);
            ev.dialog(modal, error_autoprint);
            return;
        }
        if (trigg.item_name == "del") {
            auto modal = modal_generate("roles_roles", "del", "Remove item");
            modal_add_component(modal, "Role ID", "role", "012345... (* for all)", dpp::text_style_type::text_short, true);
            ev.dialog(modal, error_autoprint);
            return;
        }

        ev.reply(make_ephemeral_message("Something went wrong! Track: on_button_click > TMProlesconf > ?"));
        return;
    }
    if (trigg.group_name == "TMParolesconf") { // AUTOMATIC
        if (trigg.item_name == "add") {
            auto modal = modal_generate("roles_aroles", "add", "Add role");
            modal_add_component(modal, "Role ID", "role", "012345...", dpp::text_style_type::text_short, true);
            ev.dialog(modal, error_autoprint);
            return;
        }
        if (trigg.item_name == "del") {
            auto modal = modal_generate("roles_aroles", "del", "Remove role");
            modal_add_component(modal, "Role ID", "role", "012345... (* for all)", dpp::text_style_type::text_short, true);
            ev.dialog(modal, error_autoprint);
            return;
        }

        ev.reply(make_ephemeral_message("Something went wrong! Track: on_button_click > TMParolesconf > ?"));
        return;
    }
    if (trigg.group_name == "TMPlrolesconf") { // LEVELING
        if (trigg.item_name == "add") {
            auto modal = modal_generate("roles_lroles", "add", "Add / update role rule");
            modal_add_component(modal, "Role ID", "role", "012345...", dpp::text_style_type::text_short, true);
            modal_add_component(modal, "Give/take on what level?", "level", "[1, inf)", dpp::text_style_type::text_short, true, 0, 4);
            ev.dialog(modal, error_autoprint);
            return;
        }
        if (trigg.item_name == "del") {
            auto modal = modal_generate("roles_lroles", "del", "Remove role rule");
            modal_add_component(modal, "Role ID", "role", "012345... (* for all)", dpp::text_style_type::text_short, true);
            ev.dialog(modal, error_autoprint);
            return;
        }

        ev.reply(make_ephemeral_message("Something went wrong! Track: on_button_click > TMPlrolesconf > ?"));
        return;
    }

    ev.reply(make_ephemeral_message("Something went wrong! Track: on_button_click > ?"));
    return;










    if (ev.custom_id == "user-show_level_up_messages")
    {
        const auto you = tf_user_info[ev.command.usr.id];
        if (!you) {
            ev.reply(make_ephemeral_message("Something went wrong! You do not exist?! Please report error! I'm so sorry."));
            return;
        }

        auto_handle_button_switch(ev, ev.custom_id, [&](dpp::component& it){
            you->show_level_up_messages = !you->show_level_up_messages;
            set_boolean_button(you->show_level_up_messages, it);
        });
    }
    else if (ev.custom_id == "user-pref_color")
    {
        dpp::interaction_modal_response modal("user-pref_color", "Select color");
        modal.add_component(
            dpp::component()
                .set_label("What color best describes you?")
                .set_id("color")
                .set_type(dpp::cot_text)
                .set_required(true)
                .set_placeholder("red, green, blue, ..., black, default, 0xHEX or DECIMAL")
                .set_min_length(1)
                .set_max_length(20)
                .set_text_style(dpp::text_short)
        );
        ev.dialog(modal, error_autoprint);
    }
    else if (ev.custom_id == "user-download_user_data")
    {
        const auto you = tf_user_info[ev.command.usr.id];
        if (!you) {
            ev.reply(make_ephemeral_message("Something went wrong! You do not exist?! Please report error! I'm so sorry."));
            return;
        }

        dpp::message msg;
        msg.set_content("Your user data:");
        msg.add_file("user_data.json", you->to_json().dump(2));
        msg.set_flags(64);
        ev.reply(msg, error_autoprint);
    }
    else if (ev.custom_id == "guildconf-paste-switch")
    {
        const auto guil = tf_guild_info[ev.command.guild_id];
        if (!guil) {
            ev.reply(make_ephemeral_message("Something went wrong! Guild do not exist?! Please report error! I'm so sorry."));
            return;
        }

        auto_handle_button_switch(ev, ev.custom_id, [&](dpp::component& it){
            guil->allow_external_paste = !guil->allow_external_paste;
            set_boolean_button(guil->allow_external_paste, it);
        });
    }
    else if (ev.custom_id == "guildconf-member_points-select_userid")
    {        
        dpp::interaction_modal_response modal(ev.custom_id, "Enter user ID");
        modal.add_component(
            dpp::component()
                .set_label("Enter a numeric user ID")
                .set_id("number")
                .set_type(dpp::cot_text)
                .set_required(true)
                .set_placeholder("0123456789...")
                .set_min_length(1)
                .set_max_length(20)
                .set_text_style(dpp::text_short)
        );
        ev.dialog(modal, error_autoprint);
    }
    else if (ev.custom_id == "guildconf-member_points-select_userpts")
    {
        dpp::interaction_modal_response modal(ev.custom_id, "Change user local points");
        modal.add_component(
            dpp::component()
                .set_label("Set this user points on this guild")
                .set_id("number")
                .set_type(dpp::cot_text)
                .set_required(true)
                .set_placeholder("A number")
                .set_min_length(1)
                .set_max_length(20)
                .set_text_style(dpp::text_short)
        );
        ev.dialog(modal, error_autoprint);
    }
    else if (ev.custom_id == "guildconf-auto_roles-add")
    {
        dpp::interaction_modal_response modal(ev.custom_id, "Add one or more roles to list");
        modal.add_component(
            dpp::component()
                .set_label("Paste IDs here")
                .set_id("paragraph")
                .set_type(dpp::cot_text)
                .set_required(true)
                .set_placeholder("123456...")
                .set_min_length(1)
                .set_max_length(512)
                .set_text_style(dpp::text_paragraph)
        );
        ev.dialog(modal, error_autoprint);
    }
    else if (ev.custom_id == "guildconf-auto_roles-del")
    {
        dpp::interaction_modal_response modal(ev.custom_id, "Remove one or more roles from list");
        modal.add_component(
            dpp::component()
                .set_label("Paste IDs or * to delete all.")
                .set_id("paragraph")
                .set_type(dpp::cot_text)
                .set_required(true)
                .set_placeholder("123456...")
                .set_min_length(1)
                .set_max_length(512)
                .set_text_style(dpp::text_paragraph)
        );
        ev.dialog(modal, error_autoprint);
    }
    else if (ev.custom_id == "guildconf-roles_command-addgroup")
    {
        dpp::interaction_modal_response modal(ev.custom_id, "Create new role group");
        modal.add_component(
            dpp::component()
                .set_label("Give it a name")
                .set_id("name")
                .set_type(dpp::cot_text)
                .set_required(true)
                .set_placeholder("Funky group")
                .set_min_length(1)
                .set_max_length(20)
                .set_text_style(dpp::text_short)
        );
        ev.dialog(modal, error_autoprint);
    }
    else if (ev.custom_id == "guildconf-roles_command-delgroup")
    {
        dpp::interaction_modal_response modal(ev.custom_id, "Remove one role group");
        modal.add_component(
            dpp::component()
                .set_label("Tell me its name")
                .set_id("name")
                .set_type(dpp::cot_text)
                .set_required(true)
                .set_placeholder("Funky group")
                .set_min_length(1)
                .set_max_length(20)
                .set_text_style(dpp::text_short)
        );
        ev.dialog(modal, error_autoprint);
    }
    else if (ev.custom_id == "guildconf-roles_command-add")
    {
        dpp::interaction_modal_response modal(ev.custom_id, "Add role to list");
        modal.add_component(
            dpp::component()
                .set_label("Role ID")
                .set_id("roleid")
                .set_type(dpp::cot_text)
                .set_required(true)
                .set_placeholder("123456...")
                .set_min_length(1)
                .set_max_length(20)
                .set_text_style(dpp::text_short)
        )
        .add_row()
        .add_component(
            dpp::component()
                .set_label("Name to show")
                .set_id("name")
                .set_type(dpp::cot_text)
                .set_required(true)
                .set_placeholder("My cool role")
                .set_min_length(1)
                .set_max_length(40)
                .set_text_style(dpp::text_short)
        );
        ev.dialog(modal, error_autoprint);
    }
    else if (ev.custom_id == "guildconf-roles_command-del")
    {
        dpp::interaction_modal_response modal(ev.custom_id, "Remove role from list");
        modal.add_component(
            dpp::component()
                .set_label("Paste ID or * to delete all.")
                .set_id("roleid")
                .set_type(dpp::cot_text)
                .set_required(true)
                .set_placeholder("123456...")
                .set_min_length(1)
                .set_max_length(20)
                .set_text_style(dpp::text_short)
        );
        ev.dialog(modal, error_autoprint);
    }
    else {
        ev.reply(dpp::ir_update_message, "This interaction is unknown. Please try again later!", error_autoprint);
    }
}