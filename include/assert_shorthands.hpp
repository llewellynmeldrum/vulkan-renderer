#pragma once
#ifndef __cplusplus
	#error "This header is intended for c++ only."
#endif // __cplusplus
#include <libassert/assert.hpp>

#define assert_same_type(T, U)\
static_assert(std::same_as<T,U>,"type `" #T "` differs from type `" #U "`.")

#define assert_aggregate(T)\
static_assert(std::is_aggregate_v<T>,"\n-> type `" #T "` is not an aggregate type.`")


