#include "configurer.hpp"

bool ConfigSetting::open(const std::string& p)
{
    auto safefile = get_lock_file();
    path = p;
	std::ifstream cfile(path);
    if (!cfile.good()) return false;
    cfile >> json;
    return true;
}

bool ConfigSetting::flush() const
{
    auto safefile = get_lock_file();
    std::lock_guard<std::mutex> luck(mu);
    std::ofstream cfile(path);
    if (cfile.good()) {
        cfile << json.dump(2);
        return true;
    }
    return false;
}

void ConfigSetting::do_safe(const std::function<void(nlohmann::json&)>& func)
{
    if (func){
        std::lock_guard<std::mutex> luck(mu);
        func(json);
    }
}

std::string ConfigSetting::qgs(const std::string& key)
{
    std::lock_guard<std::mutex> luck(mu);
    return json[key].get<std::string>();
}

unsigned long long ConfigSetting::qgl(const std::string& key)
{
    std::lock_guard<std::mutex> luck(mu);
    return json[key].get<unsigned long long>();
}

int32_t ConfigSetting::qgi(const std::string& key)
{
    std::lock_guard<std::mutex> luck(mu);
    return json[key].get<int32_t>();
}