#include <iostream>
#include <tuple>

template<class... Args>
void CaptureTest(Args... args){
    const auto lambda = [args...]{
        const auto tup = std::make_tuple(args...);
        std::cout << "tuple size: " << std::tuple_size<decltype(tup)>::value;

    };
    lambda();
}