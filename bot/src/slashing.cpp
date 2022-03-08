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