#include <redis/redis.h>

int main()
{
    redis::RedisClient client;

    client.check();
}