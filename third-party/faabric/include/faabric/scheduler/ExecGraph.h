#pragma once

#include <vector>
#include <proto/faabric.pb.h>

namespace faabric::scheduler {
    struct ExecGraphNode {
        faabric::Message msg;
        std::vector<ExecGraphNode> children;
    };

    struct ExecGraph {
        ExecGraphNode rootNode;
    };

    int countExecGraphNodes(const ExecGraph &graph);

    std::string execNodeToJson(const ExecGraphNode &node);

    std::string execGraphToJson(const ExecGraph &graph);
}