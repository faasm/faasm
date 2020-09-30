#include <atomic>
#include <stdexcept>

std::atomic_bool test = false;

void run_once_only()
{
    if (test == true) {
        throw std::runtime_error("Thingy was set");
    }
    test = true;
}

int main()
{

#pragma omp parallel num_threads(4)
#pragma omp single nowait
    run_once_only();
}
