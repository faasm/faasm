#pragma once

#include "State.h"
#include "StateMessage.h"

#include <proto/faasm.pb.h>
#include <proto/faasm.grpc.pb.h>


namespace state {
class StateServer final : public message::StateRPCService::Service {
    public:
        explicit StateServer(State &stateIn);
    private:
        State &state;
    };
}