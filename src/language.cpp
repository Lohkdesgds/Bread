#include "language.hpp"

void replaceargformat(std::string& str, std::initializer_list<std::string> args)
{
    for(size_t p = 0; p < args.size(); p++) {
        const std::string srch = "{" + std::to_string(p) + "}";
        size_t pos = str.find(srch);
        if (pos != std::string::npos) {
            str.erase(pos, srch.size());
            str.insert(pos, *(args.begin() + p));
        }
    }
    size_t ps = 0;
    while(ps != std::string::npos)
    {
        if ((ps = str.find("\\}")) != std::string::npos)
        {
            str.erase(ps, 2);
            str.insert(ps, "}");
        }
    }    
}

std::string replaceargformatdirect(std::string str, std::initializer_list<std::string> args)
{
    replaceargformat(str, args);
    return str;
}

lang_command strtolangcmd(const std::string& key)
{
    auto it = __i_conversion_lang_command.find(key);
    if (it == __i_conversion_lang_command.end()) return lang_command::_MAX;
    return it->second;
}

lang_line strtolangline(const std::string& key)
{
    auto it = __i_conversion_lang_line.find(key);
    if (it == __i_conversion_lang_line.end()) return lang_line::_MAX;
    return it->second;
}

std::string langcmdtostr(const lang_command& val)
{
    auto it = std::find_if(__i_conversion_lang_command.begin(), __i_conversion_lang_command.end(), [&](const std::pair<std::string, lang_command>& it){ return it.second == val; });
    if (it == __i_conversion_lang_command.end()) return "";
    return it->first;
}

std::string langlinetostr(const lang_line& val)
{
    auto it = std::find_if(__i_conversion_lang_line.begin(), __i_conversion_lang_line.end(), [&](const std::pair<std::string, lang_line>& it){ return it.second == val; });
    if (it == __i_conversion_lang_line.end()) return "";
    return it->first;
}

std::string checksum_lang_line(const std::unordered_map<lang_line, std::string>& vec)
{
    std::string _errs;
    for(size_t rel = 0; rel < static_cast<size_t>(lang_line::_MAX); rel++) {
        lang_line cst = static_cast<lang_line>(rel);
        auto it = vec.find(cst);
        if (it == vec.end()){
            const auto _tmp = langlinetostr(cst);
            if (_tmp.empty()){ Lunaris::cout << "Invalid value at checksum? `lang_line` can't be '" + std::to_string(rel) + "' somehow?" ; }
            else _errs += (_errs.empty() ? "" : ";") + _tmp;
        }
    }
    return _errs;
}

std::string checksum_lang_command(const std::unordered_map<lang_command, std::string>& vec)
{
    std::string _errs;
    for(size_t rel = 0; rel < static_cast<size_t>(lang_command::_MAX); rel++) {
        lang_command cst = static_cast<lang_command>(rel);
        auto it = vec.find(cst);
        if (it == vec.end()){
            const auto _tmp = langcmdtostr(cst);
            if (_tmp.empty()){ Lunaris::cout << "Invalid value at checksum? `lang_command` can't be '" + std::to_string(rel) + "' somehow?" ; }
            else _errs += (_errs.empty() ? "" : ";") + _tmp;
        }
    }
    return _errs;
}

EachLang::EachLang(const std::unordered_map<lang_line, std::string>& a, const std::unordered_map<lang_command, std::string>& b)
    : ref_lines(a), ref_cmds(b)
{
}

const std::string& EachLang::get(const lang_line& key) const
{
    auto it = ref_lines.find(key);
    if (it == ref_lines.end()) throw std::runtime_error("LANGUAGE KEY DOESN'T EXIST!");    
    return it->second;
}

const std::string& EachLang::get(const lang_command& key) const
{
    auto it = ref_cmds.find(key);
    if (it == ref_cmds.end()) throw std::runtime_error("LANGUAGE KEY DOESN'T EXIST!");    
    return it->second;
}

const std::unordered_map<lang_command, std::string>& EachLang::get_commands() const
{
    return ref_cmds;
}

lang_line EachLang::line_from(const std::string& val) const
{
    auto it = std::find_if(ref_lines.begin(), ref_lines.end(), [&](const std::pair<lang_line, std::string>& it){ return it.second == val; });
    if (it == ref_lines.end()) return lang_line::_MAX;
    return it->first;
}

lang_command EachLang::command_from(const std::string& val, const std::initializer_list<lang_command>& expected) const
{
    const auto in_expected = [&](const lang_command& c){return std::find(expected.begin(), expected.end(), c) != expected.end();};
    auto it = std::find_if(ref_cmds.begin(), ref_cmds.end(), [&](const std::pair<lang_command, std::string>& it){ return it.second == val && in_expected(it.first); });
    if (it == ref_cmds.end()) return lang_command::_MAX;
    return it->first;
}

//LanguageControl::LanguageControl()
//{
//    if (!reload()) {
//        Lunaris::cout << "FATAL ERROR LOADING LANGUAGES! PLEASE CREATE A '" << language_config_path << language_seek_filename << "' WITH AN ARRAY @ 'options' WITH LANGUAGE OPTIONS! (options:[a,b,c,d])" ;
//        exit(EXIT_FAILURE); // MUST LOAD ONCE!
//    }
//}

