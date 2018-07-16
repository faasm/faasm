#include <redis/redis.h>
#include <string>

int main()
{
    redis::RedisClient client;

    std::string checkResult = client.check("Redis worker ok");
    std::cout << checkResult;
}