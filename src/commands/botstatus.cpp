#include "botstatus.hpp"

void __handle_command_botstatus(dpp::cluster& core, const dpp::interaction_create_t& src, dpp::command_interaction& cmd, const std::shared_ptr<EachLang> lang)
{
    dpp::message replying;
    replying.id = src.command.id;
    replying.channel_id = src.command.channel_id;
    replying.set_type(dpp::message_type::mt_application_command);
    replying.set_flags(64);
    replying.nonce = std::to_string(src.command.usr.id);


    double memuse_mb = 0.0;
    double resident_mb = 0.0;
    int num_threads = 0;

    {
        ProcessInfo proc;
        proc.generate();
        num_threads                  = std::stoi(proc.get(ProcessInfo::data::NUM_THREADS));
        long long raw_rss            = std::stoll(proc.get(ProcessInfo::data::RSS));
        unsigned long long raw_vsize = std::stoull(proc.get(ProcessInfo::data::VSIZE));

        memuse_mb = raw_vsize * 1.0 / 1048576; // 1024*1024
        resident_mb = raw_rss * (1.0 * sysconf(_SC_PAGE_SIZE) / 1048576);
    }


    src.reply(dpp::interaction_response_type::ir_deferred_channel_message_with_source, replying);
    dpp::embed autoembed = dpp::embed()
        .set_author(
            dpp::embed_author{
                .name = (core.me.username + "#" + std::to_string(core.me.discriminator)),
                .url = core.me.get_avatar_url(),
                .icon_url = core.me.get_avatar_url() + "?size=256"
            })
        .set_title("**__" + lang->get(lang_line::COMMAND_BOTSTATUS_FINAL_INFOTITLE) + "__**")
        //.set_description("**―――――――――――――――――――――**") // 21 lines
        //.set_footer(dpp::embed_footer().set_text(DPP_VERSION_TEXT).set_icon(botstatus_image_dpp_url))
        .set_color(random() % 0xFFFFFF)
        .set_thumbnail(botstatus_image_url)
        .add_field(
            lang->get(lang_line::COMMAND_BOTSTATUS_FINAL_TIME_ON), (u8"⏲️ " + core.uptime().to_string()), true
        )
        .add_field(
            lang->get(lang_line::COMMAND_BOTSTATUS_FINAL_SHARD_NOW), (u8"🧭 " + std::to_string(src.from->shard_id)), true
        )
        .add_field(
            lang->get(lang_line::COMMAND_BOTSTATUS_FINAL_SHARDS_ACTIVE), (u8"🕹️ " + std::to_string(core.get_shards().size())), true
        )
        .add_field(
            lang->get(lang_line::COMMAND_BOTSTATUS_FINAL_MAX_SHARDS), (u8"🗺️ " + std::to_string(src.from->max_shards)), true
        )
        .add_field(
            lang->get(lang_line::COMMAND_BOTSTATUS_FINAL_GUILDS_IN_SHARD), (u8"🪀 " + std::to_string(src.from->get_guild_count())), true
        )
        .add_field(
            lang->get(lang_line::COMMAND_BOTSTATUS_FINAL_THREADS_BOT), (u8"🧵 " + std::to_string(num_threads)), true
        )
        .add_field(
            lang->get(lang_line::COMMAND_BOTSTATUS_FINAL_MEMORY_USAGE_FULL), (u8"🧠 " + std::to_string(memuse_mb) + " MB"), true
        )
        .add_field(
            lang->get(lang_line::COMMAND_BOTSTATUS_FINAL_MEMORY_USAGE_RAM), (u8"🧠 " + std::to_string(resident_mb) + " MB"), true
        )
        .add_field(
            lang->get(lang_line::COMMAND_BOTSTATUS_FINAL_USERS_IN_MEMORY), (u8"🐱 " + std::to_string(__user_memory_control.size())), true
        )
        .add_field(
            lang->get(lang_line::COMMAND_BOTSTATUS_FINAL_GUILDS_IN_MEMORY), (u8"🐏 " + std::to_string(__guild_memory_control.size())), true
        )
        .add_field(
            lang->get(lang_line::COMMAND_BOTSTATUS_FINAL_DELAYED_TASKS_AMOUNT), (u8"🥞 " + std::to_string(get_default_tasker().remaining())), true
        );
    
    replying.embeds.push_back(autoembed);
    replying.set_content("");
    replying.set_flags(64);

    src.edit_response(replying);

}