#include <process_info.hpp>

void process_info::generate()
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

const std::string& process_info::get(const data& one)
{
    if (one == data::_SIZE) return dat[0];
    return dat[static_cast<size_t>(one)];
}