#include <stdio.h>
#include <iostream>
#include <omp.h>

using namespace std;

int main() {
    cout << "the begin of loop" << endl;

    #pragma omp parallel for default(none)
    for (int i = 0; i < 10; ++i) {
        printf("%i from %i\n ", i, omp_get_thread_num());
    }
    cout << endl << "the end of loop" << endl;

//    Expected output will be similar to this:
//    the begin of loop
//    6378049152
//    the end of loop
    return 0;
}
