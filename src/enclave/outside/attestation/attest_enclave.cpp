// #include <enclave/outside/attestation/generateQuote.h>
#include <cstdlib>

int generateQuote(int enclaveId);

/*
 * The attestation mechanism checks that the enclave has been booted in an
 * on-premise, genuine, SGX enclave, and that the code therein loaded (i.e.
 * WAMR runtime and the sources in the src/enclave/inside folder) has not been
 * tampered with. It takes four arguments: TODO finish
 */
int main(int argc, char* argv[])
{
    if (argc < 2) {
        return 1;
    }
    
    int enclaveId = atoi(argv[1]);
    generateQuote(enclaveId);

    return 0;
}
