#pragma once

#include <iostream>
#include <cpp_redis/cpp_redis>
#include <string>

namespace redis {
    class RedisClient {
    public:
        RedisClient();

        std::string check(std::string value);

    private:
        cpp_redis::client client;

    };
}
