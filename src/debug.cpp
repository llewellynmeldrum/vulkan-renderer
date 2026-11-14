#include <cpptrace/utils.hpp>
#include <libassert/platform.hpp>
#include <unistd.h>
#include "debug.hpp"
#include "style.hpp"
#include "toml11/fwd/color_fwd.hpp"
namespace dbg{
[[nodiscard]]auto trace_formatter(bool enable_snippets = false) -> cpptrace::formatter const&;
[[noreturn]] auto on_terminate() -> void;
[[noreturn]] auto on_assert_fail(libassert::assertion_info const& info) -> void ;
auto init_trace_handlers() -> void;

auto prettify_symbol(std::string const& symbol) -> std::string{
    return cpptrace::prettify_symbol(symbol);
}

auto print_trace(cpptrace::stacktrace const& trace) -> void{
    trace_formatter().print(std::cerr, trace);
}

auto breakpoint(bool should_print_trace) -> void{
    if (should_print_trace){
        dbg::print_trace(cpptrace::generate_trace());
    }
    LIBASSERT_BREAKPOINT();
}

auto exit(bool should_print_trace) -> void{
    if (should_print_trace){
        dbg::print_trace(cpptrace::generate_trace());
    }
    std::exit(1);
}

auto trace_formatter(bool enable_snippets) -> cpptrace::formatter const&{
    using namespace cpptrace;
    static auto const fmt = cpptrace::formatter{}
        .colors(formatter::color_mode::always)
        .paths(formatter::path_mode::basename)
        .addresses(cpptrace::formatter::address_mode::none)
        .snippets(enable_snippets)
        .filter([](cpptrace::stacktrace_frame const& frame){
            return frame.line.has_value();
        })
    ;
    return fmt;
}


[[noreturn]] auto on_terminate() -> void{
    try {
        if (auto e = std::current_exception()) std::rethrow_exception(e);
        std::println(stderr, "terminate called without an active exception");
        print_trace(cpptrace::generate_trace(1));
    } catch (cpptrace::exception const& e) {      // carries its own throw-site trace
        std::println(stderr, "terminate ({}): {}",cpptrace::demangle(typeid(e).name()), e.message());
        print_trace(e.trace());
    } catch (std::exception const& e) {
        std::println(stderr, "terminate ({}): {}",cpptrace::demangle(typeid(e).name()), e.what());
        print_trace(cpptrace::generate_trace(1));
    } catch (...) {
        std::println(stderr, "terminate: unknown exception");
        print_trace(cpptrace::generate_trace(1));
    }
    std::abort();
}
[[noreturn]] auto on_assert_fail(libassert::assertion_info const& info) -> void {
    auto eyecatch = std::string{};
    eyecatch += std::format("\n{}",style::bold_red());
    for (int i = 0; i< libassert::terminal_width(STDOUT_FILENO); i++){
        eyecatch+= std::format("-");
    }
    eyecatch += std::format("{}\n",style::reset());
    std::print(
        "{}{}{}{}{}",
        eyecatch,
        style::with(style::bold_red(), info.tagline()),
        info.statement(),
        info.print_binary_diagnostics(),
        info.print_extra_diagnostics()
    );
    auto trace = info.get_stacktrace();
    constexpr std::size_t skip_count = 1;
    if (trace.frames.size() > skip_count) {
        trace.frames.erase(trace.frames.begin(), trace.frames.begin() + skip_count);
    } else {
        trace.frames.clear();
    }
    print_trace(trace);
    std::abort();
}
auto init_trace_handlers(bool enable_snippets) -> void{
    auto formatter = dbg::trace_formatter(enable_snippets);
    (void)formatter;
    std::set_terminate(on_terminate);
    libassert::set_failure_handler(on_assert_fail);

}
} // namespace dbg

