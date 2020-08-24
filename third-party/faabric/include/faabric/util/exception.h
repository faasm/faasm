#pragma once

#include <exception>
#include <string>

namespace faabric::util {
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