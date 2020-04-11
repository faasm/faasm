add_library(fakeLibB SHARED libB.h libB.c)
set_target_properties(fakeLibB PROPERTIES PUBLIC_HEADER libB.h)

target_link_libraries(fakeLibB fakeLibA)

if(FAASM_BUILD_TYPE STREQUAL "wasm")
    install(TARGETS fakeLibB
            ARCHIVE DESTINATION lib/wasm32-wasi
            LIBRARY DESTINATION lib/wasm32-wasi
            PUBLIC_HEADER DESTINATION include
            )
else()
    install(TARGETS fakeLibB
            ARCHIVE DESTINATION lib
            LIBRARY DESTINATION lib
            PUBLIC_HEADER DESTINATION include
            )
endif()
