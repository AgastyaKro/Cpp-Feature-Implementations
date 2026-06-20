#include "FutureUtils.hpp"
#include "AsyncExecutor.hpp"

#include <tuple>
#include <variant>
#include <utility>
#include <atomic>
#include <exception>
#include <future>
#include <memory>
#include <type_traits>

template <typename Variant>
struct WhenAnyResult{
    size_t index;
    Variant Value;
};

template <typename WhenAnyResult>
struct WhenAnyState{
    std::promise<WhenAnyResult> promise;
    std::atomic<bool> won{false};
};

template<typename Future>
auto get_when_any_value(Future& fut) -> NonVoidFuture_t<Future>{
    using Result = FutureValue_t<Future>;

    if constexpr (std::is_void_v<Result>){
        fut.get();
        return Unit{};
    }
    else {
        return fut.get();
    }
}

template<size_t I, typename Variant, typename FuturesTuple, typename State>
void whenAnyWriter(AsyncExecutor& exec_, FuturesTuple& futuresTuple, std::shared_ptr<State> state) {
    exec_.async([fut = std::move(std::get<I>(futuresTuple)), 
        state
    ]() mutable {
        try {
            auto value = get_when_any_value(fut);
            bool expected = false;
            if (state->won.compare_exchange_strong(expected, true)){
                state->promise.set_value({
                    I,
                    Variant{std::in_place_index<I>, std::move(value)}
                });
            }
        }
        catch(...){
            bool expected = false;
            if (state->won.compare_exchange_strong(expected, true)){
                state->promise.set_expection(std::current_exception());
            }
        }
    }

)
}

template<typename Variant, typename FuturesTuple, typename State, std::size_t... Is>
void whenAnyWriters(AsyncExecutor& exec_, FuturesTuple& futuresTuple, std::shared_ptr<State> state, std::index_sequence<Is...>){
    (whenAnyWriter<Is, Variant>(exec_, futuresTuple, state), ...);
}

template <typename... Futures>
auto whenAnyTakesVoid(AsyncExecutor& exec_, Futures&&... futures) -> std::future<WhenAnyResult<std::variant<NonVoidFuture_t<Futures>...>>>  {
    using Variant = std::variant<NonVoidFuture_t<Futures>...>;
    using Result = WhenAnyResult<Variant>;
    using State = WhenAnyState<Result>;

    auto result = std::make_shared<State>();
    auto output = result->promise.get_future();

    auto futuresTuple = std::make_tuple(std::move(futures)...);
    
    whenAnyWriters<Variant>(exec_, futuresTuple, result, std::index_sequence_for<Futures...>{});


    return output;

}



// Futures passed in -> becomes tuple -> sent to each individual handler which is queued into async
// each handler uses the same WhenAnyState (shared ptr)