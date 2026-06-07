#include <iostream>

template<typename T>
struct Task{
    virtual void run(T& input) = 0;
    virtual ~Task() = default;
};

template<typename T>
struct TaskRunner{
    void execute(std::unique_ptr<Task<T>> task, T& input){
        try{
            task->run(input);
        }
        catch (const std::exception& e){
            handleError(e);
        }
    }

    virtual void handleError(const std::exception& e){
        std::cerr << "Default error handler" << e.what() << '\n';
    }

    virtual ~TaskRunner() = default;
};

struct Payload{
    int i{0};
    const std::string message{};
};

template<typename T>
struct PayloadTask : public Task<Payload>{
    void run(T& input) override{
        if (input.i < 0){
            throw std::runtime_error("Invalid ID");
        }
        else {
            std::cout << "ran fine";
        }
    }
};

struct PayloadTaskRunner : TaskRunner<Payload>{
    virtual void handleError(const std::exception& e){
        std::cerr << "[Logging Error]" << e.what() << '\n';
    }
};
