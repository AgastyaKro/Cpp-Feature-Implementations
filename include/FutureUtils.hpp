#pragma once

#include <future>
#include <type_traits>
#include <variant>

template<typename T>
struct FutureValue;

template<typename T>
struct FutureValue<std::future<T>>{
    using type = T;
};

template<typename T>
using FutureValue_t = typename FutureValue<std::decay<T>>::type;

template<typename T>
struct NonVoidValue {
    using type = T;
};

template<>
struct NonVoidValue<void>{
    using type = std::monostate;
};

template<typename T>
using NonVoidValue_t = typename NonVoidValue<T>::type; 

template<typename Future>
using NonVoidFuture_t = NonVoidValue_t<FutureValue_t<Future>>;

