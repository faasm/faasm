cmake_minimum_required(VERSION 3.0)
project(librfaasm)

# ------ Faasm Rust library ------ #

set(TARGET rfaasm)

set(CMAKE_CXX_STANDARD 17)

set(PUBLIC_HEADERS
        faasm/rfaasm.h
        )

# Create fake target since rust uses lib-cpp's libfaasm
add_custom_target(${TARGET})
set_target_properties(${TARGET} PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")

# Install public header manually since we used a fake target
if (FAASM_BUILD_TYPE STREQUAL "wasm")
    install(FILES ${PUBLIC_HEADERS} DESTINATION ${CMAKE_SYSROOT}/include/faasm)
else ()
    install(FILES ${PUBLIC_HEADER} DESTINATION include/faasm)
endif ()
