#pragma once

#include <string>

#include <hiredis/hiredis.h>

namespace redis {
    class CoreRedis {
    public:
        CoreRedis(const std::string &ip, int port);

        ~CoreRedis();

    protected:
        redisContext *context;
    };
}