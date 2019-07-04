cmake_minimum_required(VERSION 3.0)
project(libfaasm)

set(CMAKE_CXX_STANDARD 17)

include_directories(${CMAKE_CURRENT_LIST_DIR} ${CMAKE_CURRENT_LIST_DIR}/../lib-c)

set(PUBLIC_HEADERS
        faasm/counter.h
        faasm/func.h
        faasm/matrix.h
        faasm/print.h
        faasm/random.h
        faasm/sgd.h
        faasm/time.h
        )

set(LIB_FILES
        counter.cpp
        func.cpp
        matrix.cpp
        print.cpp
        random.cpp
        sgd.cpp
        time.cpp
        )

add_library(faasm STATIC ${LIB_FILES})

target_link_libraries(faasm faasmc)

set_target_properties(faasm PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")

install(TARGETS faasm
        ARCHIVE DESTINATION ${CMAKE_SYSROOT}/lib
        LIBRARY DESTINATION ${CMAKE_SYSROOT}/lib
        RUNTIME DESTINATION ${CMAKE_SYSROOT}/bin
        PUBLIC_HEADER DESTINATION ${CMAKE_SYSROOT}/include/faasm
        )
