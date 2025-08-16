#include <iostream>

int main(){
    int x = 1;
        auto foo = [&x]() mutable{
            x = 2;
        };
        std::cout<< x;
    return 0;
}
