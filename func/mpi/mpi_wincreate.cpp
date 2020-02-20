#include <stdio.h>
#include <mpi.h>
#include <faasm/faasm.h>
#include <faasm/compare.h>
#include <string>


#define NUM_ELEMENT 4

bool checkIntAttr(MPI_Win window, int attr, long expected, const std::string &name) {
    void *resPtr;
    int flag;
    MPI_Win_get_attr(window, attr, (void *) &resPtr, &flag);

    MPI_Aint actual = *reinterpret_cast<MPI_Aint *>(resPtr);

    if (actual != expected || flag != 1) {
        printf("%s not as expected (%li != %li (%i))\n", name.c_str(), actual, expected, flag);
        return false;
    }

    return true;
}


FAASM_MAIN_FUNC() {
    MPI_Init(NULL, NULL);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int dataSize = sizeof(int);
    int sharedData[NUM_ELEMENT];
    int expectedPutData[NUM_ELEMENT];

    // Populate some data in shared region and not
    for (int i = 0; i < NUM_ELEMENT; i++) {
        sharedData[i] = 10 * rank + i;
        expectedPutData[i] = 10 * rank + i;
    }

    // Create a window
    MPI_Win window;
    int winSize = NUM_ELEMENT * dataSize;
    MPI_Win_create(sharedData, winSize, 1, MPI_INFO_NULL, MPI_COMM_WORLD, &window);

    // Check that the memory has not been corrupted (happens when pointer to MPI_Win is handled wrongly)
    if (rank < 3) {
        bool putDataEqual = faasm::compareIntArrays(sharedData, expectedPutData, NUM_ELEMENT);

        if (!putDataEqual) {
            printf("Rank %i - stack corrupted by win_create\n", rank);
            return 1;
        } else if (rank < 3) {
            printf("Rank %i - stack OK\n", rank);
        }
    }

    if (rank == 0) {
        // Check base of window
        void *actualBase;
        int baseFlag;
        MPI_Win_get_attr(window, MPI_WIN_BASE, &actualBase, &baseFlag);
        if (actualBase != sharedData || baseFlag != 1) {
            printf("MPI_WIN_BASE not as expected (%p != %p (%i))\n", actualBase, sharedData, baseFlag);
            return 1;
        }

        // Check size of window
        checkIntAttr(window, MPI_WIN_SIZE, winSize, "MPI_WIN_SIZE");
        checkIntAttr(window, MPI_WIN_DISP_UNIT, 1, "MPI_WIN_DISP_UNIT");

        printf("Win attr checks complete\n");
    }

    MPI_Win_free(&window);
    MPI_Finalize();

    return 0;
}
