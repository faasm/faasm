include_directories(${CMAKE_CURRENT_LIST_DIR})

set(LIB_FILES
        DummyStateServer.cpp
        exec_graph_utils.cpp
        matrix_utils.cpp
        message_utils.cpp
        system_utils.cpp
        worker_utils.cpp
        utils.h
        )

faasm_private_lib(test_utils "${LIB_FILES}")

target_link_libraries(test_utils
        catch
        faasm
        upload_lib
        faaslet_lib
        emulator
        )