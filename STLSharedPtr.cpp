#include <iostream>
#include <utility>

struct ControlBlock{
    size_t sharedPtrCount{};
    size_t weakPtrCount{};
    mutable std::mutex mutex{};
};

template<typename T>
class MySharedPtr{
    private:
        T* ptr = nullptr;
        ControlBlock* controlBlock = nullptr;

        void CopyFrom(const MySharedPtr& other){
            controlBlock = other.controlBlock;
            ptr = other.ptr;
            if (controlBlock){
                std::scoped_lock lock{controlBlock->mutex};
                controlBlock->sharedPtrCount++;
            }
        }

        void StealFrom(MySharedPtr&& other){
            controlBlock = other.controlBlock;
            ptr = other.ptr;
            other.ptr = nullptr;
            other.controlBlock = nullptr;
        }

        void TryReleaseSharedPtr(){
            if (controlBlock == nullptr)
                return;

            std::scoped_lock lock{controlBlock->mutex};

            controlBlock->sharedPtrCount--;
            if (!controlBlock->sharedPtrCount && !controlBlock->weakPtrCount){
                delete controlBlock;
                delete ptr;
            }
            ptr = nullptr;
            controlBlock = nullptr;
        }

    public:
        MySharedPtr() = default;
        MySharedPtr(const MySharedPtr& other){
            CopyFrom(other);
        }
        MySharedPtr(MySharedPtr&& other) noexcept{
            StealFrom(std::move(other));
        }


        MySharedPtr& operator=(const MySharedPtr& other){
            if (this == &other){
                return;
            }
            TryReleaseSharedPtr();
            CopyFrom(other);
            return *this;
        }

        MySharedPtr& operator=(MySharedPtr&& other) noexcept{
            TryReleaseSharedPtr();
            StealFrom(std::move(other));
            return *this;
        }

        ~MySharedPtr(){
            TryReleaseSharedPtr();
        }

        explicit MySharedPtr(T* rawPtr) : controlBlock(new ControlBlock{1, 0}), ptr(rawPtr) {}

        size_t getTotalCount(){
            if (!controlBlock) 
                return 0;
            std::scoped_lock lock{controlBlock->mutex};
            return controlBlock->sharedPtrCount + controlBlock->weakPtrCount;
        }

        size_t getSharedPtrCount(){
            if (!controlBlock) 
                return 0;
            std::scoped_lock lock{controlBlock->mutex};
            return controlBlock->sharedPtrCount;
        }

        size_t getWeakPtrCount(){
            if (!controlBlock) 
                return 0;
            std::scoped_lock lock{controlBlock->mutex};
            return controlBlock->weakPtrCount;
        }

        T* get() const{
            return ptr;
        }
        T& operator*() const{
            return *ptr;
        }

        T* operator->() const{
            return ptr;
        }

};