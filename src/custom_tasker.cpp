#include "custom_tasker.hpp"

void DelayedTasker::keep_tasking_so()
{
    indeed_running = true;
    Lunaris::cout << Lunaris::console::color::DARK_GREEN << "[DelayedTasker] Thread is up!";
    while(keep_working || tasks.size()){

        std::this_thread::sleep_for(tasks.size() > threshold_tasker_amount ? threshold_reached_tasker_time : delay_each);

        if (tasks.size()) {
            std::lock_guard<std::mutex> luck(tasks_m);

            //Lunaris::cout << Lunaris::console::color::DARK_GREEN << "[DelayedTasker] Working on task! Remaining: " << tasks.size();
            try {
                if (tasks.size()){
                    if (tasks[0]()){
                        tasks.erase(tasks.begin());
                    }
                }
            }
            catch(const std::exception& e) {
                Lunaris::cout << Lunaris::console::color::DARK_RED << "[DelayedTasker] Exception: " << e.what();
                if (tasks.size()) tasks.erase(tasks.begin());
            }
            catch(...) {
                Lunaris::cout << Lunaris::console::color::DARK_RED << "[DelayedTasker] Exception: UNCAUGHT";
                if (tasks.size()) tasks.erase(tasks.begin());
            }
        }
    }
    Lunaris::cout << Lunaris::console::color::DARK_GREEN << "[DelayedTasker] Thread ended tasks!";
    indeed_running = false;
}

DelayedTasker::DelayedTasker(const std::chrono::seconds t)
    : delay_each(t)
{
    keep_working = true;
    work = std::thread([&]{keep_tasking_so();});
}

DelayedTasker::~DelayedTasker()
{
    destroy();
}

// this function MUST return TRUE when done (keeps being called if false)
void DelayedTasker::push_back(const std::function<bool()>& f)
{
    std::lock_guard<std::mutex> luck(tasks_m);
    tasks.push_back(f);
}

void DelayedTasker::push_back(const std::function<void(const size_t)>& f, const size_t beg, const size_t en)
{
    if (en <= beg) return;
    auto __temp = std::make_shared<size_t>();
    *__temp = beg;
    const std::function<bool()> nf = [f, beg, en, __temp]()->bool{        
        f(*__temp);
        return ++(*__temp) >= en;
    };
    push_back(nf);
}

size_t DelayedTasker::remaining() const
{
    return tasks.size();
}

// only when closing app
void DelayedTasker::destroy(const bool wait)
{
    keep_working = false;
    if (!wait) return;
    while(tasks.size() && indeed_running) std::this_thread::sleep_for(delay_each);
    if (work.joinable()) work.join();
}

DelayedTasker& get_default_tasker()
{
    return __generic_tasker;
}