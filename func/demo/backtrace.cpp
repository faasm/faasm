extern "C"
{
#include <faasm/host_interface.h>
}

void someFunction()
{
    __faasm_backtrace(0);
}

int main(int argc, char* argv[])
{
    someFunction();

    return 0;
}
