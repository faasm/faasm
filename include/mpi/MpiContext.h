#pragma once

#include <proto/faasm.pb.h>

namespace mpi {
    class MpiContext {
    public:
        MpiContext();

        void setFromMsg(const message::Message &msg);

        bool getIsMpi();

        int getMpiWorldId();

        int getMpiRank();

        void setIsMpi(bool val);

        void setMpiWorldId(int val);

        void setMpiRank(int val);

    private:
        bool isMpi;
        int mpiWorldId;
        int mpiRank;
    };
}