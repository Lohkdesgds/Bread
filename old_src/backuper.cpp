#include "backuper.hpp"

nlohmann::json get_from_file(const std::string& path, const std::string& name, const std::string& extension)
{
    //Lunaris::cout << Lunaris::console::color::DARK_GRAY << "Loading files for '" << name << "...";

    auto safefile = get_lock_file();

    try{
        const std::string safename = name.substr(0, max_message_backup_size);

        const auto do_it = [](const std::string& fullpath, nlohmann::json& js) {
            std::ifstream cfile(fullpath);
            if (!cfile.is_open() || !cfile.good()) return false;

            std::stringstream buffer;
            buffer << cfile.rdbuf();
            js = nlohmann::json::parse(buffer.str(), nullptr, false);

            return !js.empty() && !js.is_discarded();
        };

        nlohmann::json j;
        
        if (!do_it(path + safename + extension, j)){
            if (!do_it(path + safename + file_alt_way + extension, j)){
                //Lunaris::cout << Lunaris::console::color::GOLD << "[WARN] Files (main and alt) were empty. Assuming new file for '" << safename << "'.";
                return j;
            }
        }

        //Lunaris::cout << Lunaris::console::color::DARK_GRAY << "Good files for '" << name << "...";
        return j;
    }
    catch(const std::exception& e) {
        Lunaris::cout << Lunaris::console::color::DARK_RED << "[ERROR] Couldn't load data from '" << name << "': " << e.what() << ". Assuming corrupted file.";
    }
    catch(...) {
        Lunaris::cout << Lunaris::console::color::DARK_RED << "[ERROR] Couldn't load data from '" << name << ". Assuming corrupted file.";
    }
    return {};
}

bool save_file(dpp::cluster& core, const nlohmann::json& j, const std::string& path, const std::string& name, const std::string& extension)
{
    auto safefile = get_lock_file();

    try{
        //Lunaris::cout << Lunaris::console::color::DARK_GRAY << "Saving files for '" << name << "...";
        const std::string tostr = j.dump(1);
        const std::string safename = name.substr(0, max_message_backup_size);

        if (tostr.empty() || safename.empty()) return false;

        const auto do_it_save = [](const std::string& fullpath, const std::string& buf) {
            std::ofstream cfile(fullpath);
            if (cfile.is_open() && cfile.good()) {
                cfile << buf;
                cfile.flush();
                //Lunaris::cout << Lunaris::console::color::DARK_GREEN << "Saved file @ '" << fullpath << "'!";
                return true;
            }
            Lunaris::cout << Lunaris::console::color::RED << "Can't save file @ '" << fullpath << "'!";
            return false;
        };

        for(size_t tries = 0; tries < 20 && !do_it_save(path + name + extension, tostr); tries++);
        for(size_t tries = 0; tries < 20 && !do_it_save(path + name + file_alt_way + extension, tostr); tries++);

        //Lunaris::cout << Lunaris::console::color::DARK_GRAY << "Saved the files. Creating message...";

        for(size_t offs = 0; offs < tostr.size(); offs += max_message_file_size) {
            dpp::message msg;
            msg.guild_id = guild_backup_id;
            msg.channel_id = channel_backup_id;
            msg.set_content("`" + path + safename + extension + " v1`");
            msg.set_filename(path + safename + extension);
            msg.set_file_content(tostr.substr(offs, max_message_file_size)); // can't be bigger than 8 MB right? Come on.

            //msg.content = "`" + safename + "_v1`\n```json\n" + tostr.substr(offs, max_message_backup_size) + "```";
            __flush_to_guild_tasker.push_back([msg,&core,safename](){
                bool got_func_end = false;
                core.message_create(msg, [safename, &got_func_end](const dpp::confirmation_callback_t& data){
                    if (data.is_error()) {
                        Lunaris::cout << Lunaris::console::color::RED << "Failed to backup a file named '" << safename << "'!";
                        Lunaris::cout << Lunaris::console::color::RED << "Http: " << data.get_error().message;
                    }
                    got_func_end = true;
                });
                for(size_t p = 0; p < 10 && !got_func_end; p++) std::this_thread::sleep_for(std::chrono::milliseconds(400));
                if (!got_func_end) Lunaris::cout << Lunaris::console::color::RED << "Failed to backup a file named '" << safename << "'! (timeout)";
                std::this_thread::sleep_for(std::chrono::milliseconds(700));
                return true;
            });
        }

        //Lunaris::cout << Lunaris::console::color::DARK_GRAY << "Good creating message!";
            
        return true;
    }
    catch(const std::exception& e) {
        Lunaris::cout << Lunaris::console::color::DARK_RED << "[ERROR] Couldn't save data from '" << name << "': " << e.what() << ".";
    }
    catch(...) {
        Lunaris::cout << Lunaris::console::color::DARK_RED << "[ERROR] Couldn't save data from '" << name << ".";
    }
    return false;
}

void log_joined_new_guild(dpp::cluster& core, const dpp::guild_create_t& ev)
{
    dpp::message msg;
    msg.guild_id = guild_backup_id;
    msg.channel_id = channel_logging_jl;
    msg.content = "✅ Joined guild `" + ev.created->name + "`";//  #" + std::to_string(ev.created->id);
    core.message_create(msg);
}

void log_updated_guild(dpp::cluster& core, const dpp::guild_create_t& ev)
{
    dpp::message msg;
    msg.guild_id = guild_backup_id;
    msg.channel_id = channel_logging_jl;
    msg.content = "🔄 Updated guild `" + ev.created->name + "`";//  #" + std::to_string(ev.created->id);
    core.message_create(msg);
}

void log_left_a_guild(dpp::cluster& core, const dpp::guild_delete_t& ev)
{
    dpp::message msg;
    msg.guild_id = guild_backup_id;
    msg.channel_id = channel_logging_jl;
    msg.content = "❌ Left guild `" + ev.deleted->name + "`";//  #" + std::to_string(ev.deleted->id);
    core.message_create(msg);
}

DelayedTasker& get_file_tasker()
{
    return __flush_to_guild_tasker;
}

size_t get_saving_buffer()
{
    return __flush_to_guild_tasker.remaining();
}