#pragma once

#include "knative_native/KnativeNativeEndpoint.h"

#include <util/logging.h>
#include <util/environment.h>
#include <util/func.h>
#include <Python.h>

// This will get inclued by faasm/faasm.h via the relevant preprocessor flags

using namespace knative_native;

int main() {
    util::initLogging();
    const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

    const std::string funcName = util::getEnvVar("FAASM_FUNC", "");
    const std::string user = util::getEnvVar("FAASM_USER", "");

    if (funcName.empty() || user.empty()) {
        logger->error("Expected to find FAASM_FUNC and FAASM_USER environment variables");
        throw std::runtime_error("Need to set FAASM_USER and FAASM_FUNC");
    }

    // Initialise the python runtime if necessary
    bool isPython = user == PYTHON_USER && funcName == PYTHON_FUNC;
    if (isPython) {
        Py_InitializeEx(0);
    }

    // Start endpoint (will also have multiple threads)
    logger->info("Starting knative native endpoint");
    KnativeNativeEndpoint endpoint(user, funcName);
    endpoint.start();

    if (isPython) {
        Py_FinalizeEx();
    }

    logger->info("Shutting down knative native endpoint");
    return 0;
}
