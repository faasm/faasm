//
// Created by scs17 on 12/07/18.
//

#ifndef FAASM_REDIS_H
#define FAASM_REDIS_H

#include <iostream>
#include <cpp_redis/cpp_redis>
#include <string>
#include "../util/env.h"

class RedisClient {
public:
    cpp_redis::client client;

    RedisClient() {
        std::string hostname = getEnvVar("REDIS_HOSTNAME", "localhost");

        std::cout << "Connection to Redis at " << hostname;

        this->client.connect(hostname, 6379);
    }

    void check();
};

#endif //FAASM_REDIS_H
