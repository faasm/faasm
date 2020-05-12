#include <catch/catch.hpp>

#include "utils.h"

#include <util/func.h>

namespace tests {

    TEST_CASE("Test static for scheduling", "[wasm][openmp]") {
        cleanSystem();
        message::Message msg = util::messageFactory("omp", "for_static_schedule");
        execFunction(msg);
    }

    TEST_CASE("Test OMP header API functions", "[wasm][openmp]") {
        cleanSystem();
        message::Message msg = util::messageFactory("omp", "header_api_support");
        execFunction(msg);
    }

    TEST_CASE("Test running OpenMP checks", "[wasm][openmp]") {
        cleanSystem();
        message::Message msg = util::messageFactory("omp", "omp_checks");
        execFunction(msg);
    }

    TEST_CASE("Test non-nested barrier pragma", "[wasm][openmp]") {
        cleanSystem();
        message::Message msg = util::messageFactory("omp", "simple_barrier");
        execFunction(msg);
    }

    TEST_CASE("Test basic omp parallel for pragma", "[wasm][openmp]") {
        cleanSystem();
        message::Message msg = util::messageFactory("omp", "simple_for");
        execFunction(msg);
    }

    TEST_CASE("Test non-nested master pragma", "[wasm][openmp]") {
        cleanSystem();
        message::Message msg = util::messageFactory("omp", "simple_master");
        execFunction(msg);
    }

    TEST_CASE("Test thread stack mapping", "[wasm][openmp]") {
        cleanSystem();
        message::Message msg = util::messageFactory("omp", "stack_debug");
        execFunction(msg);
    }

    TEST_CASE("Test simple reduction function", "[wasm][openmp]") {
        cleanSystem();
        message::Message msg = util::messageFactory("omp", "simple_reduce");
        execFunction(msg);
    }

    TEST_CASE("Test averaging with different methods (atomic RR and reduction)", "[wasm][openmp]") {
        cleanSystem();
        message::Message msg = util::messageFactory("omp", "reduction_average");
        execFunction(msg);
    }

    TEST_CASE("Test integrating using many OpenMP constructs", "[wasm][openmp]") {
        cleanSystem();
        message::Message msg = util::messageFactory("omp", "reduction_integral");
        execFunction(msg);
    }

    TEST_CASE("Test critical section", "[wasm][openmp]") {
        cleanSystem();
        message::Message msg = util::messageFactory("omp", "simple_critical");
        execFunction(msg);
    }

    TEST_CASE("Test single section", "[wasm][openmp]") {
        cleanSystem();
        message::Message msg = util::messageFactory("omp", "simple_single");
        execFunction(msg);
    }

    TEST_CASE("Test custom reduction function", "[wasm][openmp]") {
        cleanSystem();
        message::Message msg = util::messageFactory("omp", "custom_reduce");
        execFunction(msg);
    }

    TEST_CASE("Test similar nested API than Clang 9.0.1", "[wasm][openmp]") {
        cleanSystem();
        message::Message msg = util::messageFactory("omp", "nested_levels_test");
        execFunction(msg);
    }

    TEST_CASE("Test nested parallel region support", "[wasm][openmp]") {
        cleanSystem();
        message::Message msg = util::messageFactory("omp", "nested_parallel");
        execFunction(msg);
    }

    TEST_CASE("Test proper handling of getting and setting next level num threads", "[wasm][openmp]") {
        cleanSystem();
        message::Message msg = util::messageFactory("omp", "setting_num_threads");
        execFunction(msg);
    }
}