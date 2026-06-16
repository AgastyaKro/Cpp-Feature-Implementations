#pragma once

#include "AsyncExecutor.hpp"
#include "FutureUtils.hpp"

#include <future>
#include <tuple>

template<typename... Futures>
auto when_all(AsyncExecutor& exec_, Futures&&... futures)
    -> std::future<std::tuple<FutureValue_t<Futures>...>>
{
    using ResultTuple = std::tuple<FutureValue_t<Futures>...>;

    return exec_.async(
        [futuresTuple = std::make_tuple(std::move(futures)...)]() mutable -> ResultTuple{
            return std::apply(
                [](auto&... futs) -> ResultTuple{
                    return ResultTuple(futs.get()...);
                },
                futuresTuple
            );
        }
    );
}