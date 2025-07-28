template<typename T>
class MyUniquePtr{
    private:
        T* ptr_{};

    public:
        MyUniquePtr() {};
        MyUniquePtr(T* ptr) : ptr_(ptr) {}
        ~MyUniquePtr(){
            delete ptr_;
        }

        MyUniquePtr(const MyUniquePtr& other) = delete;
        MyUniquePtr& operator=(const MyUniquePtr& other) = delete;

        MyUniquePtr& operator=(MyUniquePtr&& other) noexcept{
            if (this != &other){
                delete ptr_;
                ptr_ = other.ptr_;
                other.ptr_ = nullptr;
            }
            return *this;

        }

        MyUniquePtr(MyUniquePtr&& other) noexcept{
            ptr_ = other.ptr_;
            other.ptr_ = nullptr;
        }

        T* operator->() const{
            return ptr_;
        }

        T& operator*() const {
            return *ptr_;
        }

        T* get() const{
            return ptr_;
        }

        T* release(){
            T* temp = ptr_;
            ptr_ = nullptr;
            return temp;
        }

        void reset(T* other = nullptr){
            delete ptr_;
            ptr_ = other;
        }

        template<typename... Types>
        auto operator()(Types&&... args) const{
            return (*ptr_)(std::forward<Types>(args)...);
        }
    
};