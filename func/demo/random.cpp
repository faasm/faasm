#include "faasm/faasm.h"

#include <algorithm>
#include <vector>
#include <random>
#include <chrono>

namespace faasm {
    int exec(FaasmMemory *memory) {
        int n = 10;
        std::vector<int> vec(n);

        for (int i = 0; i < n; i++) {
            vec.at(i) = i;
        }

        long seed = std::chrono::system_clock::now().time_since_epoch().count();
        auto rng = std::default_random_engine(seed);
        std::shuffle(vec.begin(), vec.end(), rng);

        printf("Vector elements %i %i %i\n", vec.at(0), vec.at(1), vec.at(2));

        return 0;
    }
}
