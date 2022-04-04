#include <slashing.hpp>

bool slash_global::load_from(const std::string& s)
{
    const auto buf = dpp::utility::read_file(s);
    if (buf.empty()) return false;
    nlohmann::json j = nlohmann::json::parse(buf);

    if (j.contains("slashes")) {
        for(auto& _field : j["slashes"])
        {
            dpp::slashcommand sl;
            sl.fill_from_json(&_field);
            if (sl.name.empty()) continue;
            sl.application_id = default_bot_id;
            valid_commands.push_back(std::move(sl));
        }
    }
    return valid_commands.size() > 0;
}

size_t slash_global::size() const
{
    return valid_commands.size();
}

dpp::slashcommand& slash_global::index(size_t i)
{
    if (i >= valid_commands.size()) throw std::out_of_range("Index too big");
    return valid_commands[i];
}

const dpp::slashcommand& slash_global::index(size_t i) const
{
    if (i >= valid_commands.size()) throw std::out_of_range("Index too big");
    return valid_commands[i];
}

void slash_global::update_bot_id(const dpp::cluster& bot)
{
    for(auto& i : valid_commands) i.application_id = bot.me.id;
    default_bot_id = bot.me.id;
    cout << console::color::DARK_GREEN << "[SG] Updated bot ID internally to: " << bot.me.id;
}

void slash_global::remove_global(dpp::cluster& bot) const
{
    auto mybomb = std::shared_ptr<bomb>(new bomb([]{
        cout << console::color::GREEN << "[SG] Deleting of global commands is now done.";
    }));

    bot.global_commands_get([&bot,mybomb](const dpp::confirmation_callback_t& dat){
        if (dat.is_error()) {
            cout << console::color::RED << "[SG] Error getting global commands for deletion: " << dat.get_error().message;
            cout << console::color::DARK_RED << "[SG] Full HTTP: \n" << dat.http_info.body;
        }

        dpp::slashcommand_map mappo = std::get<dpp::slashcommand_map>(dat.value);

        for(auto& it : mappo){
            bot.global_command_delete(it.first, [mybomb, secnam = it.second.name](const dpp::confirmation_callback_t& dat){
                if (dat.is_error()) {
                    cout << console::color::RED << "[SG] Error deleting global command: " << secnam;
                }
                else {
                    cout << console::color::DARK_GRAY << "[SG] Removed global command: " << secnam;
                }
            });
        }

        cout << console::color::GREEN << "[SG] Queued all global commands deletion.";
    });
}

void slash_global::apply_bulk(dpp::cluster& bot) const
{
    bot.global_bulk_command_create(valid_commands, [](const dpp::confirmation_callback_t& dat){
        if (dat.is_error()) {
            cout << console::color::RED << "[SG] Error setting up global commands on bulk: " << dat.get_error().message;
            cout << console::color::DARK_RED << "[SG] Full HTTP: \n" << dat.http_info.body;
        }
        else cout << console::color::GREEN << "[SG] Bulk global commands applied successfully.";
    });
}