#include <cstdio>
#include <chrono>
#include <fstream>
#include <vector>
#include <sys/stat.h>

constexpr auto BINARY_DIR = "/usr/bin/";

void
nativeRun(std::ofstream &profOut, const std::string &execPath, long num_iterations, const std::string &iteration_name,
          int num_threads);

int main(int argc, char *argv[]) {

    if (argc != 3) {
        printf("Usage:\ndocker_benchmarker <function> <nNative>\n");
        return 1;
    }

    std::string funcName = argv[1];
    int nativeIterations = std::stoi(argv[2]);

    std::string outfile = std::string("/wasm/omp/") + funcName + "/bench.csv";

    std::ofstream profOut;
    profOut.open(outfile);
    profOut << "iterations,numThreads,type,microseconds" << std::endl;

    std::string execPath = std::string(BINARY_DIR) + funcName;

    std::vector<long> iterations = {200000l, 20000000l, 200000000l};
    std::vector<std::string> iter_names = {"Tiny,", "Small,", "Big,"};

    for (int run = 1; run <= nativeIterations; run++) {
        printf("NATIVE - %s (%d/%d)\n", funcName.c_str(), run, nativeIterations);
        for (size_t i = 0; i < iterations.size(); i++) {
            nativeRun(profOut, execPath, iterations[i], iter_names[i], 1);
            for (int num_threads = 2; num_threads < 25; num_threads += 2) {
                nativeRun(profOut, execPath, iterations[i], iter_names[i], num_threads);
            }
        }
    }

    profOut.flush();
    profOut.close();

    chmod(outfile.c_str(),S_IRWXU | S_IRWXG | S_IRWXO);

    return 0;
}


void
nativeRun(std::ofstream &profOut, const std::string &execPath, long num_iterations, const std::string &iteration_name,
          int num_threads) {

    char buffer [128];
    sprintf(buffer, "%s %d %ld", execPath.c_str(), num_threads, num_iterations);

    std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
    int error = system(buffer);
    const int nativeTime = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - t1).count();

    if (error != 0) {
        printf("Failed to execute %s", buffer);
    }

    profOut << iteration_name << num_threads << ",native," << nativeTime << std::endl;
}
