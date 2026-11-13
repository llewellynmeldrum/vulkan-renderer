#pragma once 
#include <cstddef>
#include <charconv>
#include <string>
#include "common.hpp"
#if CPP26
#include <meta>
namespace ansi{
//template<const char* t_on, const char* t_off>
//struct StyleToggle{
//    static constexpr auto on = t_on;
//    static constexpr auto off = t_off;
struct StyleToggle{
    const char* on;
    const char* off;
    consteval auto operator()(std::string_view txt) const noexcept -> std::string{
        #if defined(CLANGD_PARSE)
        return std::string(txt);
        #else 
        return on + std::string(txt) + off;
        #endif 
    }
};
#define DEF_TOGGLE(SYM, CODE_ON, CODE_OFF)\
inline constexpr StyleToggle SYM {"\e[" #CODE_ON "m", "\e[" #CODE_OFF "m"};
#define DEF_SYMBOL(SYM, CODE) inline constexpr const char* SYM = "\e[" #CODE "m";

DEF_TOGGLE(FG_BLACK, 			30,39)
DEF_TOGGLE(FG_RED, 			    31,39)
DEF_TOGGLE(FG_GREEN, 			32,39) 	
DEF_TOGGLE(FG_YELLOW, 			33,39) 	
DEF_TOGGLE(FG_BLUE, 			34,39) 	
DEF_TOGGLE(FG_MAGENTA, 			35,39) 
DEF_TOGGLE(FG_CYAN, 			36,39) 	
DEF_TOGGLE(FG_WHITE, 			37,39) 	
DEF_TOGGLE(FG_BR_BLACK, 		90,39)
DEF_TOGGLE(FG_BR_RED, 			91,39) 	
DEF_TOGGLE(FG_BR_GREEN, 		92,39) 	
DEF_TOGGLE(FG_BR_YELLOW, 		93,39) 	
DEF_TOGGLE(FG_BR_BLUE, 			94,39)

DEF_TOGGLE(BG_BLACK, 			40 ,49)
DEF_TOGGLE(BG_RED, 			    41 ,49)
DEF_TOGGLE(BG_GREEN, 			42 ,49) 	
DEF_TOGGLE(BG_YELLOW, 			43 ,49) 	
DEF_TOGGLE(BG_BLUE, 			44 ,49) 	
DEF_TOGGLE(BG_MAGENTA, 			45 ,49) 
DEF_TOGGLE(BG_CYAN, 			46 ,49) 	
DEF_TOGGLE(BG_WHITE, 			47 ,49) 	
DEF_TOGGLE(BG_BR_BLACK, 		100,49)
DEF_TOGGLE(BG_BR_RED, 			101,49) 	
DEF_TOGGLE(BG_BR_GREEN, 		102,49) 	
DEF_TOGGLE(BG_BR_YELLOW, 		103,49) 	
DEF_TOGGLE(BG_BR_BLUE, 			104,49)

static constexpr auto reset_bg = 49;
static constexpr auto reset_fg = 39;

DEF_TOGGLE(S_BOLD, 1, 22)
DEF_TOGGLE(S_UNDERLINE, 4, 24)

#define SET_COL(n) "\e[" #n "G"
#define CLR_TO_EOL() "\033[K"

#define NEXT_LINE(n) "\033[" #n "E"
#define PREV_LINE(n) "\033[" #n "F"

consteval auto tostr(int v){
    static constexpr auto max_digits = 128;
    std::string buf = std::string(max_digits, '\0');
    std::to_chars(&*buf.begin(), &*buf.end(), v);
    return std::define_static_string(buf);
}
template<typename ...Args>
consteval auto join_strings(Args&& ...args){
    auto res = std::string{};
    (res += std::string(args),...);
    return res;
}
#define static_error(TITLE, ...) static_assert(false,\
SET_COL(0) CLR_TO_EOL() \
+ S_BOLD(__FILE__ ":" + std::string(tostr(__LINE__)) + ": ") + FG_RED(S_BOLD("static error: ")) + S_BOLD(TITLE) +\
"\n    " +join_strings(__VA_ARGS__)+ \
"\n------------------------------------\n"\
)

}
#endif
