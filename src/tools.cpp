#include "tools.hpp"

void atomic_ull::plus()
{
    std::lock_guard<std::mutex> luck(m);
    val++;
}

void atomic_ull::minus()
{
    std::lock_guard<std::mutex> luck(m);
    val--;
}

mull atomic_ull::read()
{
    return val;
}

mull get_time_ms()
{
    return std::chrono::duration_cast<std::chrono::duration<mull, std::milli>>(std::chrono::system_clock::now().time_since_epoch()).count();
}

std::string fix_name_for_cmd(std::string var)
{
    for(auto&i : var) {
        if (i == ' ') i = '_';
        i = std::tolower(i);
    }
    return var;
}

std::string get_thread_id_str()
{
    std::ostringstream ss;
    ss << std::this_thread::get_id();
    return ss.str();
}

//size_t cast_to_switch(const std::string& src, const std::initializer_list<std::string> opts)
//{
//    size_t opt = 0;
//    for(const auto& i : opts){
//        if (src == i) return opt;
//        ++opt;
//    }
//    return static_cast<size_t>(-1);
//}
//
//size_t cast_to_switch(const nullable_ref<std::string>& src, const std::initializer_list<std::string> opts)
//{
//    return src.is_null() ? static_cast<size_t>(-1) : cast_to_switch(*src, opts);
//}

nullable_ref<std::string> get_first_name(const dpp::command_interaction& cmd)
{
    if (cmd.options.size()) return &cmd.options[0].name;
    return {nullptr};
}

nullable_ref<dpp::command_data_option>  get_first_option(const dpp::command_interaction& cmd)
{
    if (cmd.options.size()) return &cmd.options[0];
    return {nullptr};
}

nullable_ref<std::string> get_first_name(const dpp::command_data_option& cmd)
{
    if (cmd.options.size()) return &cmd.options[0].name;
    return {nullptr};
}

nullable_ref<dpp::command_data_option>  get_first_option(const dpp::command_data_option& cmd)
{
    if (cmd.options.size()) return &cmd.options[0];
    return {nullptr};
}

nullable_ref<std::string> get_first_name(const nullable_ref<dpp::command_interaction>& a)
{
    if (a.is_null()) return nullptr;
    return get_first_name(*a);
}
nullable_ref<dpp::command_data_option> get_first_option(const nullable_ref<dpp::command_interaction>& a)
{
    if (a.is_null()) return nullptr;
    return get_first_option(*a);
}
nullable_ref<std::string> get_first_name(const nullable_ref<dpp::command_data_option>& a)
{
    if (a.is_null()) return nullptr;
    return get_first_name(*a);
}
nullable_ref<dpp::command_data_option> get_first_option(const nullable_ref<dpp::command_data_option>& a)
{
    if (a.is_null()) return nullptr;
    return get_first_option(*a);
}


nullable_ref<std::string> get_str_in_command(const dpp::command_interaction& cmd, const std::string& key)
{
    for(const auto& i : cmd.options){
        if (i.name == key && i.type == dpp::command_option_type::co_string) {
            return &std::get<std::string>(i.value);
        }
    }
    return nullptr;
}

nullable_ref<int32_t> get_int_in_command(const dpp::command_interaction& cmd, const std::string& key)
{
    for(const auto& i : cmd.options){
        if (i.name == key && i.type == dpp::command_option_type::co_integer){
            return &std::get<int32_t>(i.value);
        }
    }
    return nullptr;
}

nullable_ref<bool> get_bool_in_command(const dpp::command_interaction& cmd, const std::string& key)
{
    for(const auto& i : cmd.options){
        if (i.name == key && i.type == dpp::command_option_type::co_boolean){
            return &std::get<bool>(i.value);
        }
    }
    return nullptr;
}

nullable_ref<dpp::snowflake> get_mull_in_command(const dpp::command_interaction& cmd, const std::string& key)
{
    for(const auto& i : cmd.options){
        if (i.name == key && (i.type == dpp::command_option_type::co_user || i.type == dpp::command_option_type::co_channel || i.type == dpp::command_option_type::co_role)){
            return &std::get<dpp::snowflake>(i.value);
        }
    }
    return nullptr;
}

nullable_ref<dpp::command_data_option> get_cmd_in_command(const dpp::command_interaction& cmd, const std::string& key)
{
    for(const auto& i : cmd.options){
        if (i.name == key && (i.type == dpp::command_option_type::co_sub_command || i.type == dpp::command_option_type::co_sub_command_group)){
            return &i;
        }
    }
    return nullptr;
}

nullable_ref<std::string> get_str_in_command(const dpp::command_data_option& cmd, const std::string& key)
{
    for(const auto& i : cmd.options){
        if (i.name == key && i.type == dpp::command_option_type::co_string) {
            return &std::get<std::string>(i.value);
        }
    }
    return nullptr;
}

