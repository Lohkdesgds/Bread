#pragma once

template<typename T>
ComplexSharedPtr<T>::ComplexSharedPtr(const ComplexSharedPtr& o)
    : ptr(o.ptr)
{
}

template<typename T>
ComplexSharedPtr<T>::ComplexSharedPtr(ComplexSharedPtr&& o)
    : ptr(std::move(o.ptr))
{
}

template<typename T>
ComplexSharedPtr<T>::ComplexSharedPtr(std::unique_ptr<T>&& a)
    : ptr(std::make_shared<std::unique_ptr<T>>(std::move(a)))
{
}

template<typename T>
ComplexSharedPtr<T>::ComplexSharedPtr(const shared_unique& a)
    : ptr(a)
{
}

template<typename T>
ComplexSharedPtr<T>::ComplexSharedPtr(shared_unique&& a)
    : ptr(std::move(a))
{
}

template<typename T>
void ComplexSharedPtr<T>::operator=(const ComplexSharedPtr& a)
{
    ptr = a.ptr;
}

template<typename T>
void ComplexSharedPtr<T>::operator=(ComplexSharedPtr&& a)
{
    ptr = std::move(a.ptr);
}

template<typename T>
void ComplexSharedPtr<T>::operator=(std::unique_ptr<T>&& a)
{
    *ptr = std::move(a);
}

template<typename T>
void ComplexSharedPtr<T>::operator=(const shared_unique& a)
{
    ptr = a;
}

template<typename T>
void ComplexSharedPtr<T>::operator=(shared_unique&& a)
{
    ptr = std::move(a);
}

template<typename T>
T* const ComplexSharedPtr<T>::operator->() const
{
    return ptr.get() ? ptr->get() : nullptr;
}

template<typename T>
const T& ComplexSharedPtr<T>::operator*() const
{
    return *(ptr.get() ? ptr->get() : nullptr);
}

template<typename T>
T& ComplexSharedPtr<T>::get_reference_edit()
{
    return *(ptr.get() ? ptr->get() : nullptr);
}

template<typename T>
ComplexSharedPtr<T>::operator bool() const
{
    return ptr.get() != nullptr && ptr->get() != nullptr;
}

template<typename T>
size_t ComplexSharedPtr<T>::use_count() const
{
    return ptr.use_count();
}

template<typename T>
void ComplexSharedPtr<T>::destroy()
{
    ptr->reset();
}

template<typename T, typename... Args>
ComplexSharedPtr<T> make_complex_ptr(Args&&... args)
{
    std::unique_ptr<T> uptr = std::make_unique<T>(std::move(args...));
    return std::make_shared<std::unique_ptr<T>>(std::move(uptr));
}


template<typename T>
inline void MemoryReferenceManager<T>::keep_flush()
{
    Lunaris::cout << Lunaris::console::color::DARK_BLUE << "[MemoryReferenceManager] Thread " << genid() << " started." ;
    is_running = true;
    while(keep_working) {
        for(size_t _p = 0; _p < time_check_references && keep_working; _p++) std::this_thread::sleep_for(time_each);

        const mull time_here_now = get_time_ms();
        size_t cleared_amount = 0;

        {
            auto luck = get_lock();
            for (auto it = umap.begin(); it != umap.end();)
            {
                if (it->second.when_ms < time_here_now && it->second.ref.use_count() <= 1){
                    //const auto __ref_tmp = it->first;
                    //Lunaris::cout << "[MemoryReferenceManager] Clearing " << __ref_tmp << " from memory...";
                    Lunaris::cout << Lunaris::console::color::DARK_AQUA << "Auto-clear (unload) timeout triggered for '" << it->first << "'.";
                    it = umap.erase(it);
                    //Lunaris::cout << "[MemoryReferenceManager] Cleared " << __ref_tmp << " from memory.";
                    if (++cleared_amount > memory_reference_flush_max_overload_control) break;
                }
                else if (it->second.last_flush < time_here_now) {
                    Lunaris::cout << Lunaris::console::color::DARK_AQUA << "Auto-flush (save) timeout triggered for '" << it->first << "'.";

                    if (save_func) save_func(it->second.ref.get_reference_edit());
                    else Lunaris::cout << Lunaris::console::color::GOLD << "Save function was invalid somehow. Please fix.";

                    it->second.last_flush = time_here_now + time_flush_always;
                }
                else ++it;
            }
        }
        //if (cleared_amount) Lunaris::cout << "[MemoryReferenceManager] Cleared " << cleared_amount << " memory objects." ;
    }
    Lunaris::cout << Lunaris::console::color::DARK_BLUE << "[MemoryReferenceManager] Thread " << genid() << " closing..." ;

    while(1) {
        auto luck = get_lock();
        try {
            for(auto& it : umap) it.second.ref.destroy();
            umap.clear();
        }
        catch(const std::exception& e) {
            Lunaris::cout << Lunaris::console::color::DARK_RED << "[MemoryReferenceManager] Thread " << genid() << " had a issue: " << e.what() << ". Trying again in 1 second." ;
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }
        catch(...){
            Lunaris::cout << Lunaris::console::color::DARK_RED << "[MemoryReferenceManager] Thread " << genid() << " had a issue: UNCAUGHT. Trying again in 1 second." ;
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }
        break;
    }
    Lunaris::cout << Lunaris::console::color::BLUE << "[MemoryReferenceManager] Thread " << genid() << " ended." ;
    is_running = false;
}

