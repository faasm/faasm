#include <redis/redis.h>
#include <string>
#include <worker/worker.h>

int main()
{
    worker::Worker worker;

    worker.start();
}