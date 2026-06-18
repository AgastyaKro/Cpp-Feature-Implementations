#include "AsyncExecutor.hpp"
#include "FutureUtils.hpp"

#include <atomic>
#include <cstddef>
#include <exception>
#include <future>
#include <memory>
#include <tuple>
#include <utility>
#include <variant>

template<typename Variant>
struct WhenAnyResult{
    std::size_t index;
    Variant variant;
};

template<typename WhenAnyResult>
struct WhenAnyState{
    std::promise<WhenAnyResult> promise;
    std::atomic_bool won{false};
};


template<size_t I, typename Variant, typename Tuple, typename State>
void when_any_waiter(AsyncExecutor& exec_, Tuple& futuresTuple, std::shared_ptr<State> state) {
   exec_.async([fut = std::move(std::get<I>(futuresTuple)), state]() mutable{
    try {
        auto value = fut.get();

        bool expected = false;
        if (state->won.compare_exchange_strong(expected, true)){
            state->promise.set_value({
                I,
                Variant{std::in_place_index<I>, std::move(value)}
            });
        }
    }
    catch (...){
        bool expected = false;
        if (state->won.compare_exchange_strong(expected, true)){
            state->promise.set_exception(std::current_exception());
        }
    }

   });
}

template<typename Variant, typename Tuple, typename State, std::size_t... Is>
void when_any_waiters(AsyncExecutor& exec_, Tuple& futuresTuple, std::shared_ptr<State> state, std::index_sequence<Is...>)
{
    (when_any_waiter<Is, Variant>(exec_, futuresTuple, state), ...);
}


template<typename... Futures>
auto when_any(AsyncExecutor& exec_, Futures&&... futures) -> std::future<WhenAnyResult<std::variant<FutureValue_t<Futures>...>>>{

    using Variant = std::variant<FutureValue_t<Futures>...>;
    using Result = WhenAnyResult<Variant>;
    using State = WhenAnyState<Result>;

    auto state = std::make_shared<State>();
    auto output = state->promise.get_future();

    auto futuresTuple = std::make_tuple(std::move(futures)...);


    when_any_waiters<Variant>(exec_, futuresTuple, state, std::index_sequence_for<Futures...>{});
    return output;

}