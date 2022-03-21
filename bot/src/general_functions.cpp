#include <general_functions.hpp>

void lock_indefinitely()
{
    while(1) std::this_thread::sleep_for(std::chrono::seconds(3600));
}

std::vector<dpp::snowflake> slice_string_auto_snowflake(const std::string& str)
{
    if (str.empty()) return {};
    std::vector<dpp::snowflake> nd;

    const char* noww = str.data();
    char* endd = nullptr;

    while(1) {
        unsigned long long _val = 0;
        _val = std::strtoull(noww, &endd, 10);
        if (endd == noww) break;
        if (_val != 0) nd.push_back(_val);
        noww = endd;
    }

    return nd;
}