template <typename T>
class CircularQueue{
    private:
        std::size_t capacity_ {};
        std::size_t size_ {};
        std::size_t read_idx {};
        std::size_t write_idx {};
        std::unique_ptr<T[]> ptr_{nullptr};
    public:
        CircularQueue() {}
        CircularQueue(std::size_t capacity) : capacity_(capacity), ptr_(std::make_unique<T[]>(capacity_)){}

        void reset(){
            read_idx = 0;
            write_idx = 0;
            size_ = 0;
        }

        bool full(){
            return (size_ == capacity_);
        }

        bool isEmpty(){
            return (size_ == 0);
        }

        std::size_t getSize(){
            return size_;
        }
        
        std::size_t getCapacity(){
            return capacity_;
        }

        std::size_t getReadIdx(){
            return read_idx;
        }

        std::size_t getWriteIdx(){
            return write_idx;
        }

        void write(T value){
            ptr_[write_idx] = value;
            incrementWrite();
        }

        std::optional<T> tryRead(){
            if (size_ != 0){
                T val = ptr_[read_idx];
                incrementRead();
                return val;

            }
            else {
                return std::nullopt;
            }
        }

        void incrementRead(){
            read_idx = (read_idx + 1) % capacity_;
            size_--;
        }

        void incrementWrite(){
            size_t nextWrite = (write_idx + 1) % capacity_;

            if (nextWrite == read_idx){
                read_idx = (read_idx + 1) % capacity_;
            }
            write_idx = nextWrite;
            size_ = std::min(size_ + 1, capacity_);
        }

};