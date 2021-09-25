#include <dpp/dpp.h>
#include <iostream>
#include <dpp/nlohmann/json.hpp>
#include <dpp/fmt/format.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <memory>

#include "language.hpp"
#include "configurer.hpp"
#include "setup.hpp"

std::string generate_progress_bar(const double perc, const size_t siz)
{
	std::string result(siz, '_');
	for(size_t p = 0; p < siz && p < (perc * siz); p++) result[p] = '#';
	return result;
}

int main()
{
	ConfigSetting conf;
	if (!conf.open("./config.json")) {
		Lunaris::cout << "Failed to open config file!" ;
		return 0;
	}
	bool skip_events = false;

	setup_bot(conf, skip_events);

	__global_cluster_sad_times->start(true);

	std::string commandline;	
	while(1) { 
		std::getline(std::cin, commandline);
		
		if (commandline == "help"){
			Lunaris::cout << "Commands available:";
			Lunaris::cout << "> exit | Close and exit safely";
			Lunaris::cout << "> setstatus <string> | Set bot status string";
			Lunaris::cout << "> reloadlang | Reload lang file (safe)";
		}
		else if (commandline == "exit") {
    		auto safefile = get_lock_file();

			skip_events = true;

			std::this_thread::yield();

			Lunaris::cout << Lunaris::console::color::YELLOW << "Locking new file openings and waiting for no file open..." ;


			__user_memory_control.stop(false);
			__guild_memory_control.stop(false);

			while(safefile.current_amount() > 1 || __user_memory_control.size() > 0 || __guild_memory_control.size() > 0){
				Lunaris::cout << Lunaris::console::color::YELLOW << "Waiting files to close... (" << safefile.current_amount() << " raw files remaining, " << __user_memory_control.size() << " users in memory, " << __guild_memory_control.size() << " guilds in memory)" ;
				std::this_thread::sleep_for(std::chrono::seconds(1));
			}

			Lunaris::cout << Lunaris::console::color::YELLOW << "Cleaning up remaining stuff...";

			__guild_memory_control.stop(true);
			__user_memory_control.stop(true);

			Lunaris::cout << Lunaris::console::color::YELLOW << "Ending delayed tasks...";

			DelayedTasker& dt = get_default_tasker();
			DelayedTasker& dt2 = get_file_tasker();

			dt.destroy(false);
			
			while(dt.remaining() > 0) {
				Lunaris::cout << Lunaris::console::color::YELLOW << "Waiting delayed tasks to end... (Remaining: " << dt.remaining() << ")";
				std::this_thread::sleep_for(std::chrono::seconds(5));
			}
			
			dt.destroy(true); // join()

			dt2.destroy(false);

			while(dt2.remaining() > 0) {
				Lunaris::cout << Lunaris::console::color::YELLOW << "Waiting delayed file flush to end... (Remaining: " << dt2.remaining() << ")";
				std::this_thread::sleep_for(std::chrono::seconds(5));
			}

			dt2.destroy(true);

			Lunaris::cout << Lunaris::console::color::GREEN << "Ended delayed tasks." ;


			Lunaris::cout << Lunaris::console::color::GREEN << "Done. Goodbye! (Ignore error below, 5 sec delay)" ;

			std::this_thread::sleep_for(std::chrono::seconds(5));

			exit(0);
		}
		else if (commandline.find("setstatus ") == 0 && commandline.size() > sizeof("setstatus"))
		{
			std::string sliced = commandline.substr(sizeof("setstatus"));
			if (sliced.size()) {
				conf.do_safe([&](nlohmann::json& j){
					j["status_text"] = sliced;
				});
				conf.flush();
				set_presence(*__global_cluster_sad_times, conf);
				Lunaris::cout << "Applied presence '" << sliced << "'" ;
			}
			else {
				Lunaris::cout << "Invalid input." ;
			}
		}
		else if (commandline == "reloadlang") {
			Lunaris::cout << "Reloading language file...";
			if (!langctrl.try_reload()){
				Lunaris::cout << "Could not reload file. Nothing has changed.";
			}
			else {
				Lunaris::cout << "Success reloading languages.";
			}
		}
	} // quick exit


	exit(0);
	return 0;
}