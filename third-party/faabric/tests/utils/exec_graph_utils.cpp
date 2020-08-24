#include <catch/catch.hpp>

#include "faabric_utils.h"

namespace tests {
    void checkExecGraphNodeEquality(const scheduler::ExecGraphNode &nodeA, const scheduler::ExecGraphNode &nodeB) {
        // Check the message itself
        checkMessageEquality(nodeA.msg, nodeB.msg);

        if (nodeA.children.size() != nodeB.children.size()) {
            FAIL(fmt::format("Children not same size: {} vs {}", nodeA.children.size(), nodeB.children.size()));
        }

        // Assume children are in same order
        for (int i = 0; i < nodeA.children.size(); i++) {
            checkExecGraphNodeEquality(nodeA.children.at(i), nodeB.children.at(i));
        }
    }

    void checkExecGraphEquality(const scheduler::ExecGraph &graphA, const scheduler::ExecGraph &graphB) {
        checkExecGraphNodeEquality(graphA.rootNode, graphB.rootNode);
    }
}


