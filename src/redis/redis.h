//
// Created by scs17 on 12/07/18.
//

#ifndef FAASM_REDIS_H
#define FAASM_REDIS_H

#include <iostream>
#include <cpp_redis/cpp_redis>

class RedisClient {
    public:
        cpp_redis::client client;

        RedisClient() {
            this->client.connect("redis", 6379);

        }

        void check();
};

#endif //FAASM_REDIS_H
