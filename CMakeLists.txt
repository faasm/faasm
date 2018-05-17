cmake_minimum_required(VERSION 3.5)
project(faasm)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_COMPILER /usr/bin/g++)
set(CMAKE_C_COMPILER /usr/bin/gcc)

# set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_VERBOSE_MAKEFILE ON)

set(V8_DIR /usr/local/code/v8)

#link_directories(
#        ${V8_OBJ_DIR}
#        ${V8_OBJ_DIR}/third_party/icu
#        ${V8_OBJ_DIR}/src/inspector
#)

add_executable(main main.cpp)

target_include_directories(main PUBLIC ${V8_DIR} ${V8_DIR}/include)

set(V8_BUILD_DIR ${V8_DIR}/out.gn/x64.release)
set(V8_OBJ_DIR ${V8_BUILD_DIR}/obj)
set(V8_LIBS
        ${V8_OBJ_DIR}/libv8_base.a
        ${V8_OBJ_DIR}/libv8_libbase.a
        ${V8_OBJ_DIR}/libv8_libplatform.a
        ${V8_OBJ_DIR}/libv8_external_snapshot.a
        ${V8_OBJ_DIR}/libv8_libsampler.a
        ${V8_OBJ_DIR}/third_party/icu/libicuuc.a
        ${V8_OBJ_DIR}/third_party/icu/libicui18n.a
        ${V8_OBJ_DIR}/src/inspector/libinspector.a
)

target_link_libraries(main -Wl,--start-group ${V8_LIBS} -Wl,--end-group rt dl pthread -std=c++11)

set(V8_FILES
        ${V8_BUILD_DIR}/natives_blob.bin
        ${V8_BUILD_DIR}/snapshot_blob.bin
)

foreach(f ${V8_FILES})
    file(COPY ${f} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endforeach(f)