const std::vector<std::string> LanguageControl::language_list() const
{
    std::lock_guard<std::mutex> luck(control);

    std::vector<std::string> opts;
    for(const auto& i : language_commands) {
        opts.push_back(i.first);
    }

    return opts;
}

std::shared_ptr<EachLang> LanguageControl::get_lang(const std::string& key) const
{
    std::lock_guard<std::mutex> luck(control);

    if (key.empty()) Lunaris::cout << "NOTE: language key was empty? @ LanguageControl::get_lang" ;

    auto it_line = language_lines.find(key);
    auto it_cmds = language_commands.find(key);

    if (it_line == language_lines.end() || it_cmds == language_commands.end()){
        it_line = language_lines.find(language_fallback);
        it_cmds = language_commands.find(language_fallback);
        if (it_line == language_lines.end() || it_cmds == language_commands.end()) throw std::runtime_error("CANNOT GET ANY VALID LANGUAGE!");
    }

    return std::make_shared<EachLang>(EachLang{it_line->second, it_cmds->second});
}

bool LanguageControl::reload()
{
    std::lock_guard<std::mutex> luck(control);

    language_lines.clear();
    language_commands.clear();

    Lunaris::cout << Lunaris::console::color::AQUA << "[LANG]" << Lunaris::console::color::YELLOW << " Opening configuration file '" << language_config_path << language_seek_filename << "'..." ;

    nlohmann::json j;
	std::ifstream cfile(language_config_path + language_seek_filename);
    if (!cfile.good()) return false;
    cfile >> j;

    Lunaris::cout << Lunaris::console::color::AQUA << "[LANG]" << Lunaris::console::color::YELLOW << " Working..." ;
    bool found_issues = false;

    if (j.contains("options") && !j["options"].is_null()) {
        for(const auto& _field : j["options"])
        {
            const std::string current_lang = _field.get<std::string>();
            Lunaris::cout << Lunaris::console::color::AQUA << "[LANG]" << Lunaris::console::color::GREEN << " Loading language '" << current_lang << "'..." ;

            try {
                nlohmann::json jeach;
                std::ifstream feach(language_config_path + current_lang + ".json");
                if (!cfile.good()) throw std::runtime_error("File is inexistent!");
                feach >> jeach;

                // LINES

                if (jeach.contains("lines") && !jeach["lines"].is_null()){
                    for(const auto& eachjson : jeach["lines"])
                    {
                        std::pair<std::string, std::string> eachpair = eachjson.get<std::pair<std::string, std::string>>();
                        
                        auto theon = strtolangline(eachpair.first);

                        if (theon == lang_line::_MAX){ 
                            Lunaris::cout << Lunaris::console::color::AQUA << "[LANG]" << Lunaris::console::color::RED << " Invalid key pair at COMMANDS! Check code '" << eachpair.first << "' with value \"" << eachpair.second << "\". Skipped this line." ;
                            found_issues = true;
                        }
                        else {
                            language_lines[current_lang][theon] = eachpair.second;
                        }
                    }
                    //Lunaris::cout << Lunaris::console::color::AQUA << "[LANG]" << Lunaris::console::color::YELLOW << " Success loading language '" << current_lang << "' LINES." ;
                }
                else throw std::runtime_error("[LANG] LANGUAGE '" + current_lang + "' HAS NO LINES?!");

                // COMMANDS

                if (jeach.contains("commands") && !jeach["commands"].is_null()){
                    for(const auto& eachjson : jeach["commands"])
                    {
                        std::pair<std::string, std::string> eachpair = eachjson.get<std::pair<std::string, std::string>>();
                        
                        auto theon = strtolangcmd(eachpair.first);

                        if (theon == lang_command::_MAX) {
                            Lunaris::cout << Lunaris::console::color::AQUA << "[LANG]" << Lunaris::console::color::RED << " Invalid key pair at COMMANDS! Check code '" << eachpair.first << "' with value \"" << eachpair.second << "\". Skipped this line." ;
                            found_issues = true;
                        }
                        else {
                            language_commands[current_lang][theon] = eachpair.second;
                        }
                    }
                    //Lunaris::cout << Lunaris::console::color::AQUA << "[LANG]" << Lunaris::console::color::YELLOW << " Success loading language '" << current_lang << "' COMMANDS." ;
                }
                else throw std::runtime_error("[LANG] LANGUAGE '" + current_lang + "' HAS NO COMMANDS?!");

                if (auto __str = checksum_lang_line(language_lines[current_lang]); !__str.empty()) {
                    Lunaris::cout << Lunaris::console::color::AQUA << "[LANG]" << Lunaris::console::color::GOLD << " Found missing LINE keys for '" << current_lang << "': " << __str ;
                    found_issues = true;
                }
                if (auto __str = checksum_lang_command(language_commands[current_lang]); !__str.empty()) {
                    Lunaris::cout << Lunaris::console::color::AQUA << "[LANG]" << Lunaris::console::color::GOLD << " Found missing COMMAND keys for '" << current_lang << "': " << __str;
                    found_issues = true;
                }
            }
            catch(const std::exception& e) 
            {
                Lunaris::cout << Lunaris::console::color::AQUA << "[LANG]" << Lunaris::console::color::RED << " Failed loading language '" << current_lang << "'. Error: " << e.what() ;
                found_issues = true;
            }
            catch(...)
            {
                Lunaris::cout << Lunaris::console::color::AQUA << "[LANG]" << Lunaris::console::color::RED << " Failed loading language '" << current_lang << "'. Error: UNCAUGHT" ;
                found_issues = true;
            }
        }
        Lunaris::cout << Lunaris::console::color::AQUA << "[LANG]" << (found_issues ? Lunaris::console::color::YELLOW : Lunaris::console::color::GREEN) << " All languages tasked.";

        if (found_issues){
            Lunaris::cout << Lunaris::console::color::AQUA << "[LANG]" << Lunaris::console::color::RED << " Please do something about the errors. I would NOT continue if there are errors.";
            while(1) std::this_thread::sleep_for(std::chrono::seconds(30));
        }

        return !language_lines.empty() && !language_commands.empty();
    }
    return false;
}

