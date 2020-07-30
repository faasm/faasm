include_directories(${FAASM_INCLUDE_DIR}/arrow_state)

# Minimal Arrow CMake example:
# https://github.com/apache/arrow/blob/master/cpp/examples/minimal_build/CMakeLists.txt
find_package(Arrow REQUIRED)
message(STATUS "Found Arrow ${ARROW_VERSION}")

set(PUBLIC_HEADERS
        ${FAASM_INCLUDE_DIR}/arrow_state/ArrowMemoryWrapper.h
        )

set(LIB_FILES
        ArrowMemoryWrapper.cpp
        )

faasm_private_lib(arrow "${LIB_FILES}")

# Link the shared Arrow library
target_link_libraries(arrow PRIVATE
        util
        arrow_shared
        )
set_target_properties(arrow PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")

# Dummy executable
add_executable(arrow_poc arrow_poc.cpp)
target_link_libraries(arrow_poc arrow)
