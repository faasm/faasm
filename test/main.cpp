#define CATCH_CONFIG_RUNNER

#include <catch/catch.hpp>
#include <util/util.h>


int main(int argc, char *argv[]) {
    util::initLogging();

    int result = Catch::Session().run(argc, argv);

    return result;
}