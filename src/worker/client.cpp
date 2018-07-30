#include <infra/infra.h>
#include <string>
#include <worker/worker.h>

int main()
{
    worker::Worker worker;

    worker.start();
}