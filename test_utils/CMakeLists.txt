include_directories(${CMAKE_CURRENT_LIST_DIR})

set(LIB_FILES
        matrix_utils.cpp
        message_utils.cpp
        system_utils.cpp
        worker_utils.cpp
        utils.h
        )

faasm_private_lib(test_utils "${LIB_FILES}")

target_link_libraries(test_utils catch faasm edge_lib upload_lib worker_lib emulator data worker_knative_lib)
