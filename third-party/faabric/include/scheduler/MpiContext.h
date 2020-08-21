#pragma once

#include <scheduler/MpiWorldRegistry.h>
#include <proto/faasm.pb.h>

namespace scheduler {
    class MpiContext {
    public:
        MpiContext();

        void createWorld(const message::Message &msg);

        void joinWorld(const message::Message &msg);

        bool getIsMpi();

        int getWorldId();

        int getRank();
    private:
        bool isMpi;
        int rank;
        int worldId;
    };
}