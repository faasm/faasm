#include <string>
#include <worker/worker.h>

int main()
{
    worker::Worker worker;

    message::FunctionCall call;

    worker.invokeFunction(call);

    return 0;
}