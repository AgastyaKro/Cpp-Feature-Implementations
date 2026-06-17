#include "AsyncExecutor.hpp"
#include "FutureUtils.hpp"

#include <future>


template <typename Future, typename F>
auto then(AsyncExecutor& exec_, Future&& future, F&& func) -> std::future<std::invoke_result_t<F, FutureValue_t<Future>>> {
    using FutureResult = FutureValue_t<Future>;
    using ReturnType = std::invoke_result<F, FutureResult>;

    return exec_.async([func = std::forward<F>(func), insideFut = std::move(future)]() mutable -> ReturnType{
        FutureResult res = insideFut.get();
        return func(std::move(res));
    });
}