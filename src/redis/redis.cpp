#include "redis.h"
#include <util/util.h>

namespace redis {
    RedisClient::RedisClient() {
        {
            std::string hostname = util::getEnvVar("REDIS_HOSTNAME", "localhost");

            std::cout << "Connection to Redis at " << hostname << "\n";

            this->client.connect(hostname, 6379);
        }
    }

    void RedisClient::check() {
        this->client.set("hello", "42");
        this->client.get("hello", [](cpp_redis::reply &reply) {
            std::cout << reply << std::endl;
        });

        this->client.sync_commit();
    }
}

