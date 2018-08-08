#include <string>
#include <worker/worker.h>

int main()
{
    worker::WasmModule module;

    message::FunctionCall call;
    call.set_user("simon");
    call.set_function("print");

    std::string inputData = "THIS IS MY INPUT";

    call.set_inputdata(inputData);

    module.execute(call);

    return 0;
}