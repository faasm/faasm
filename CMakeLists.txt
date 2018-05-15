cmake_minimum_required(VERSION 3.10)
project(faasm)

set(CMAKE_CXX_STANDARD 14)

set(V8_DIR /usr/local/code/v8)
set(V8_BUILD_DIR ${V8_DIR}/out.gn/x64.release)
set(V8_OBJ_DIR ${V8_BUILD_DIR}/obj)


add_executable(main main.cpp)

target_include_directories(main PUBLIC ${V8_DIR} ${V8_DIR}/include)

#link_directories(
#        ${V8_OBJ_DIR}
#        ${V8_OBJ_DIR}/third_party/icu
#        ${V8_OBJ_DIR}/src/inspector
#)

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

target_link_libraries(main ${V8_LIBS} rt dl pthread)

set(V8_FILES
        ${V8_BUILD_DIR}/natives_blob.bin
        ${V8_BUILD_DIR}/snapshot_blob.bin
        ${V8_BUILD_DIR}/snapshot_blob_trusted.bin
        )

foreach(f ${V8_FILES})
    file(COPY ${f} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endforeach(f)