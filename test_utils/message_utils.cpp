#include <catch/catch.hpp>

#include "utils.h"

using namespace faasm;

namespace tests {
    void checkMessageEquality(const message::Message &msgA, const message::Message &msgB) {
        REQUIRE(msgA.id() == msgB.id());

        REQUIRE(msgA.user() == msgB.user());
        REQUIRE(msgA.function() == msgB.function());
        REQUIRE(msgA.idx() == msgB.idx());
        REQUIRE(msgA.hops() == msgB.hops());
        REQUIRE(msgA.pythonuser() == msgB.pythonuser());
        REQUIRE(msgA.pythonfunction() == msgB.pythonfunction());
        REQUIRE(msgA.pythonidx() == msgB.pythonidx());
        REQUIRE(msgA.isasync() == msgB.isasync());
        REQUIRE(msgA.ispython() == msgB.ispython());
        REQUIRE(msgA.istypescript() == msgB.istypescript());
        REQUIRE(msgA.isstatusrequest() == msgB.isstatusrequest());
        REQUIRE(msgA.isflushrequest() == msgB.isflushrequest());
        REQUIRE(msgA.success() == msgB.success());
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
    }
}

