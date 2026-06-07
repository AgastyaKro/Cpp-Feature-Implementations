#include <compare>

std::strong_ordering compareInts(int a, int b){
    if (a < b){
        return std::strong_ordering::less;
    }
    else if (a > b){
        return std::strong_ordering::greater;
    }
    else 
        return std::strong_ordering::equal;
}