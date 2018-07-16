#pragma once

#include <iostream>
#include <cpp_redis/cpp_redis>
#include <string>

namespace redis {
    class RedisClient {
    public:
        cpp_redis::client client;

        RedisClient();

        void check();
    };
}
