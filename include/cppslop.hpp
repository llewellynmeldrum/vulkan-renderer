#pragma once

#include "common.hpp"
#include <cmath>
#include "primitive_types.hpp"
#include <stack>
#include <deque>
#include <ranges>

template<typename T> requires std::integral<T> && (!std::same_as<T,bool>)
auto euclid_mod(T a, T b){
    int r = a %b;
    return T{r<0 ? r+std::abs(b) : r};
}

// SECTION: macros -------------------------------------------------------------

// SUBSECTION: keyword reassignments and language stuff
// delete is so easy to miss, and I often read it as 'default'. This helps distinguish them
#define DELETE delete

// Controversial. I dont really use this much
#define st_cast static_cast
#define cast static_cast

// I dont like how messy these are, so for classes with really simple semantics,
// (eg. this class should not be movable or copyable), i find these macros
// read a lot nicer. Less clutter, especially when they are placed at the top of 
// the class (as they should be, value semantics bugs are cancer)
#define NO_COPY_CTOR(T )    T(const T &) = DELETE;
#define NO_COPY_ASSN(T )    T& operator=(const T &) = DELETE;
#define NO_MOVE_CTOR(T )    T(T &&) = DELETE;
#define NO_MOVE_ASSN(T )    T& operator=(T &&) = DELETE;

#define COPY_CTOR(T, ...)    T(const T __VA_ARGS__ &)
#define COPY_ASSN(T, ...)    T& operator=(const T __VA_ARGS__ &)
#define MOVE_CTOR(T, ...)    T(T __VA_ARGS__ &&)
#define MOVE_ASSN(T, ...)    T& operator=(T __VA_ARGS__ &&)

#define DEFL_CTOR(T, ...)    T()
#define DEFL_DTOR(T, ...)    ~T()


// I dont know why this (or something similar) isnt a builtin keyword.
// Forward declarations like 'struct foo;' are so easy to miss, and imo 
// they should always be very intentional. You should be forced to stop and think:
//  "why am i forward declaring this? Is this the right idea? Am I micro-optimizing compile times
//  on a 2000 line project? Am I overusing PIMPL because our lord and saviour scott meyers told 
//  me it would solve all of my problems?"
#define FORWARD_DECLARE_STRUCT(NAME) struct NAME;

// SUBSECTION: Preprocessor helpers
#define NOP ((void)0)
#define CONCAT(a,b) a##b



using ssize_t = std::ptrdiff_t;

// The ctor that never was
template<typename T, typename Container=std::deque<T>>
constexpr inline auto init_stack(std::initializer_list<T> in){
    return std::stack<T,Container>(std::from_range, in);
}
constexpr auto divisible_by(f32 a, f32 b){
    auto a_int = static_cast<i32>(std::round(a));
    auto b_int = static_cast<i32>(std::round(b));
    return a_int % b_int == 0;
}


// I know this is considered bad practice, but the 'sv' suffix is unique to string view,
// unlike the incredibly silly 's' suffix for both std::string and std::chrono::seconds
using namespace std::string_view_literals;


//NOTE: instantiate this to get a compile error with the type name included
template <typename T>
struct force_type_error;


// Helper for std::visit stuff
template<typename... Ts> struct overloaded : Ts... {using Ts::operator()...;};
