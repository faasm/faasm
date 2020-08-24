#include <catch/catch.hpp>

#include "utils.h"

namespace faabric {
    namespace tests {
        void checkMessageEquality(const faabric::Message &msgA, const faabric::Message &msgB) {
            REQUIRE(msgA.id() == msgB.id());

            REQUIRE(msgA.user() == msgB.user());
            REQUIRE(msgA.function() == msgB.function());
            REQUIRE(msgA.idx() == msgB.idx());
            REQUIRE(msgA.hops() == msgB.hops());
            REQUIRE(msgA.executedhost() == msgB.executedhost());
            REQUIRE(msgA.finishtimestamp() == msgB.finishtimestamp());

            REQUIRE(msgA.timestamp() == msgB.timestamp());
            REQUIRE(msgA.snapshotkey() == msgB.snapshotkey());
            REQUIRE(msgA.snapshotsize() == msgB.snapshotsize());
            REQUIRE(msgA.funcptr() == msgB.funcptr());

            REQUIRE(msgA.pythonuser() == msgB.pythonuser());
            REQUIRE(msgA.pythonfunction() == msgB.pythonfunction());
            REQUIRE(msgA.pythonentry() == msgB.pythonentry());
            REQUIRE(msgA.isasync() == msgB.isasync());
            REQUIRE(msgA.ispython() == msgB.ispython());
            REQUIRE(msgA.istypescript() == msgB.istypescript());
            REQUIRE(msgA.isstatusrequest() == msgB.isstatusrequest());
            REQUIRE(msgA.isexecgraphrequest() == msgB.isexecgraphrequest());
            REQUIRE(msgA.isflushrequest() == msgB.isflushrequest());

            REQUIRE(msgA.returnvalue() == msgB.returnvalue());

            REQUIRE(msgA.inputdata() == msgB.inputdata());
            REQUIRE(msgA.outputdata() == msgB.outputdata());

            REQUIRE(msgA.resultkey() == msgB.resultkey());
            REQUIRE(msgA.statuskey() == msgB.statuskey());
            REQUIRE(msgA.coldstartinterval() == msgB.coldstartinterval());
            REQUIRE(msgA.type() == msgB.type());

            REQUIRE(msgA.ismpi() == msgB.ismpi());
            REQUIRE(msgA.mpiworldid() == msgB.mpiworldid());
            REQUIRE(msgA.mpirank() == msgB.mpirank());
            REQUIRE(msgA.mpiworldsize() == msgB.mpiworldsize());

            REQUIRE(msgA.cmdline() == msgB.cmdline());

            REQUIRE(msgA.issgx() == msgB.issgx());
        }
    }
}


