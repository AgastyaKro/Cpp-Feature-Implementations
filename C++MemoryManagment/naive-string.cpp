#include <cstddef>
#include <cstring>
#include <algorithm>

class naive_string{

private:

    char* p{};
    std::size_t nelems{};

public:

    naive_string() = default;
    naive_string(const char* s) : nelems(std::strlen(s)){
        p = new char[size() + 1];

        std::copy(s, s + size(), p);
        p[size()] = '\0';
    }

    naive_string(const naive_string& other) : p{new char[other.size() + 1]}, nelems{other.size()} {
        std::copy(other.p, other.p + size(), p);
        p[other.size()] = '\0';
    }

    void swap(naive_string& other){
        using std::swap;

        swap(p, other.p);
        swap(nelems, other.nelems);
    }

    naive_string& operator=(naive_string& other){
        naive_string(other).swap(*this);
        return *this;
    }

    naive_string(naive_string&& other) /*
    p{std::exchange(other.p, nullptr)}, nelems{std::exchange(other.nelems, 0)}
    */
   {
        swap(other); // works becaus NSDMI
    }

    naive_string& operator=(naive_string&& other){
        naive_string{std::move(other)}.swap(*this);
        return *this;
    }

    char& operator[](std::size_t idx){
        return p[idx];
    }

    char operator[](std::size_t idx) const {
        return p[idx];
    }

    ~naive_string(){
        delete[] p;
    }

    std::size_t size() const{
        return nelems;
    }

    bool empty() const {
        return size() == 0;
    }

};

void f(naive_string) {}


void copy_construction_examples(){
    naive_string s1{"What an amazing day!!"};
    naive_string s0 = s1;

    naive_string s2{s1};
    f(s0);
    s2 = s0;

}
int main(){
    copy_construction_examples();
}