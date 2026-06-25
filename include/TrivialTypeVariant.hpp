
#include <tuple>

template <typename... Ts>
class TrivialTypeVariant{
    static constexpr std::size_t storage_size = std::max({sizeof(Ts)...});
    static constexpr std::size_t storage_align = std::max({alignof(Ts)...});

    alignas(storage_align) unsigned char storage_[storage_size];
    std::size_t active_index = -1;


    public:
        template<std::size_t I, typename T>
        void set(T value){
            using Target = std::tuple_element_t<I, std::tuple<Ts...>>;

            new (storage_) Target(value);

            active_index = I;
        }

        template<typename T>
        auto& get() const{
            size_t I = index_of<T, Ts...>::value;

            if (I != active_index){
                throw std::runtime_error("type doesn't equal active variables type");
            }
            return *reinterpret_cast<T*>(storage_);
        }

        size_t get_index() const{
            return active_index;
        }
};