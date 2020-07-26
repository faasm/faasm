#pragma once

#include <vector>
#include <proto/faasm.pb.h>

namespace scheduler {
    class ExecGraphNode {
    public:
        ExecGraphNode() = default;

        message::Message msg;
        std::vector<ExecGraphNode> children;
    };

    class ExecGraph {
    public:
        ExecGraph() = default;

        ExecGraphNode rootNode;
    };
}