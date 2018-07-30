#include <string>
#include <worker/worker.h>

int main()
{
    worker::WasmModule module;

    message::FunctionCall call;

    module.execute(call);

    return 0;
}