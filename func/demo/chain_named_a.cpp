#include "faasm/core.h"
#include <faasm/faasm.h>
#include <vector>

int main(int argc, char* argv[])
{
    std::vector<uint8_t> inputB1 = { 0, 1, 2 };
    std::vector<uint8_t> inputB2 = { 3, 4, 5 };
    std::vector<uint8_t> inputC1 = { 6, 7 };

    unsigned int idB1 =
      faasmChainNamed("chain_named_b", inputB1.data(), 3);
    unsigned int idB2 =
      faasmChainNamed("chain_named_b", inputB2.data(), 3);
    unsigned int idC1 =
      faasmChainNamed("chain_named_c", inputC1.data(), 2);

    unsigned int resB1 = faasmAwaitCall(idB1);
    unsigned int resB2 = faasmAwaitCall(idB2);
    unsigned int resC1 = faasmAwaitCall(idC1);

    if (resB1 != 0 || resB2 != 0 || resC1 != 0) {
        return 1;
    }

    return 0;
}