std::shared_ptr<EachLang> get_lang(const std::string& lang)
{
    return langctrl.get_lang(lang);
}


/// STARTUP STUFF

std::unordered_map<std::string, lang_command> __conversion_lang_command_gen()
{
    std::unordered_map<std::string, lang_command> wrk;

    wrk["PING"]                                     = lang_command::PING;
    wrk["BOTSTATUS"]                                = lang_command::BOTSTATUS;
    wrk["POINTS"]                                   = lang_command::POINTS;
    wrk["COPY"]                                     = lang_command::COPY;
    wrk["PASTE"]                                    = lang_command::PASTE;
    wrk["STATS"]                                    = lang_command::STATS;
    wrk["POLL"]                                     = lang_command::POLL;
    wrk["ROLES"]                                    = lang_command::ROLES;
    wrk["CONFIG"]                                   = lang_command::CONFIG;
    wrk["SELFCONF"]                                 = lang_command::SELFCONF;
    wrk["PING_DESC"]                                = lang_command::PING_DESC;
    wrk["BOTSTATUS_DESC"]                           = lang_command::BOTSTATUS_DESC;
    wrk["POINTS_DESC"]                              = lang_command::POINTS_DESC;
    wrk["POINTS_USER"]                              = lang_command::POINTS_USER;
    wrk["POINTS_USER_DESC"]                         = lang_command::POINTS_USER_DESC;
    wrk["POINTS_GLOBAL"]                            = lang_command::POINTS_GLOBAL;
    wrk["POINTS_GLOBAL_DESC"]                       = lang_command::POINTS_GLOBAL_DESC;    
    wrk["COPY_DESC"]                                = lang_command::COPY_DESC;    
    wrk["PASTE_DESC"]                               = lang_command::PASTE_DESC;
    wrk["PASTE_TEXT"]                               = lang_command::PASTE_TEXT;
    wrk["PASTE_TEXT_DESC"]                          = lang_command::PASTE_TEXT_DESC;
    wrk["STATS_DESC"]                               = lang_command::STATS_DESC;
    wrk["STATS_USER"]                               = lang_command::STATS_USER;
    wrk["STATS_USER_DESC"]                          = lang_command::STATS_USER_DESC;
    wrk["POLL_DESC"]                                = lang_command::POLL_DESC;
    wrk["POLL_TEXT"]                                = lang_command::POLL_TEXT;
    wrk["POLL_TEXT_DESC"]                           = lang_command::POLL_TEXT_DESC;
    wrk["POLL_TITLE"]                               = lang_command::POLL_TITLE;
    wrk["POLL_TITLE_DESC"]                          = lang_command::POLL_TITLE_DESC;
    wrk["POLL_MODE"]                                = lang_command::POLL_MODE;
    wrk["POLL_MODE_DESC"]                           = lang_command::POLL_MODE_DESC;
    wrk["POLL_MODE_OPTION_0_DESC"]                  = lang_command::POLL_MODE_OPTION_0_DESC;
    wrk["POLL_MODE_OPTION_1_DESC"]                  = lang_command::POLL_MODE_OPTION_1_DESC;
    wrk["POLL_MODE_OPTION_2_DESC"]                  = lang_command::POLL_MODE_OPTION_2_DESC;
    wrk["POLL_MODE_OPTION_3_DESC"]                  = lang_command::POLL_MODE_OPTION_3_DESC;
    wrk["POLL_MODE_OPTION_4_DESC"]                  = lang_command::POLL_MODE_OPTION_4_DESC;
    wrk["POLL_EMOJIS"]                              = lang_command::POLL_EMOJIS;
    wrk["POLL_EMOJIS_DESC"]                         = lang_command::POLL_EMOJIS_DESC;
    wrk["POLL_LINK"]                                = lang_command::POLL_LINK;
    wrk["POLL_LINK_DESC"]                           = lang_command::POLL_LINK_DESC;
    wrk["POLL_COLOR"]                               = lang_command::POLL_COLOR;
    wrk["POLL_COLOR_DESC"]                          = lang_command::POLL_COLOR_DESC;
    wrk["ROLES_DESC"]                               = lang_command::ROLES_DESC;
    wrk["ROLES_EACH"]                               = lang_command::ROLES_EACH;
    wrk["ROLES_EACH_DESC"]                          = lang_command::ROLES_EACH_DESC;
    wrk["CONFIG_DESC"]                              = lang_command::CONFIG_DESC;
    wrk["CONFIG_APPLY"]                             = lang_command::CONFIG_APPLY;
    wrk["CONFIG_APPLY_DESC"]                        = lang_command::CONFIG_APPLY_DESC;
    wrk["CONFIG_LOGS"]                              = lang_command::CONFIG_LOGS;
    wrk["CONFIG_LOGS_DESC"]                         = lang_command::CONFIG_LOGS_DESC;
    wrk["CONFIG_LANGUAGE"]                          = lang_command::CONFIG_LANGUAGE;
    wrk["CONFIG_LANGUAGE_DESC"]                     = lang_command::CONFIG_LANGUAGE_DESC;
    wrk["CONFIG_LANGUAGE_STRING"]                   = lang_command::CONFIG_LANGUAGE_STRING;
    wrk["CONFIG_LANGUAGE_STRING_DESC"]              = lang_command::CONFIG_LANGUAGE_STRING_DESC;
    wrk["CONFIG_EXTERNAL"]                          = lang_command::CONFIG_EXTERNAL;
    wrk["CONFIG_EXTERNAL_DESC"]                     = lang_command::CONFIG_EXTERNAL_DESC;
    wrk["CONFIG_EXTERNAL_CANPASTE"]                 = lang_command::CONFIG_EXTERNAL_CANPASTE;
    wrk["CONFIG_EXTERNAL_CANPASTE_DESC"]            = lang_command::CONFIG_EXTERNAL_CANPASTE_DESC;
    wrk["CONFIG_EXTERNAL_CANPASTE_ALLOW"]           = lang_command::CONFIG_EXTERNAL_CANPASTE_ALLOW;
    wrk["CONFIG_EXTERNAL_CANPASTE_ALLOW_DESC"]      = lang_command::CONFIG_EXTERNAL_CANPASTE_ALLOW_DESC;
    wrk["CONFIG_ADMIN"]                             = lang_command::CONFIG_ADMIN;
    wrk["CONFIG_ADMIN_DESC"]                        = lang_command::CONFIG_ADMIN_DESC;
    wrk["CONFIG_ADMIN_ADD"]                         = lang_command::CONFIG_ADMIN_ADD;
    wrk["CONFIG_ADMIN_ADD_DESC"]                    = lang_command::CONFIG_ADMIN_ADD_DESC;
    wrk["CONFIG_ADMIN_REMOVE"]                      = lang_command::CONFIG_ADMIN_REMOVE;
    wrk["CONFIG_ADMIN_REMOVE_DESC"]                 = lang_command::CONFIG_ADMIN_REMOVE_DESC;
    wrk["CONFIG_ADMIN_VERIFY"]                      = lang_command::CONFIG_ADMIN_VERIFY;
    wrk["CONFIG_ADMIN_VERIFY_DESC"]                 = lang_command::CONFIG_ADMIN_VERIFY_DESC;
    wrk["CONFIG_ADMIN_COMMON_ROLE"]                 = lang_command::CONFIG_ADMIN_COMMON_ROLE;
    wrk["CONFIG_ADMIN_COMMON_ROLE_DESC"]            = lang_command::CONFIG_ADMIN_COMMON_ROLE_DESC;
    wrk["CONFIG_POINTS"]                            = lang_command::CONFIG_POINTS;
    wrk["CONFIG_POINTS_DESC"]                       = lang_command::CONFIG_POINTS_DESC;
    wrk["CONFIG_POINTS_WHO"]                        = lang_command::CONFIG_POINTS_WHO;
    wrk["CONFIG_POINTS_WHO_DESC"]                   = lang_command::CONFIG_POINTS_WHO_DESC;
    wrk["CONFIG_POINTS_VALUE"]                      = lang_command::CONFIG_POINTS_VALUE;
    wrk["CONFIG_POINTS_VALUE_DESC"]                 = lang_command::CONFIG_POINTS_VALUE_DESC;
    wrk["CONFIG_ROLES"]                             = lang_command::CONFIG_ROLES;
    wrk["CONFIG_ROLES_DESC"]                        = lang_command::CONFIG_ROLES_DESC;
    wrk["CONFIG_ROLES_ADD"]                         = lang_command::CONFIG_ROLES_ADD;
    wrk["CONFIG_ROLES_ADD_DESC"]                    = lang_command::CONFIG_ROLES_ADD_DESC;
    wrk["CONFIG_ROLES_REMOVE"]                      = lang_command::CONFIG_ROLES_REMOVE;
    wrk["CONFIG_ROLES_REMOVE_DESC"]                 = lang_command::CONFIG_ROLES_REMOVE_DESC;
    wrk["CONFIG_ROLES_CLEANUP"]                     = lang_command::CONFIG_ROLES_CLEANUP;
    wrk["CONFIG_ROLES_CLEANUP_DESC"]                = lang_command::CONFIG_ROLES_CLEANUP_DESC;
    wrk["CONFIG_ROLES_COMBO"]                       = lang_command::CONFIG_ROLES_COMBO;
    wrk["CONFIG_ROLES_COMBO_DESC"]                  = lang_command::CONFIG_ROLES_COMBO_DESC;
    wrk["CONFIG_ROLES_ADD_CATEGORY"]                = lang_command::CONFIG_ROLES_ADD_CATEGORY;
    wrk["CONFIG_ROLES_ADD_CATEGORY_DESC"]           = lang_command::CONFIG_ROLES_ADD_CATEGORY_DESC;
    wrk["CONFIG_ROLES_ADD_ROLE"]                    = lang_command::CONFIG_ROLES_ADD_ROLE;
    wrk["CONFIG_ROLES_ADD_ROLE_DESC"]               = lang_command::CONFIG_ROLES_ADD_ROLE_DESC;
    wrk["CONFIG_ROLES_ADD_NAME"]                    = lang_command::CONFIG_ROLES_ADD_NAME;
    wrk["CONFIG_ROLES_ADD_NAME_DESC"]               = lang_command::CONFIG_ROLES_ADD_NAME_DESC;
    wrk["CONFIG_ROLES_REMOVE_CATEGORY"]             = lang_command::CONFIG_ROLES_REMOVE_CATEGORY;
    wrk["CONFIG_ROLES_REMOVE_CATEGORY_DESC"]        = lang_command::CONFIG_ROLES_REMOVE_CATEGORY_DESC;
    wrk["CONFIG_ROLES_REMOVE_ROLE"]                 = lang_command::CONFIG_ROLES_REMOVE_ROLE;
    wrk["CONFIG_ROLES_REMOVE_ROLE_DESC"]            = lang_command::CONFIG_ROLES_REMOVE_ROLE_DESC;
    wrk["CONFIG_ROLES_COMBO_COMBINABLE"]            = lang_command::CONFIG_ROLES_COMBO_COMBINABLE;
    wrk["CONFIG_ROLES_COMBO_COMBINABLE_DESC"]       = lang_command::CONFIG_ROLES_COMBO_COMBINABLE_DESC;
    wrk["CONFIG_ROLES_COMBO_CATEGORY"]              = lang_command::CONFIG_ROLES_COMBO_CATEGORY;
    wrk["CONFIG_ROLES_COMBO_CATEGORY_DESC"]         = lang_command::CONFIG_ROLES_COMBO_CATEGORY_DESC;
    wrk["CONFIG_AUTOROLE"]                          = lang_command::CONFIG_AUTOROLE;
    wrk["CONFIG_AUTOROLE_DESC"]                     = lang_command::CONFIG_AUTOROLE_DESC;
    wrk["CONFIG_AUTOROLE_ADD"]                      = lang_command::CONFIG_AUTOROLE_ADD;
    wrk["CONFIG_AUTOROLE_ADD_DESC"]                 = lang_command::CONFIG_AUTOROLE_ADD_DESC;
    wrk["CONFIG_AUTOROLE_REMOVE"]                   = lang_command::CONFIG_AUTOROLE_REMOVE;
    wrk["CONFIG_AUTOROLE_REMOVE_DESC"]              = lang_command::CONFIG_AUTOROLE_REMOVE_DESC;
    wrk["CONFIG_AUTOROLE_VERIFY"]                   = lang_command::CONFIG_AUTOROLE_VERIFY;
    wrk["CONFIG_AUTOROLE_VERIFY_DESC"]              = lang_command::CONFIG_AUTOROLE_VERIFY_DESC;
    wrk["CONFIG_AUTOROLE_COMMON_ROLE"]              = lang_command::CONFIG_AUTOROLE_COMMON_ROLE;
    wrk["CONFIG_AUTOROLE_COMMON_ROLE_DESC"]         = lang_command::CONFIG_AUTOROLE_COMMON_ROLE_DESC;
    wrk["CONFIG_LEVELS"]                            = lang_command::CONFIG_LEVELS;
    wrk["CONFIG_LEVELS_DESC"]                       = lang_command::CONFIG_LEVELS_DESC;
    wrk["CONFIG_LEVELS_ADD"]                        = lang_command::CONFIG_LEVELS_ADD;
    wrk["CONFIG_LEVELS_ADD_DESC"]                   = lang_command::CONFIG_LEVELS_ADD_DESC;
    wrk["CONFIG_LEVELS_REMOVE"]                     = lang_command::CONFIG_LEVELS_REMOVE;
    wrk["CONFIG_LEVELS_REMOVE_DESC"]                = lang_command::CONFIG_LEVELS_REMOVE_DESC;
    wrk["CONFIG_LEVELS_REDIRECT"]                   = lang_command::CONFIG_LEVELS_REDIRECT;
    wrk["CONFIG_LEVELS_REDIRECT_DESC"]              = lang_command::CONFIG_LEVELS_REDIRECT_DESC;
    wrk["CONFIG_LEVELS_REDIRECT_CHANNELID"]         = lang_command::CONFIG_LEVELS_REDIRECT_CHANNELID;
    wrk["CONFIG_LEVELS_REDIRECT_CHANNELID_DESC"]    = lang_command::CONFIG_LEVELS_REDIRECT_CHANNELID_DESC;
    wrk["CONFIG_LEVELS_MESSAGES"]                   = lang_command::CONFIG_LEVELS_MESSAGES;
    wrk["CONFIG_LEVELS_MESSAGES_DESC"]              = lang_command::CONFIG_LEVELS_MESSAGES_DESC;
    wrk["CONFIG_LEVELS_ADD_ROLE"]                   = lang_command::CONFIG_LEVELS_ADD_ROLE;
    wrk["CONFIG_LEVELS_ADD_ROLE_DESC"]              = lang_command::CONFIG_LEVELS_ADD_ROLE_DESC;
    wrk["CONFIG_LEVELS_ADD_LEVEL"]                  = lang_command::CONFIG_LEVELS_ADD_LEVEL;
    wrk["CONFIG_LEVELS_ADD_LEVEL_DESC"]             = lang_command::CONFIG_LEVELS_ADD_LEVEL_DESC;
    wrk["CONFIG_LEVELS_REMOVE_ROLE"]                = lang_command::CONFIG_LEVELS_REMOVE_ROLE;
    wrk["CONFIG_LEVELS_REMOVE_ROLE_DESC"]           = lang_command::CONFIG_LEVELS_REMOVE_ROLE_DESC;
    wrk["CONFIG_LEVELS_REMOVE_LEVEL"]               = lang_command::CONFIG_LEVELS_REMOVE_LEVEL;
    wrk["CONFIG_LEVELS_REMOVE_LEVEL_DESC"]          = lang_command::CONFIG_LEVELS_REMOVE_LEVEL_DESC;
    wrk["CONFIG_LEVELS_MESSAGES_BLOCK"]             = lang_command::CONFIG_LEVELS_MESSAGES_BLOCK;
    wrk["CONFIG_LEVELS_MESSAGES_BLOCK_DESC"]        = lang_command::CONFIG_LEVELS_MESSAGES_BLOCK_DESC;
    wrk["SELFCONF_DESC"]                            = lang_command::SELFCONF_DESC;
    wrk["SELFCONF_LEVELNOTIF"]                      = lang_command::SELFCONF_LEVELNOTIF;
    wrk["SELFCONF_LEVELNOTIF_DESC"]                 = lang_command::SELFCONF_LEVELNOTIF_DESC;
    wrk["SELFCONF_LEVELNOTIF_ENABLE"]               = lang_command::SELFCONF_LEVELNOTIF_ENABLE;
    wrk["SELFCONF_LEVELNOTIF_ENABLE_DESC"]          = lang_command::SELFCONF_LEVELNOTIF_ENABLE_DESC;
    wrk["SELFCONF_COLOR"]                           = lang_command::SELFCONF_COLOR;
    wrk["SELFCONF_COLOR_DESC"]                      = lang_command::SELFCONF_COLOR_DESC;
    wrk["SELFCONF_COLOR_VALUE"]                     = lang_command::SELFCONF_COLOR_VALUE;
    wrk["SELFCONF_COLOR_VALUE_DESC"]                = lang_command::SELFCONF_COLOR_VALUE_DESC;

    return wrk;
}

