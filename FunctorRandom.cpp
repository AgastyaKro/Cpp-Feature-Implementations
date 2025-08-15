#include <iostream>

struct MyFunctorObject{
    inline void operator()(double param) const{
        std::cout << "handling doubles" << "\n";
    }
    inline void operator()(int param) const{
        std::cout << "handling ints" << "\n";
    }
};

