#include <enclave/outside/getSgxSupport.h>

int main()
{
    return !sgx::isSgxEnabled();
}
