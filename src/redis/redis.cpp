#include "redis.h"

void RedisClient::check() {
    this->client.set("hello", "42");
    this->client.get("hello", [](cpp_redis::reply& reply) {
        std::cout << reply << std::endl;
    });

    this->client.sync_commit();
}
