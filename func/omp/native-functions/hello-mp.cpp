#include <iostream>
#include <stdio.h>
#include <omp.h>

using namespace std;

int main() {
    cout << "Main before" << endl;

    // OMP parallel section
    #pragma omp parallel default(none)
    {
        int i = omp_get_thread_num();
        printf("%i\n", i);
    }

    cout << "Main after" << endl;
    return 0;
}
