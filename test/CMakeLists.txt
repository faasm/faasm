add_library(catch INTERFACE)

add_executable(tests main.cpp util/test_util.cpp redis/test_redis.cpp proto/test_proto.cpp)

target_link_libraries(tests proto catch redis util)

add_test(NAME tests COMMAND "test/tests")
