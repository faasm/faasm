#include "redis.h"
#include <util/util.h>

namespace redis {
    RedisClient::RedisClient() {
        {
            std::string hostname = util::getEnvVar("REDIS_HOST", "localhost");

            std::cout << "Connection to Redis at " << hostname << "\n";

            client.connect(hostname, 6379);
        }
    }

    /**
     * Function used to check writing/ reading
     */
    std::string RedisClient::check(std::string value) {
        const char *key = "check_key";
        std::string result;

        client.set(key, value, [](cpp_redis::reply& reply) {
            // Do nothing
        });

        client.get(key, [&result](cpp_redis::reply& reply) {
            result = reply.as_string();
        });

        client.sync_commit();

        return result;
    }
}