template<typename T>
inline std::unique_lock<std::shared_mutex> MemoryReferenceManager<T>::get_lock(const std::string& unique_name)
{
    size_t wait_count = 0;
    while(!smu.try_lock()){
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if (++wait_count == 20) {
            if (unique_name.empty()) Lunaris::cout << Lunaris::console::color::DARK_RED << "[MemoryReferenceManager] Thread " << genid() << " is having some issues locking shared!";
            else                     Lunaris::cout << Lunaris::console::color::DARK_RED << "[MemoryReferenceManager] Thread named '" << unique_name << "' is having some issues locking shared!";
            wait_count = 0;
            if (!keep_working) throw std::runtime_error("Cannot lock when closing.");
        }
    }
    std::unique_lock<std::shared_mutex> luck(smu, std::adopt_lock);
    return luck;
}

template<typename T>
inline std::shared_lock<std::shared_mutex> MemoryReferenceManager<T>::get_lock_shared(const std::string& unique_name)
{
    size_t wait_count = 0;
    while(!smu.try_lock_shared()){
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if (++wait_count == 20) {
            if (unique_name.empty()) Lunaris::cout << Lunaris::console::color::DARK_RED << "[MemoryReferenceManager] Thread " << genid() << " is having some issues locking shared!" ;
            else                     Lunaris::cout << Lunaris::console::color::DARK_RED << "[MemoryReferenceManager] Thread named '" << unique_name << "' is having some issues locking shared!" ;
            wait_count = 0;
            if (!keep_working) throw std::runtime_error("Cannot lock when closing.");
        }
    }
    std::shared_lock<std::shared_mutex> luck(smu, std::adopt_lock);
    return luck;
}

template<typename T>
inline bool MemoryReferenceManager<T>::get_nolock(const mull id, ComplexSharedPtr<T>& mov)
{
    auto it = umap.find(id);
    if (it != umap.end()){
        it->second.when_ms = get_time_ms() + min_living_time;
        mov = it->second.ref;
        return true;
    }
    return false;
}

template<typename T>
std::string MemoryReferenceManager<T>::genid() const
{
    return nameit.empty() ? ("#" + get_thread_id_str()) : nameit;
}

template<typename T>
inline MemoryReferenceManager<T>::MemoryReferenceManager(const std::function<T(const mull&)> gen, const std::function<void(T&)> sav, const std::string& newname)
    : generator(gen), save_func(sav), nameit(newname)
{
    is_running = false;
    keep_working = true;
    silence_flush = std::thread([&]{ keep_flush(); });
}

template<typename T>
inline MemoryReferenceManager<T>::~MemoryReferenceManager()
{
    stop();
}

template<typename T>
inline void MemoryReferenceManager<T>::stop(const bool lockin)
{
    if (keep_working || is_running) {
        keep_working = false;
        if (!lockin) return;

        while(is_running) std::this_thread::sleep_for(std::chrono::milliseconds(250));
        if (silence_flush.joinable()) silence_flush.join();
    }
}

template<typename T>
inline ComplexSharedPtr<T> MemoryReferenceManager<T>::get(const mull id)
{
    if (!keep_working) throw std::runtime_error("[MemoryReferenceManager] App is quitting, can't get data right now.");

    ComplexSharedPtr<T> gettin;
    {
        auto luck = get_lock_shared();
        if (get_nolock(id, gettin)) return gettin;
    }

    auto luck = get_lock("MAIN");

    if (get_nolock(id, gettin)) return gettin; // maybe another thread got lock before this one and initialized it for us already

    auto& shr = umap[id];
    shr.ref = make_complex_ptr<T>(generator(id));
    shr.when_ms = get_time_ms() + min_living_time;
    shr.last_flush = get_time_ms() + time_flush_always;
    //Lunaris::cout << "[MemoryReferenceManager] Generated new " << id << "." ;
    return shr.ref;
}

template<typename T>
inline size_t MemoryReferenceManager<T>::size() const
{
    return umap.size();
}