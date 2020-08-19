#pragma once

#define CHECK_RPC(label, op)  Status __status = op; \
        if(!__status.ok()) {   \
            printf("RPC error %s: %s\n", std::string(label).c_str(),  __status.error_message().c_str());    \
            throw std::runtime_error("RPC error " + std::string(label));    \
        }


#define DEFAULT_RPC_HOST "0.0.0.0"
#define STATE_PORT 8003
#define FUNCTION_CALL_PORT 8004