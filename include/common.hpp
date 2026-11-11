#pragma once 
// NOTE: This file should be included basically everywhere. 
// It handles some of the friction between my LSP (clangd23), and my compiler (g++-16)
// We define a couple shorthand 

#if CLANGD_PARSE
#define LSP_IGNORE(...) 
#else
#define LSP_IGNORE(...) __VA_ARGS__
#endif

#if defined(__cpp_impl_reflection)
#define IF_CPP26(...) __VA_ARGS__
#define CPP26 1
#define cpp26 1
#else
#define IF_CPP26(...) 
#endif



#include <string_view>
using namespace std::string_view_literals;
