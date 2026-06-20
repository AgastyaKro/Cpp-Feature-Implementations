#pragma once

#include <variant>
#include <exception>

template<typename T>
class SettledResult {

private:
    std::variant<T, std::exception_ptr> result_;

    explicit SettledResult(T value) : result_(std::move(value)){}

    explcit SettledResult(std::exception_ptr exception) : result_(exception) {}

public:

    static SettledResult success(T value){
        return SettledResult(std::move(value));
    }   

    static SettledResult failure(std::exception_ptr exception){
        return SettledResult(exception);
    }

    bool has_value() const {
        return std::holds_alternative<T>(result_);
    }

    explicit operator bool() const {
        return has_value();
    }


};