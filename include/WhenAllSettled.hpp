#include "AsyncExecutor.hpp"
#include "SettledResult.hpp"
#include "FutureUtils.hpp"

#include <future>
#include <tuple>
#include <utility>
#include <exception>

template <typename Future>
auto whenOneSettled(Future& future) -> SettledResult<NonVoidFuture_t<Future>>{
    using RawValue = FutureValue_t<Future>;
    using StoredValue = NonVoidFuture_t<Future>;
    using Result = SettledResult<StoredValue>;

    try {
        if constexpr(std::is_void_v<RawValue>){
            future.get();
            return SettledResult::success(Unit{});
        }
        else{
            return SettledResult::success(future.get());
        }
    }
    catch (...){
        return SettledResult::failure(std::current_exception());
    }
}

template <typename... Futures>
auto whenAllSettled(AsyncExecutor& exec_, Futures&&... futures) -> std::future<std::tuple<SettledResult<NonVoidFuture_t<Futures>>...>> {

    using ResultTuple = std::tuple<SettledResult<NonVoidFuture_t<Futures>>...>;

    return exec_.async(
        [futuresTuple = std::make_tuple(std::move(futures)...)]() mutable -> ResultTuple {
            return std::apply(
            [](auto&... futuresTuple) -> ResultTuple{
                return ResultTuple(whenOneSettled(futuresTuple)...);
            },
            futuresTuple
            );
        }
    );
}