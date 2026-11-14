#include <cxxopts.hpp>
#include <cpptrace/formatting.hpp>

#include "engine.hpp"
#include "globals.hpp"
#include "renderer2d.hpp"
#include "font_atlas.hpp"
#include "logger.hpp"
#include "debug.hpp"

auto read_cli_args(int argc, char** argv) -> cxxopts::ParseResult;
auto main(int argc, char** argv) -> int{
    timer::set_program_epoch();
    auto options = read_cli_args(argc,argv);
    global::debug_mode          = options["debug"].as<bool>();
    global::timer               = options["timer"].as<bool>();
    global::line_no             = options["line-no"].as<bool>();
    bool stacktrace_snippets    = options["stacktrace-snippets"].as<bool>();
    dbg::init_trace_handlers(stacktrace_snippets);

    {
        auto engine = Engine{};
        while (engine.should_run()) {
            engine.loop();
        }
    }
}

auto read_cli_args(int argc, char** argv) -> cxxopts::ParseResult{
    auto option_spec = cxxopts::Options{"vmc", "vulkan rewrite of mc."};
    option_spec.add_options()
        ("d,debug",     "Enable debug mode.")
        ("t,timer",     "Enable timers for log messages.")
        ("l,line-no",   "Enable line numbers for log messages.")
        ("S,stacktrace-snippets",  "Enable snippets in stack traces")
    ;
    auto options = option_spec.parse(argc,argv);
    return options;
}
