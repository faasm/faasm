#pragma once

#include <exception>
#include <string>
#include <utility>

namespace util {
    class FaasmException : public std::exception {
    public:
        explicit FaasmException(std::string message) : _message(std::move(message)) {

        }

        [[nodiscard]] const char *what() const noexcept override {
            return _message.c_str();
        }

    protected:
        std::string _message;
    };
}