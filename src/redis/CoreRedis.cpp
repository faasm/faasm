#include "CoreRedis.h"

namespace redis {
    CoreRedis::CoreRedis(const std::string &ip, int port) {
        context = redisConnect(ip.c_str(), port);
    }

    CoreRedis::~CoreRedis() {
        redisFree(context);
    }
}
