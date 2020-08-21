include_directories(${CMAKE_CURRENT_LIST_DIR})

set(LIB_FILES
        DummyStateServer.cpp
        exec_graph_utils.cpp
        message_utils.cpp
        utils.h
        )

faasm_private_lib(faabric_test_utils "${LIB_FILES}")

target_link_libraries(faabric_test_utils
    catch
    scheduler
)