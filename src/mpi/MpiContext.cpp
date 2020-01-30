#include <proto/faasm.pb.h>
#include "mpi/MpiContext.h"

namespace mpi {
    MpiContext::MpiContext(): isMpi(false), mpiWorldId(-1), mpiRank(-1) {

    }

    void MpiContext::setFromMsg(const message::Message &msg) {
        isMpi = msg.ismpi();
        mpiWorldId = msg.mpiworldid();
        mpiRank = msg.mpirank();
    }

    bool MpiContext::getIsMpi() {
        return isMpi;
    }

    int MpiContext::getMpiWorldId() {
        return mpiWorldId;
    }

    int MpiContext::getMpiRank() {
        return mpiRank;
    }

    void MpiContext::setIsMpi(bool val) {
        isMpi = val;
    }

    void MpiContext::setMpiWorldId(int val) {
        mpiWorldId = val;
    }

    void MpiContext::setMpiRank(int val){
        mpiRank = val;
    }
}