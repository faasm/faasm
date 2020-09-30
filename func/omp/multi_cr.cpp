#import <chrono>
#include <cstdio>
#include <omp.h>
#include <random>
typedef std::chrono::system_clock::time_point TimePoint;

unsigned long thread_seed()
{
    int threadNum = omp_get_thread_num();
    return static_cast<unsigned long>(threadNum * threadNum * 77 +
                                      22 * threadNum + 1927);
}

int main(int argc, char** argv)
{
    uint32_t num_threads = 1;
    int32_t num_devices = 0;
    uint32_t num_times = 20;
    if (argc == 4) {
        num_threads = std::stoul(argv[1]);
        num_times = std::stoul(argv[2]);
        num_devices = std::stol(argv[3]);
    } else if (argc != 1) {
        printf("Usage: multi_cr [num_threads num_times num_devices]");
        return 5;
    }

    // I'm boolean impaired, this is bad even for me writing this, sorry :)
    if (num_times == 0 || num_threads == 0 ||
        (num_devices != 0 && num_devices != -1)) {
        printf("Bad args value");
        return 6;
    }

    omp_set_num_threads(num_threads);
    omp_set_default_device(num_devices);

    std::vector<uint32_t> accs;
    accs.reserve(num_times);
    for (int i = 0; i < num_times; i++) {
        TimePoint t1 = std::chrono::system_clock::now();
        int result(0);
#pragma omp parallel default(none) reduction(+ : result)
        {
            result += omp_get_thread_num();
        }
        accs.emplace_back((int64_t)result);
        TimePoint t2 = std::chrono::system_clock::now();
        long diff =
          std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();
#ifdef __wasm__
        printf("%d,Wasm local,%ld\n", num_threads, diff);
#else
        printf("%d,Native,%ld\n", num_threads, diff);
#endif
    }

    uint32_t expected = num_threads * (num_threads - 1) / 2;
    for (int i = 0; i < accs.size(); i++) {
        uint32_t acc = accs[i];
        if (acc != expected) {
            printf("Expected %d, got %d at %d\n", expected, acc, i);
            break;
        }
    }
}
