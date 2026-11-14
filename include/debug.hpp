#pragma once
#include <cpptrace/formatting.hpp>
#include "globals.hpp"

namespace dbg{
auto init_trace_handlers(bool enable_snippets) -> void;
auto print_trace(cpptrace::stacktrace const& trace) -> void;


auto breakpoint(bool should_print_trace=true) -> void;
[[noreturn]] auto exit(bool should_print_trace=true) -> void;
auto prettify_symbol(std::string const& symbol) -> std::string;
}// namespace dbg

