# Set up catch library
add_library(catch INTERFACE)

# Make test executable
add_executable(tests main.cpp util/test_util.cpp redis/test_redis.cpp util/test_messaging.cpp)

target_link_libraries(tests catch redis cpp_redis tacopie util)

add_test(NAME tests COMMAND "test/tests")
