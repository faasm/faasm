#ifndef FAASM_POOL_H
#define FAASM_POOL_H

#include <vector>

namespace faasm {
    class FunctionPool {
    public:
        unsigned int chain(int funcIdx);

        void awaitAll();
    private:
        std::vector<unsigned int> callIds;
    };
}

#endif
