#include <sstream>
#include <string>

#include <cpptrace/formatting.hpp>
#include <libassert/platform.hpp>

#include "vk_debug.hpp"
#include "style.hpp"
#include "types.hpp"

static std::string format_vk_error(vk::DebugUtilsMessengerCallbackDataEXT const* d){
    auto res = std::string{};
//    const char*                                  pMessageIdName;
//    int32_t                                      messageIdNumber;
//    const char*                                  pMessage;
//    uint32_t                                     queueLabelCount;
//    const VkDebugUtilsLabelEXT*                  pQueueLabels;
//    uint32_t                                     cmdBufLabelCount;
//    const VkDebugUtilsLabelEXT*                  pCmdBufLabels;
//    uint32_t                                     objectCount;
//    const VkDebugUtilsObjectNameInfoEXT*         pObjects;
    //
    
    auto header = std::format("VULKAN ERROR [{}]: \n", d->pMessageIdName);
    auto body = std::format("{}\n",d->pMessage);

    auto q_header  =std::format("queue entries (n={}):\n",d->queueLabelCount);
    auto q = std::string{};
    for (u32 i =0 ; i<d->queueLabelCount; i++){
        q += std::format("\t[{}]-> {}, \n",i,d->pQueueLabels[i].pLabelName);
    }
    auto cb_header =std::format("Command buffer entries(n={}):\n",d->cmdBufLabelCount);
    auto cb = std::string{};
    for (u32 i =0 ; i<d->cmdBufLabelCount; i++){
        cb += std::format("\t[{}]-> {}, \n",i,d->pCmdBufLabels[i].pLabelName);
    }
    auto objs_header =std::format("Related objects (n={}):\n",d->objectCount);
    auto objs = std::string{};
    for (u32 i =0 ; i<d->objectCount; i++){
        objs += std::format("\t[{}]-> vk::{} '{}', \n",
        i,vk::to_string(d->pObjects[i].objectType), d->pObjects[i].pObjectName);
    }


    return std::format(
        "{}{}{}{}{}{}{}{}",
        style::with_fg(style::fg_br_red(),header),
        style::with_fg(style::fg_red(),body),
        style::with_fg(style::fg_br_red(),q_header),
        style::with_fg(style::fg_br_red(),q),
        style::with_fg(style::fg_br_red(),cb_header),
        style::with_fg(style::fg_br_red(),cb),
        style::with_fg(style::fg_br_red(),objs_header),
        style::with_fg(style::fg_br_red(),objs)
    );
}
VKAPI_ATTR vk::Bool32 VKAPI_CALL vk_debug_callback(
    vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
    vk::DebugUtilsMessageTypeFlagsEXT types,
    vk::DebugUtilsMessengerCallbackDataEXT const* data,
    void* _
) {
    auto custom_formatter = cpptrace::formatter{}
        .colors(cpptrace::formatter::color_mode::always)    // Force ANSI colors
        .paths(cpptrace::formatter::path_mode::basename)    // Print "main.cpp" instead of full path
        .addresses(cpptrace::formatter::address_mode::none) // Object file address instead of raw
        .snippets(false);                                    // Include source code snippets
    using Sev = vk::DebugUtilsMessageSeverityFlagBitsEXT;
    auto const kind = vk::to_string(types);
    if (severity >= Sev::eError) {
        std::ostringstream out{};
        auto trace = cpptrace::generate_trace();
        for (auto idx = 0uz; idx< trace.frames.size(); idx++){
            auto const& frame = trace.frames.at(idx);
            bool has_dbg_symbols = frame.line.has_value();
            if (has_dbg_symbols){
                out << custom_formatter.format(frame) << "\n";
            }else{
                auto omitted_streak = 0uz;
                while (idx < trace.frames.size() && !trace.frames.at(idx).line.has_value()){
                    omitted_streak++;
                    idx++;
                }
                out << std::format("Omitted {} frames (no dbg_symbols)\n",omitted_streak);
            }
        }
        std::cerr << std::format("VULKAN ERROR! {} ", format_vk_error(data));
        std::cerr << out.str();
        LIBASSERT_BREAKPOINT();
    }
    return vk::False;
}
