#pragma once

#include <dpp/dpp.h>
#include <string>

const std::string needed_paths[] = {"./bot/", "./guilds/", "./users/"};
const std::string config_path = "./bot/config.json";
const std::string slash_path = "./bot/slashes.json";
const auto default_intents = dpp::i_default_intents | dpp::i_message_content;

const auto factory_default_time = std::chrono::seconds(120); // 2 min in memory

namespace user_props {
    constexpr size_t user_path_off = 2; // needed_paths[2]
    constexpr unsigned long long time_to_earn_points_sameuser_ms = 2 * 60 * 1000;

    constexpr int range_low = -10;
    constexpr int range_high = 35;
    constexpr int range_total = range_high - range_low; // easier
    constexpr int range_boost_low = 40;
    constexpr int range_boost_high = 200;
    constexpr int range_boost_total = range_boost_high - range_boost_low; // easier
    constexpr unsigned range_boost_chances = 40; // one in 40 (it was 42)
}

namespace guild_props {
    constexpr size_t guild_path_off = 1; // needed_paths[1]
    constexpr unsigned long long time_to_earn_points_diffuser_ms = 45 * 1000;
    constexpr size_t guild_log_each_max_size = 256;

    constexpr size_t max_onjoin_roles_len = 20;
    
    constexpr size_t max_role_groups = 8;
    constexpr size_t max_role_group_each = 20;
}

namespace images {
    const std::string botstatus_image_url = "https://media.discordapp.net/attachments/739704685505544363/740803216626679808/personal_computer.png?width=468&height=468";
    //const std::string botstatus_image_dpp_url = "https://cdn.discordapp.com/attachments/739704685505544363/886048618711244851/dpp_image.png?size=256";

    const std::string url_author_icon_clipboard = "https://media.discordapp.net/attachments/739704685505544363/872539023057489920/folder_icon.png";

    const std::string poll_image_url = "https://media.discordapp.net/attachments/739704685505544363/740437613755760650/POLL_img.png";

    const std::string points_image_url = "https://media.discordapp.net/attachments/739704685505544363/740364166769934407/stonks_dna.png";

    // big data
    const char dna_points_percentage_per_percent_begin[] = "https://cdn.discordapp.com/attachments/739704685505544363/";
    const char dna_points_percentage_per_percent_end[] = ".png";
    const char dna_points_percentage_per_percent[101][23] = { "741004057501499422/00", "741004199457980446/01", "741004202469359656/02", "741004206613200976/03", "741004213886124073/04", "741004218659504128/05", "741004222845288458/06", "741004227966533662/07", "741004232911487096/08", "741004237986594846/09", "741004242193481779/10", "741004247079846038/11", "741004253518102641/12", "741004256374423682/13", "741004264754774016/14", "741004270278803496/15", "741004275336872037/16", "741004281511149739/17", "741004286384799754/18", "741004291271032842/19", "741004297885450290/20", "741004303900213288/21", "741004308576731136/22", "741004383537594448/23", "741004394237001738/24", "741004401019191347/25", "741004408145575996/26", "741004414923309167/27", "741004422309740544/28", "741004428743671899/29", "741004435924320326/30", "741004442379223050/31", "741004449543094382/32", "741004455700463626/33", "741004462621196378/34", "741004469831073842/35", "741004475887648949/36", "741004482963308704/37", "741004488839528548/38", "741004494619541514/39", "741004501900722206/40", "741004507843919963/41", "741004513938243644/42", "741004520930148442/43", "741004526642790470/44", "741004533362327632/45", "741004539083227227/46", "741004546020737034/47", "741004552093958144/48", "741004558351859722/49", "741004564857225328/50", "741004571450540142/51", "741004577809105016/52", "741004585036152862/53", "741004591407169627/54", "741004598097215698/55", "741004606221320293/56", "741004610860351669/57", "741004618129080450/58", "741004708122198016/59", "741004714942005368/60", "741004721019551744/61", "741004727998742538/62", "741004735590563870/63", "741004741806522409/64", "741004748425134110/65", "741004754519588894/66", "741004763096678450/67", "741004770654945312/68", "741004777294397460/69", "741004783795568680/70", "741004790451929159/71", "741004799314755644/72", "741004806453198858/73", "741004813910933584/74", "741004821254897724/75", "741004832923582516/76", "741004907649171556/77", "741004917107458099/78", "741004925227761705/79", "741004935562395678/80", "741004943690956820/81", "741004956085256292/82", "741004964595236924/83", "741004987072774144/84", "741004996803428445/85", "741005004743377016/86", "741005013064876072/87", "741005022002937906/88", "741005029535776938/89", "741005043783958548/90", "741005055359975495/91", "741005061907284069/92", "741005071097135114/93", "741005080026939392/94", "741005088536920094/95", "741005450031530024/96", "741005464199888906/97", "741005477336580208/98", "741005489801789500/99", "819693191972126816/100" };

    const std::string statistics_image_url = "https://media.discordapp.net/attachments/739704685505544363/740787679364382790/cadeado_key.png";
}

namespace leveling {

    constexpr unsigned long long threshold_points_level_0 = 6; // pow(calc_level_div, this). Level 2 is >= pow(calc_level_div, this + 1)
    constexpr double calc_level_div = 1.8; // val /= this X times = TIMES level
}