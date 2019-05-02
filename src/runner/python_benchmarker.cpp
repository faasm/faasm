#include <Python.h>
#include <prof/prof.h>
#include <util/logging.h>
#include <proto/faasm.pb.h>
#include <wasm/WasmModule.h>
#include <easy/profiler.h>
#include <util/config.h>

#define PYTHON_USER "python"
#define PYTHON_WASM_FUNC "py_func"

void _wasmPyFunction(wasm::WasmModule &module, message::Message &call, wasm::CallChain &callChain) {
    module.execute(call, callChain);
}

void _nativePyFunction(const std::string &pyFile) {
    std::string fullPath = std::string("/usr/local/code/faasm/python/funcs/") + pyFile;

    FILE *fp = fopen(fullPath.c_str(), "r");
    if (fp == nullptr) {
        throw std::runtime_error("Failed to open python file");
    }

    printf("Running python function: %s\n", fullPath.c_str());

    Py_InitializeEx(0);
    printf("\n\nInitialised\n");

    PyRun_SimpleFile(fp, "python_func");
    printf("\n\nExecuted\n");

    Py_Finalize();
    printf("Finalised\n");

    fclose(fp);
}

int main(int argc, char *argv[]) {
    util::initLogging();

    std::string pyFile = "float_bench.py";

    // Run in unsafe mode to give Python access
    util::SystemConfig &conf = util::getSystemConfig();
    conf.unsafeMode = "on";

    // Initialise wasm runtime
    const util::TimePoint tpInit = prof::startTimer();
    wasm::WasmModule module;

    message::Message call;
    call.set_user(PYTHON_USER);
    call.set_function(PYTHON_WASM_FUNC);
    call.set_inputdata(pyFile);
    wasm::CallChain callChain(call);

    module.initialise();
    module.bindToFunction(call);
    prof::logEndTimer("WASM initialisation", tpInit);

    EASY_PROFILER_ENABLE
    EASY_FUNCTION()

    EASY_BLOCK("Native")
    _nativePyFunction(pyFile);
    EASY_END_BLOCK

    EASY_BLOCK("WASM")
    _wasmPyFunction(module, call, callChain);
    //EASY_END_BLOCK; // This is not needed because all blocks are ended on destructor when closing braces met

    profiler::dumpBlocksToFile("profile.prof");

    return 0;
}