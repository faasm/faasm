#include <stdio.h>
#include <string>
#include <vector>


int main(int argc, char **argv) {
    if (argc != 5) {
        printf("Expected 5 argc but got %i\n", argc);
        return 1;
    }

    std::vector<std::string> expectedArgs;
    expectedArgs.emplace_back("function.wasm");
    expectedArgs.emplace_back("alpha");
    expectedArgs.emplace_back("B_eta");
    expectedArgs.emplace_back("G$mma");
    expectedArgs.emplace_back("d3-lt4");

    for (int i = 0; i < argc; ++i) {
        std::string actual(argv[i]);
        if (actual != expectedArgs[i]) {
            printf("Argv %i - expected %s but got %s\n", i, expectedArgs[i].c_str(), actual.c_str());
            return 1;
        }
    }

    return 0;
}