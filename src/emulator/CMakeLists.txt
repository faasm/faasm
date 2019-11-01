include_directories(${FAASM_INCLUDE_DIR}/emulator)

set(LIB_FILES
    emulator.cpp
    ${FAASM_INCLUDE_DIR}/emulator/emulator.h
)

faasm_public_lib(emulator "${LIB_FILES}")

if (FAASM_BUILD_TYPE STREQUAL "native-tools")
    target_link_libraries(emulator faasm state)
else()
    target_link_libraries(emulator state)
endif()

install(TARGETS emulator
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        PUBLIC_HEADER DESTINATION include/faasm
        )

add_executable(post_json_call post_json_call.cpp)

target_link_libraries(post_json_call util)