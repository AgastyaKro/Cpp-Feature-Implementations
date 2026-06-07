#include "STLSharedPtr.cpp"

template <typename T>
class MyWeakPtr{
    private:
        T* ptr{};
        ControlBlock* controlBlock{};
        
        void TryReleaseWeakPtr(){
            if (controlBlock == nullptr){
                return;
            }
            std::scoped_lock lock{controlBlock->mutex};

            controlBlock->weakPtrCount--;
            if (!controlBlock->sharedPtrCount && !controlBlock->weakPtrCount){
                delete controlBlock;
            }

            ptr = nullptr;
            controlBlock = nullptr;
        }

        void copyFrom(const MyWeakPtr& other){
            ptr = other.ptr;
            controlBlock = other.controlBlock;
            if (controlBlock) {
                std::scoped_lock lock{controlBlock->mutex};
                controlBlock->weakPtrCount++;
            }
        }

        void stealFrom(const MyWeakPtr& other){
            ptr = other.ptr;
            controlBlock = other.controlBlock;
            other.controlBlock = nullptr;
            other.ptr = nullptr;
        }

    public:

        MyWeakPtr() = default;

        MyWeakPtr(const MySharedPtr<T>& shared): ptr(shared.get()), controlBlock(shared.getControlBlock()) {
            if (controlBlock) {
                std::scoped_lock lock{controlBlock->mutex};
                controlBlock->weakPtrCount++;
            }
        }
        
        ~MyWeakPtr(){
            TryReleaseWeakPtr();
        }

        MyWeakPtr(const MyWeakPtr& other){
            copyFrom(other);
        }

        MyWeakPtr(MyWeakPtr&& other) noexcept{
            stealFrom(other);
        }

        MyWeakPtr& operator=(const MyWeakPtr& other){
            TryReleaseWeakPtr();

            ptr = other.ptr;
            controlBlock = other.controlBlock;
            scoped_lock lock{controlBlock->mutex};
            controlBlock->weakPtrCount++;
            
            return *this;
        }

        MyWeakPtr& operator=(MyWeakPtr&& other) noexcept{
            TryReleaseWeakPtr();
            stealFrom(other);

            return *this;
        }

        bool expired(){
            if (controlBlock == nullptr){
                return true;
            }
            std::scoped_lock lock{controlBlock->mutex};
            return (controlBlock->sharedPtrCount == 0);
        }

        MySharedPtr<T> lock() const{
            if (!controlBlock)
                return MySharedPtr<T>{};
            std::scoped_lock lock{controlBlock->mutex};
            if (controlBlock->sharedPtrCount == 0){
                return MySharedPtr<T>{};
            }
            return MySharedPtr<T>(*this);
        }


};