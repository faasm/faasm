#include <string>
#include <worker/worker.h>

int main()
{
    worker::WasmModule module;

    message::FunctionCall call;
    call.set_user("simon");
    call.set_function("dummy");

    module.execute(call);

    return 0;
}