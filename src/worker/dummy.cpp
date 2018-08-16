#include <string>
#include <worker/worker.h>

#include <unistd.h>

int main()
{
    std::string inputData = "THIS IS MY INPUT";

    message::FunctionCall call;
    call.set_user("simon");
    call.set_function("dummy");
    call.set_inputdata(inputData);

    // Execute in new process
    pid_t child = fork();
    if(child == 0) {
        worker::WasmModule module;
        module.execute(call);

        std::cout << "In process" << std::endl;
    }
    else {
        sleep(5);
    }

    return 0;
}