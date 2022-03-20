#include <bot.hpp>

using namespace Lunaris;

int main()
{
    cout << console::color::AQUA << "[MAIN] Starting application...";

    for(const auto& i : needed_paths) {if (mkdir(i.c_str(), 0777) && errno != EEXIST) { cout << console::color::RED << "Can't create path \"" << i << "\". Fatal error.";}}

    safe_data<general_config> config;
    safe_data<slash_global> gslash;
    safe_data<std::vector<slash_local>> lslashes;


    if (!gslash.safe<bool>([](slash_global& s){ return s.load_from(slash_path); })) {
        cout << console::color::RED << "Can't load global slash commands (needed for bot use). Please fix \"" << slash_path << "\"";
        lock_indefinitely();
    }

    if (!config.safe<bool>([](general_config& g){ return g.load_from(config_path) || g.token.empty(); })) {
        cout << console::color::YELLOW << "[WARN] New configuration? Can't find \"" << config_path << "\" or token is empty. Entering setup process...";

        cout << console::color::GREEN << "[MAIN] Paste your bot token: ";

        config.safe<void>([](general_config& g){ 
            std::getline(std::cin, g.token);

            if (g.intents == 0) {
                cout << console::color::GREEN << "[MAIN] Setting up default tokens...";
                g.intents = default_intents;
            }

            if (!g.save_as(config_path)) {
                cout << console::color::RED << "Can't save config at \"" << config_path << "\". Can't start. Please check if bot has permission to read/write files at that location.";
                lock_indefinitely();
            }
        });

        cout << console::color::GREEN << "[MAIN] Starting bot...";
    }

    auto bot = build_bot_from(config);
    if (!bot) {
        cout << console::color::RED << "Can't start bot!";
        lock_indefinitely();
    }

    setup_bot(*bot, gslash);

    cout << console::color::GREEN << "[MAIN] Configuration loaded properly. Starting bot...";

    // prepare hard stuff
    auto presence_update_timer = bot->start_timer([&]{ g_tick_presence(config, *bot);}, 60);
    auto tusers_timer = bot->start_timer([&]{ tf_user_info.free_freeable(); }, 60);

    cout << console::color::AQUA << "[MAIN] Any help do 'help'";

    bot->start(true);

    for(bool _keep = true; _keep && bot;) {
        std::string cmd;
        std::getline(std::cin, cmd);
        input_handler_cmd(*bot, _keep, config, lslashes, gslash, cmd);
    }
    
    // close hard stuff
    bot->stop_timer(presence_update_timer);
    bot->stop_timer(tusers_timer);

    cout << console::color::AQUA << "[MAIN] If from now on this freezes, it's safe to just close.";

    bot.reset();

    cout << console::color::AQUA << "[MAIN] Ended bot properly.";
    return 0;
}