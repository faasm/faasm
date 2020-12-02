set(PUBLIC_HEADERS ../include/sharedHeader.h libA.h)

add_library(fakeLibA SHARED ${PUBLIC_HEADERS} libA.c)
set_target_properties(fakeLibA PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")

if (CMAKE_SYSTEM_NAME STREQUAL "Wasm")
    install(TARGETS fakeLibA
            ARCHIVE DESTINATION lib/wasm32-wasi
            LIBRARY DESTINATION lib/wasm32-wasi
            PUBLIC_HEADER DESTINATION include
            )
else()
    install(TARGETS fakeLibA
            ARCHIVE DESTINATION lib
            LIBRARY DESTINATION lib
            PUBLIC_HEADER DESTINATION include
            )
endif()