nullable_ref<int32_t> get_int_in_command(const dpp::command_data_option& cmd, const std::string& key)
{
    for(const auto& i : cmd.options){
        if (i.name == key && i.type == dpp::command_option_type::co_integer){
            return &std::get<int32_t>(i.value);
        }
    }
    return nullptr;
}

nullable_ref<bool> get_bool_in_command(const dpp::command_data_option& cmd, const std::string& key)
{
    for(const auto& i : cmd.options){
        if (i.name == key && i.type == dpp::command_option_type::co_boolean){
            return &std::get<bool>(i.value);
        }
    }
    return nullptr;
}

nullable_ref<dpp::snowflake> get_mull_in_command(const dpp::command_data_option& cmd, const std::string& key)
{
    for(const auto& i : cmd.options){
        if (i.name == key && (i.type == dpp::command_option_type::co_user || i.type == dpp::command_option_type::co_channel || i.type == dpp::command_option_type::co_role)){
            return &std::get<dpp::snowflake>(i.value);
        }
    }
    return nullptr;
}

nullable_ref<dpp::command_data_option> get_cmd_in_command(const dpp::command_data_option& cmd, const std::string& key)
{
    for(const auto& i : cmd.options){
        if (i.name == key && (i.type == dpp::command_option_type::co_sub_command || i.type == dpp::command_option_type::co_sub_command_group)){
            return &i;
        }
    }
    return nullptr;
}


nullable_ref<std::string>               get_str_in_command(const nullable_ref<dpp::command_interaction>& a, const std::string& b)
{
    if (a.is_null()) return nullptr;
    return get_str_in_command(*a, b);
}
nullable_ref<int32_t>                   get_int_in_command(const nullable_ref<dpp::command_interaction>& a, const std::string& b)
{
    if (a.is_null()) return nullptr;
    return get_int_in_command(*a, b);
}
nullable_ref<bool>                      get_bool_in_command(const nullable_ref<dpp::command_interaction>& a, const std::string& b)
{
    if (a.is_null()) return nullptr;
    return get_bool_in_command(*a, b);
}
nullable_ref<dpp::snowflake>            get_mull_in_command(const nullable_ref<dpp::command_interaction>& a, const std::string& b)
{
    if (a.is_null()) return nullptr;
    return get_mull_in_command(*a, b);
}
nullable_ref<dpp::command_data_option>  get_cmd_in_command(const nullable_ref<dpp::command_interaction>& a, const std::string& b)
{
    if (a.is_null()) return nullptr;
    return get_cmd_in_command(*a, b);
}
nullable_ref<std::string>               get_str_in_command(const nullable_ref<dpp::command_data_option>& a, const std::string& b)
{
    if (a.is_null()) return nullptr;
    return get_str_in_command(*a, b);
}
nullable_ref<int32_t>                   get_int_in_command(const nullable_ref<dpp::command_data_option>& a, const std::string& b)
{
    if (a.is_null()) return nullptr;
    return get_int_in_command(*a, b);
}
nullable_ref<bool>                      get_bool_in_command(const nullable_ref<dpp::command_data_option>& a, const std::string& b)
{
    if (a.is_null()) return nullptr;
    return get_bool_in_command(*a, b);
}
nullable_ref<dpp::snowflake>            get_mull_in_command(const nullable_ref<dpp::command_data_option>& a, const std::string& b)
{
    if (a.is_null()) return nullptr;
    return get_mull_in_command(*a, b);
}
nullable_ref<dpp::command_data_option>  get_cmd_in_command(const nullable_ref<dpp::command_data_option>& a, const std::string& b)
{
    if (a.is_null()) return nullptr;
    return get_cmd_in_command(*a, b);
}

jsonable::jsonable(const nlohmann::json& j)
{
    from_json(j);
}

void jsonable::operator=(const nlohmann::json& j)
{
    from_json(j);
}

void ProcessInfo::generate()
{
   using std::ios_base;
   using std::ifstream;

   // 'file' stat seems to give the most reliable results
   //
   ifstream stat_stream("/proc/self/stat",ios_base::in);
   // the two fields we want
   
   for(auto& it : dat) stat_stream >> it;

   stat_stream.close();

   //long page_size_kb = sysconf(_SC_PAGE_SIZE) / 1024; // in case x86-64 is configured to use 2MB pages
   //vm_usage     = vsize / 1024.0;
   //resident_set = rss * page_size_kb;    
}

const std::string& ProcessInfo::get(const data& one)
{
    if (one == data::_SIZE) return dat[0];
    return dat[static_cast<size_t>(one)];
}