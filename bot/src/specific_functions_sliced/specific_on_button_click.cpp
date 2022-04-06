#include <specific_functions.hpp>
// SHOULD BE SAFE (MUTEXES)

void g_on_button_click(const dpp::button_click_t& ev)
{
    if (ev.command.usr.id != ev.command.msg.interaction.usr.id && ev.command.msg.interaction.usr.id != 0) {
        ev.reply(make_ephemeral_message("If you want to use this command, please trigger it yourself. This is not yours.")); 
        return;
    }

    transl_button_event wrk(ev);

    force_const<guild_info> guil = tf_guild_info[ev.command.guild_id];
    if (!guil) { ev.reply(make_ephemeral_message("Something went wrong! Guild do not exist?! Please report error! I'm so sorry.")); return; }

    std::shared_lock<std::shared_mutex> guilmtx(guil.unsafe().muu);

    if (!wrk.get_trigger().target_if_button) { ev.reply(make_ephemeral_message("Something went wrong! Internal reference got LOST!")); return; }

    auto& trigg = wrk.get_trigger();

    if (trigg.group_name == "selfconf") {
        force_const<user_info> you = tf_user_info[ev.command.usr.id];
        if (!you) { ev.reply(make_ephemeral_message("Something went wrong! You do not exist?! Please report error! I'm so sorry.")); return; }
        
        std::unique_lock<std::shared_mutex> lu(you.unsafe().muu);

        if (trigg.item_name == "ptspublic") {
            trigg.target_if_button->extra.style = 
                (trigg.target_if_button->extra.style == dpp::cos_success) ? dpp::cos_danger : dpp::cos_success;
            you.unsafe().show_level_up_messages = trigg.target_if_button->extra.style == dpp::cos_success;
            wrk.reply(!guil->commands_public);
            return;
        }
        if (trigg.item_name == "setcolor") {
            auto modal = modal_generate("selfconf", "colorpicker", "Select color");
            modal_add_component(modal, "Color name, hex or decimal", "color", "red, green, cyan, white, 0xFFFFFF...", dpp::text_style_type::text_short, true);
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

        guilmtx.unlock();
        std::unique_lock<std::shared_mutex> gl(guil.unsafe().muu);

        if (trigg.item_name == "ext") {
            trigg.target_if_button->extra.style = 
                (trigg.target_if_button->extra.style == dpp::cos_success) ? dpp::cos_danger : dpp::cos_success;
            guil.unsafe().allow_external_paste = trigg.target_if_button->extra.style == dpp::cos_success;
            wrk.reply(!guil->commands_public);
            return;
        }
        if (trigg.item_name == "pub") {
            trigg.target_if_button->extra.style =
                (trigg.target_if_button->extra.style == dpp::cos_success) ? dpp::cos_danger : dpp::cos_success;
            guil.unsafe().commands_public = trigg.target_if_button->extra.style == dpp::cos_success;
            wrk.reply(!guil->commands_public);
            return;
        }
        if (trigg.item_name == "levelmsg") {
            trigg.target_if_button->extra.style =
                (trigg.target_if_button->extra.style == dpp::cos_success) ? dpp::cos_danger : dpp::cos_success;
            guil.unsafe().block_levelup_user_event = trigg.target_if_button->extra.style != dpp::cos_success;
            wrk.reply(!guil->commands_public);
            return;
        }
        if (trigg.item_name == "levelch") {
            auto modal = modal_generate("commconf", "levelch", "Select channel");
            modal_add_component(modal, "Channel ID or 'NONE'", "chid", "012345... or NONE", dpp::text_style_type::text_short, true);
            ev.dialog(modal, error_autoprint);
            return;
        }

        ev.reply(make_ephemeral_message("Something went wrong! Track: on_button_click > TMPcommconf > ?"));
        return;
    }
    if (trigg.group_name == "TMPpointsconf") {
        if (trigg.item_name == "select") {
            auto modal = modal_generate("pointsconf", "select", "Select user");
            modal_add_component(modal, "User ID (number)", "userid", "012345...", dpp::text_style_type::text_short, true);
            ev.dialog(modal, error_autoprint);
            return;
        }
        if (trigg.item_name == "setpts") {
            auto modal = modal_generate("pointsconf", "setpts", "Set user points in guild");
            modal_add_component(modal, "Points (number, positive)", "points", "121314...", dpp::text_style_type::text_short, true);
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
            modal_add_component(modal, "One role at a time or any combination?", "limit", "\"ONE\" for one, \"ANY\" for any", dpp::text_style_type::text_short, true);
            ev.dialog(modal, error_autoprint);
            return;
        }
        if (trigg.item_name == "delg") {
            auto modal = modal_generate("roles_roles", "delg", "Remove role group");
            modal_add_component(modal, "Role group name", "name", "* deletes everything, else perfect match", dpp::text_style_type::text_short, true);
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
}