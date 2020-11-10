#include <faasm/faasm.h>
#include <omp.h>
#include <stdio.h>

bool fail = false;
bool accessed = false; /* racy: should be atomic bool */

int main(int argc, char* argv[])
{
    int mainNum = omp_get_thread_num();
#pragma omp parallel default(none) shared(mainNum)
    {
#pragma omp master
        {
            if (accessed) {
                printf("Master section was entered multiple times\n");
                fail = true;
            }
            accessed = true;
            int localNum = omp_get_thread_num();
            if (mainNum != localNum) {
                printf("Master section not executed by master thread. "
                       "Got %d, expected %d\n",
                       localNum,
                       mainNum);
                fail = true;
            }
        }
    }

    if (fail) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
