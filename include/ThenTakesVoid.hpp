
#include "FutureUtils.hpp"
#include "AsyncExecutor.hpp"

#include <future>
#include <type_traits>
#include <functional>
#include <utility>



template<typename Future, typename F>
struct ThenReturn{
private:
    using FutureResult = FutureValue_t<Future>;

public:
    using type = std::conditional_t<
        std::is_void_v<FutureResult>, 
        std::invoke_result_t<F>, 
        std::invoke_result_t<F, FutureResult>
        >;

};

template<typename Future, typename F>
using ThenResult_t = ThenReturn<Future, F>::type;


template<typename Future, typename F>
auto ThenTakesVoid(AsyncExecutor& exec_, Future&& future, F&& func) -> 
    std::future<ThenResult_t<Future, F>>{
    
    using FutureReturn = FutureValue_t<Future>;
    using FuncReturn = ThenResult_t<Future, F>;

    return exec_.async([insideFuture = std::move(future), insideFunction = std::future<F>(func)]() mutable -> FuncReturn{
        if constexpr (std::is_void_v(FutureReturn)){
            insideFuture.get();
            if constexpr (std::is_void_v(FuncReturn)){
                insideFunction();
                return;
            }
            else {
                return insideFunction();

            }
        }
        else {
            auto params = insideFuture.get();
            if constexpr (std::is_void_v(FuncReturn)){
                insideFunction(std::move(params));
                return;
            }
            else {
                return insideFunction(std::move(params));
            }
        }
    }
);
        
}