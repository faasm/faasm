#pragma once

#include <vector>
#include <proto/faasm.pb.h>

namespace scheduler {
    struct ExecGraphNode {
        message::Message msg;
        std::vector<ExecGraphNode> children;
    };

    struct ExecGraph {
        ExecGraphNode rootNode;
    };

    int countExecGraphNodes(const ExecGraph &graph);

    std::string execNodeToJson(const ExecGraphNode &node);

    std::string execGraphToJson(const ExecGraph &graph);
}