#include <dpp/dpp.h>
#include <dpp/nlohmann/json.hpp>
#include <dpp/fmt/format.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>

#include <console.h>
#include <bomb.h>

#include <general_config.hpp>
#include <general_functions.hpp>
#include <safe_template.hpp>
#include <slashing.hpp>

using namespace Lunaris;

const std::string needed_paths[] = {"./bot/", "./guilds/", "./users/"};
const std::string config_path = "./bot/config.json";
const std::string slash_path = "./bot/slashes.json";

int main()
{
    cout << console::color::AQUA << "[MAIN] Starting application...";

    for(const auto& i : needed_paths) {if (mkdir(i.c_str(), 0777) && errno != EEXIST) { cout << console::color::RED << "Can't create path \"" << i << "\". Fatal error.";}}

    safe_of<general_config> config;
    slash_global gslash;
    safe_of<std::vector<slash_local>> lslashes;

    if (!gslash.load_from(slash_path)) {
        cout << console::color::RED << "Can't load global slash commands (needed for bot use). Please fix \"" << slash_path << "\"";
        lock_indefinitely();
        return 0;
    }

    if (!config.obj.load_from(config_path) || config.obj.token.empty()) {
        cout << console::color::YELLOW << "[WARN] New configuration? Can't find \"" << config_path << "\" or token is empty. Entering setup process...";

        cout << console::color::GREEN << "[MAIN] Paste your bot token: ";
        std::getline(std::cin, config.obj.token);

        if (config.obj.intents == 0) {
            cout << console::color::GREEN << "[MAIN] Setting up default tokens...";
            config.obj.intents = dpp::i_default_intents;
        }

        if (!config.obj.save_as(config_path)) {
            cout << console::color::RED << "Can't save config at \"" << config_path << "\". Can't start. Please check if bot has permission to read/write files at that location.";
        lock_indefinitely();
            return 0;
        }

        cout << console::color::GREEN << "[MAIN] Starting bot...";
    }

    auto bot = std::unique_ptr<dpp::cluster>(new dpp::cluster(config.obj.token, config.obj.intents, config.obj.shard_count));
    setup_bot(*bot, gslash);


    cout << console::color::GREEN << "[MAIN] Configuration loaded properly. Starting bot...";

    // prepare hard stuff
    auto presence_update_timer = bot->start_timer([&]{ std::lock_guard<std::mutex> l(config.obj_mu); g_tick_presence(config.obj, *bot);}, 60);


    cout << console::color::AQUA << "[MAIN] Any help do 'help'";

    bot->start(true);

    for(bool _keep = true; _keep && bot;) {
        std::string cmd;
        std::getline(std::cin, cmd);
        input_handler_cmd(*bot, _keep, config, lslashes, gslash, cmd);
    }
    
    // close hard stuff
    bot->stop_timer(presence_update_timer);


    cout << console::color::AQUA << "[MAIN] If from now on this freezes, it's safe to just close.";

    bot.reset();

    cout << console::color::AQUA << "[MAIN] Ended bot properly.";
    return 0;
}