set(FAASM_USER errors)

faasm_func(bad_malloc bad_malloc.cpp)
faasm_func(multi_poll multi_poll.cpp)
faasm_func(munmap munmap.cpp)
faasm_func(open_blocked open_blocked.cpp)
faasm_func(ret_one ret_one.cpp)
faasm_func(socket socket.cpp)
faasm_func(stdin_poll stdin_poll.cpp)

faasm_func(pthread_creation pthread_creation.cpp)

if (FAASM_BUILD_TYPE STREQUAL "wasm")
    faasm_func(stackoverflow stackoverflow.cpp)
endif()