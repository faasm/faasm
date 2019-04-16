add_library(fakeLibB SHARED libB.h libB.c)
set_target_properties(fakeLibB PROPERTIES PUBLIC_HEADER libB.h)

target_link_libraries(fakeLibB fakeLibA)

install(TARGETS fakeLibB
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        PUBLIC_HEADER DESTINATION include
        )