std::unordered_map<std::string, lang_line> __conversion_lang_line_gen()
{
    std::unordered_map<std::string, lang_line> wrk;

    wrk["GENERIC_BOT_FAILED_DISCORD_ERROR"]                     = lang_line::GENERIC_BOT_FAILED_DISCORD_ERROR;
    wrk["GENERIC_BOT_FAILED_DOING_TASK"]                        = lang_line::GENERIC_BOT_FAILED_DOING_TASK;
    wrk["GENERIC_BOT_SUCCESS"]                                  = lang_line::GENERIC_BOT_SUCCESS;
    wrk["GENERIC_BOT_SUCCESS_ON_RESET"]                         = lang_line::GENERIC_BOT_SUCCESS_ON_RESET;
    wrk["GENERIC_BOT_NOT_ALLOWED_BECAUSE_GUILD_SETTINGS"]       = lang_line::GENERIC_BOT_NOT_ALLOWED_BECAUSE_GUILD_SETTINGS;
    // generic from commands
    wrk["COMMAND_GENERIC_INVALID_ARGS"]                         = lang_line::COMMAND_GENERIC_INVALID_ARGS;
    wrk["COMMAND_GENERIC_INVALID_ARGS_WITH_NAME"]               = lang_line::COMMAND_GENERIC_INVALID_ARGS_WITH_NAME;
    wrk["COMMAND_GENERIC_CANT_FIND"]                            = lang_line::COMMAND_GENERIC_CANT_FIND;
    wrk["COMMAND_GENERIC_CANT_FIND_WITH_NAME"]                  = lang_line::COMMAND_GENERIC_CANT_FIND_WITH_NAME;
    wrk["COMMAND_GENERIC_ALREADY_SET_WITH_NAME"]                = lang_line::COMMAND_GENERIC_ALREADY_SET_WITH_NAME;
    wrk["COMMAND_GENERIC_ALREADY_EXISTS"]                       = lang_line::COMMAND_GENERIC_ALREADY_EXISTS;
    wrk["COMMAND_GENERIC_SUCCESS_SETTING_WITH_NAME"]            = lang_line::COMMAND_GENERIC_SUCCESS_SETTING_WITH_NAME;
    wrk["COMMAND_GENERIC_SUCCESS_TAKE_TIME"]                    = lang_line::COMMAND_GENERIC_SUCCESS_TAKE_TIME;
    wrk["COMMAND_GENERIC_FOUND_SWITCH_FROM_TO_NAMES"]           = lang_line::COMMAND_GENERIC_FOUND_SWITCH_FROM_TO_NAMES;
    wrk["COMMAND_GENERIC_SUCCESS_ADD_WITH_NAME"]                = lang_line::COMMAND_GENERIC_SUCCESS_ADD_WITH_NAME;
    wrk["COMMAND_GENERIC_SUCCESS_REMOVE_WITH_NAME"]             = lang_line::COMMAND_GENERIC_SUCCESS_REMOVE_WITH_NAME;
    wrk["COMMAND_GENERIC_IN_LIST_TRUE_WITH_NAME"]               = lang_line::COMMAND_GENERIC_IN_LIST_TRUE_WITH_NAME;
    wrk["COMMAND_GENERIC_IN_LIST_FALSE_WITH_NAME"]              = lang_line::COMMAND_GENERIC_IN_LIST_FALSE_WITH_NAME;
    // botstatus.cpp
    wrk["COMMAND_BOTSTATUS_FINAL_INFOTITLE"]                    = lang_line::COMMAND_BOTSTATUS_FINAL_INFOTITLE;
    wrk["COMMAND_BOTSTATUS_FINAL_TIME_ON"]                      = lang_line::COMMAND_BOTSTATUS_FINAL_TIME_ON;
    wrk["COMMAND_BOTSTATUS_FINAL_SHARD_NOW"]                    = lang_line::COMMAND_BOTSTATUS_FINAL_SHARD_NOW;
    wrk["COMMAND_BOTSTATUS_FINAL_SHARDS_ACTIVE"]                = lang_line::COMMAND_BOTSTATUS_FINAL_SHARDS_ACTIVE;
    wrk["COMMAND_BOTSTATUS_FINAL_MAX_SHARDS"]                   = lang_line::COMMAND_BOTSTATUS_FINAL_MAX_SHARDS;
    wrk["COMMAND_BOTSTATUS_FINAL_GUILDS_IN_SHARD"]              = lang_line::COMMAND_BOTSTATUS_FINAL_GUILDS_IN_SHARD;
    wrk["COMMAND_BOTSTATUS_FINAL_THREADS_BOT"]                  = lang_line::COMMAND_BOTSTATUS_FINAL_THREADS_BOT;
    wrk["COMMAND_BOTSTATUS_FINAL_MEMORY_USAGE_FULL"]            = lang_line::COMMAND_BOTSTATUS_FINAL_MEMORY_USAGE_FULL;
    wrk["COMMAND_BOTSTATUS_FINAL_MEMORY_USAGE_RAM"]             = lang_line::COMMAND_BOTSTATUS_FINAL_MEMORY_USAGE_RAM;
    wrk["COMMAND_BOTSTATUS_FINAL_USERS_IN_MEMORY"]              = lang_line::COMMAND_BOTSTATUS_FINAL_USERS_IN_MEMORY;
    wrk["COMMAND_BOTSTATUS_FINAL_GUILDS_IN_MEMORY"]             = lang_line::COMMAND_BOTSTATUS_FINAL_GUILDS_IN_MEMORY;
    wrk["COMMAND_BOTSTATUS_FINAL_DELAYED_TASKS_AMOUNT"]         = lang_line::COMMAND_BOTSTATUS_FINAL_DELAYED_TASKS_AMOUNT;
    // configurar.cpp
    wrk["COMMAND_CONFIG_LOG_LOGS_TITLE"]                        = lang_line::COMMAND_CONFIG_LOG_LOGS_TITLE;
    wrk["COMMAND_CONFIG_LANGUAGE_SUCCESS_TAKE_TIME_WITH_NAME"]  = lang_line::COMMAND_CONFIG_LANGUAGE_SUCCESS_TAKE_TIME_WITH_NAME;
    wrk["COMMAND_CONFIG_LEVELS_BLOCKING_TRUE"]                  = lang_line::COMMAND_CONFIG_LEVELS_BLOCKING_TRUE;
    wrk["COMMAND_CONFIG_LEVELS_BLOCKING_FALSE"]                 = lang_line::COMMAND_CONFIG_LEVELS_BLOCKING_FALSE;
    // copy_paste.cpp
    wrk["COMMAND_COPY_SUCCESS_SPOILER_PASTE"]                   = lang_line::COMMAND_COPY_SUCCESS_SPOILER_PASTE;
    wrk["COMMAND_PASTE_CLIPBOARD_EMPTY"]                        = lang_line::COMMAND_PASTE_CLIPBOARD_EMPTY;
    wrk["COMMAND_PASTE_FINAL_ACCESS_DIRECTLY"]                  = lang_line::COMMAND_PASTE_FINAL_ACCESS_DIRECTLY;
    wrk["COMMAND_PASTE_FINAL_REFERENCED_BY"]                    = lang_line::COMMAND_PASTE_FINAL_REFERENCED_BY;
    wrk["COMMAND_PASTE_FINAL_REFERENCED_SOURCE"]                = lang_line::COMMAND_PASTE_FINAL_REFERENCED_SOURCE;
    wrk["COMMAND_PASTE_FINAL_REFERENCED_GUILD"]                 = lang_line::COMMAND_PASTE_FINAL_REFERENCED_GUILD;
    wrk["COMMAND_PASTE_FINAL_USER_COMMENTED"]                   = lang_line::COMMAND_PASTE_FINAL_USER_COMMENTED;
    wrk["COMMAND_PASTE_FINAL_ORIGINAL_TEXT"]                    = lang_line::COMMAND_PASTE_FINAL_ORIGINAL_TEXT;
    wrk["COMMAND_PASTE_FINAL_ORIGINAL_EMBED_BELOW"]             = lang_line::COMMAND_PASTE_FINAL_ORIGINAL_EMBED_BELOW;
    // points.cpp
    wrk["COMMAND_POINTS_MESSAGE_LEVEL_UP"]                      = lang_line::COMMAND_POINTS_MESSAGE_LEVEL_UP;
    wrk["COMMAND_POINTS_MESSAGE_LEVEL_DOWN"]                    = lang_line::COMMAND_POINTS_MESSAGE_LEVEL_DOWN;
    wrk["COMMAND_POINTS_MESSAGE_GLOBAL_TAG"]                    = lang_line::COMMAND_POINTS_MESSAGE_GLOBAL_TAG;
    wrk["COMMAND_POINTS_MESSAGE_LOCAL_TAG"]                     = lang_line::COMMAND_POINTS_MESSAGE_LOCAL_TAG;
    wrk["COMMAND_POINTS_MESSAGE_LEVEL"]                         = lang_line::COMMAND_POINTS_MESSAGE_LEVEL;
    wrk["COMMAND_POINTS_FINAL_GLOBAL"]                          = lang_line::COMMAND_POINTS_FINAL_GLOBAL;
    wrk["COMMAND_POINTS_FINAL_LOCAL"]                           = lang_line::COMMAND_POINTS_FINAL_LOCAL;
    wrk["COMMAND_POINTS_FINAL_LEVEL_NOW"]                       = lang_line::COMMAND_POINTS_FINAL_LEVEL_NOW;
    wrk["COMMAND_POINTS_FINAL_POINTS_NOW"]                      = lang_line::COMMAND_POINTS_FINAL_POINTS_NOW;
    wrk["COMMAND_POINTS_FINAL_NEXT_LEVEL_IN"]                   = lang_line::COMMAND_POINTS_FINAL_NEXT_LEVEL_IN;
    // poll.cpp
    wrk["COMMAND_POLL_FINAL_TITLE_NAME"]                        = lang_line::COMMAND_POLL_FINAL_TITLE_NAME;
    // statistics.cpp
    wrk["COMMAND_STATS_FINAL_TITLE_NAME"]                       = lang_line::COMMAND_STATS_FINAL_TITLE_NAME;
    wrk["COMMAND_STATS_FINAL_TOTAL_MESSAGES"]                   = lang_line::COMMAND_STATS_FINAL_TOTAL_MESSAGES;
    wrk["COMMAND_STATS_FINAL_MESSAGES_HERE"]                    = lang_line::COMMAND_STATS_FINAL_MESSAGES_HERE;
    wrk["COMMAND_STATS_FINAL_MESSAGES_PERC"]                    = lang_line::COMMAND_STATS_FINAL_MESSAGES_PERC;
    wrk["COMMAND_STATS_FINAL_TOTAL_FILES"]                      = lang_line::COMMAND_STATS_FINAL_TOTAL_FILES;
    wrk["COMMAND_STATS_FINAL_FILES_HERE"]                       = lang_line::COMMAND_STATS_FINAL_FILES_HERE;
    wrk["COMMAND_STATS_FINAL_FILES_PERC"]                       = lang_line::COMMAND_STATS_FINAL_FILES_PERC;
    wrk["COMMAND_STATS_FINAL_TOTAL_COMMANDS"]                   = lang_line::COMMAND_STATS_FINAL_TOTAL_COMMANDS;

    return wrk;
}