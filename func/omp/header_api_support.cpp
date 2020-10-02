#include <faasm/faasm.h>
#include <omp.h>
#include <stdio.h>

bool failed = false;

struct ThreadInfo
{
    int32_t thread_num;
    int32_t local_thread_num;
} __attribute__((aligned(64)));

int main(int argc, char* argv[])
{
    int max = omp_get_max_threads();
    auto infos = new ThreadInfo[max];

#pragma omp parallel num_threads(max) default(none) shared(infos)
    {
        int thread_num = omp_get_thread_num();
        infos[thread_num].thread_num = thread_num;
        infos[thread_num].local_thread_num = omp_get_num_threads();
    }

    for (int i = 0; i < max; i++) {
        auto thread = infos[i];
        if (thread.thread_num != i) {
            printf("Expected thread_num to be %d but got %d\n",
                   i,
                   infos[i].thread_num);
            failed = true;
        }
        if (thread.local_thread_num != max) {
            printf("Expected local_thread_num to be %d but got %d\n",
                   max,
                   thread.local_thread_num);
            failed = true;
        }
    }

    delete[] infos;
    if (failed) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
