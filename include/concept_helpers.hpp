#pragma once


#include <concepts>
#include <ranges>

template<typename T>
using element_t = std::ranges::range_value_t<T>;

template<typename T>
concept is_enum = std::is_enum_v<T>;

template<typename T>
concept is_scoped_enum = std::is_scoped_enum_v<T>;

template<typename T>
using nocvref = std::remove_cvref_t<T>;

template<typename T>
using noref = std::remove_reference_t<T>;


template<typename T, typename U>
concept same_as_nocvref = std::same_as<nocvref<T>,nocvref<U>>;

template<typename T, typename U>
concept same_as_noref = std::same_as<noref<T>,noref<U>>;


// NOTE: Variadic concepts

template<typename T, typename ...Ts>
concept same_as_any = (std::same_as<T, Ts> || ...);

template<typename T, typename ...Ts>
concept same_as_all = (std::same_as<T, Ts> && ...);

template<typename T, typename ...Ts>
concept same_as_none = ((!std::same_as<T, Ts>) && ...);

template<typename ...Ts>
concept all_are_formattable = (std::formattable<Ts,char> && ...);


template<typename T, typename ...Ts>
concept same_as_any_nocvref = (same_as_nocvref<T, Ts> || ...);

template<typename T, typename ...Ts>
concept same_as_all_nocvref = (same_as_nocvref<T, Ts> && ...);

template<typename T, typename ...Ts>
concept same_as_none_nocvref = ((!same_as_nocvref<T, Ts>) && ...);



// NOTE: Numeric concepts
template<typename T>
concept is_numeric = (!same_as_nocvref<T,bool>) && (std::floating_point<T> || std::integral<T>);


// NOTE: generic concepts

template<typename T>
concept is_tuple_like = requires(T){
    std::tuple_size_v<nocvref<T>>;
};
template <typename T> //
using no_cvref = std::remove_cvref<T>;

template <typename T, typename Fn, typename... Args> //
concept result_type_is = std::same_as<std::invoke_result_t<Fn, Args...>, T>;



namespace variadic{

template<typename... Args>
struct first_type;

template<typename tFirst, typename ...tRest>
struct first_type<tFirst, tRest...>{
    using type = tFirst;
};
template<typename ...Args>
using variadic_first_type_t = first_type<Args... >::type;

template<typename T, typename ...Args>
concept same_as = (std::same_as<T,Args> && ...);



template<typename ...Args>
concept all_same = 
    same_as<
        variadic_first_type_t<Args...>,
        Args...
    >;

} // namespace variadic
