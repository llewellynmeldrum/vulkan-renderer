#pragma once 
//void setup_ui(PlatformContext const& ctx);
//void draw_ui();
//void render_ui(PlatformContext const& ctx);
//
#include "types.hpp"
#define FWD_DECL_STRUCT(name) struct name
#define FWD_DECL_UNION(name) union name

#define assert_same_type(T, U)\
static_assert(std::same_as<T,U>,"type `" #T "` differs from type `" #U "`.")

#define assert_aggregate(T)\
static_assert(std::is_aggregate_v<T>,"\n-> type `" #T "` is not an aggregate type.`")


template <typename T>
    requires std::floating_point<T> || std::integral<T>
u64 stons(T sec) {
    return static_cast<u64>(sec * 1'000'000'000);
}
