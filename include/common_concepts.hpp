#pragma once


template <typename T> //
using no_cvref = std::remove_cvref<T>;

template <typename T, typename Fn, typename... Args> //
concept result_type_is = std::same_as<std::invoke_result_t<Fn, Args...>, T>;

template<typename T>
concept is_enum = std::is_enum_v<T>;



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
