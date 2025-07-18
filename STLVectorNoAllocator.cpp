#include <iostream>

// no std::optional variant

template<typename T, typename N>
class MyVector {
    private:
        T* data_;
        std::size_t size_;
        std::size_t capacity_{N};
    
    void grow(){
        capacity_ = (capacity_ == 0) ? 1 : capacity_ * 2;
        T* newData = new T[capacity_];

        for (int i = 0; i < size_; i++){
            newData[i] = std::move(data_[i]);
        }
        delete[] data_;
        data_ = newData;
    }

    public:
        MyVector() : data(nullptr), size_(0), capacity_(0) {}; 
        
        ~MyVector(){
            delete[] data_;
        }

        MyVector(const MyVector& other) : capacity_(other.capacity_), size_(other.size_){
            data_ = new T[capacity_];
        }
        
        MyVector(MyVector&& other) noexcept : data_(other.data_), size_(other.size_), capacity_(other.capacity_){
            other.data_ = nullptr;
        }

        MyVector& operator=(const MyVector& other){
            if (this != &other){
                size_ = other.size_;
                capacity_ = other.capacity_;
                delete[] data_;
                data_ = new T[other.capacity_];
                for (int i = 0; i < other.data_.size(); i++){
                    data_[i] = other.data_[i];
                }
            }
            return *this;
        }

        MyVector& operator=(MyVector&& other) noexcept{
            if (this != &other){
                size_ = other.size_;
                capacity_ = other.capacity_;
                delete[] data_;
                data_ = other.data_;
                other.data_ = nullptr;
            }
            return *this;
        }

        void push_back(const T& element){
            if (size_ = capacity_){
                grow();
            }
            data_[size_++] = element;
        }

        void push_back(T&& element){
            if (size_ = capacity_){
                grow();
            }
            data_[size_++] = std::move(element);
        }





        

    
};