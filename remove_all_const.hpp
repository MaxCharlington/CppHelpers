#pragma once

#include <type_traits>

template <typename T>
struct remove_all_const : std::remove_const<T> {};

template <typename T>
struct remove_all_const<T *>
{
    using type = remove_all_const<T>::type *;
};

template <typename T>
struct remove_all_const<T *const>
{
    using type = remove_all_const<T>::type *;
};

template <typename T>
using remove_all_const_t = remove_all_const<T>::type;
