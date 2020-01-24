#include <iostream>
#include <omp.h>

using namespace std;

int main() {
    #pragma omp parallel
    {
        int ID = omp_get_thread_num();
        printf("hello  %d\n", ID);
        printf("hello again %d\n", ID);
    }
//    output:
//    hello  1
//    hello  0
//    hello  3
//    hello  2
//    hello again 1
//    hello again 0
//    hello again 3
//    hello again 2
    return 0;
}