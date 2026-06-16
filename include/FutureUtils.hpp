#pragma once

#include <future>
#include <type_traits>

template<typename T>
struct FutureValue;

template<typename T>
struct FutureValue<std::future<T>>{
    using type = T;
};


template<typename T>
using FutureValue_t = typename FutureValue<std::decay<T>::type;
