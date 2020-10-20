include(FindGit)
find_package(Git)
include (ExternalProject)
include (FetchContent)

include_directories(${CMAKE_INSTALL_PREFIX}/include)

# Tightly-coupled dependencies
FetchContent_Declare(wavm_ext
    GIT_REPOSITORY "https://github.com/faasm/WAVM.git"
    GIT_TAG "faasm"
    CMAKE_ARGS "-DDLL_EXPORT= \
        -DDLL_IMPORT= \
        -DWAVM_ENABLE_UNWIND=ON"
)

FetchContent_Declare(faabric_ext
    GIT_REPOSITORY "https://github.com/faasm/faabric.git"
    GIT_TAG "v0.0.10"
    CMAKE_ARGS "-DFAABRIC_BUILD_TESTS=OFF"
)

FetchContent_MakeAvailable(wavm_ext faabric_ext)

# Need access to private WAVM headers
FetchContent_GetProperties(wavm_ext SOURCE_DIR FAASM_WAVM_SOURCE_DIR)
message(STATUS FAASM_WAVM_SOURCE_DIR ${FAASM_WAVM_SOURCE_DIR})

# General 3rd party dependencies
ExternalProject_Add(eigen_ext
    GIT_REPOSITORY "https://gitlab.com/shillaker/eigen"
    GIT_TAG "faasm"
    CMAKE_CACHE_ARGS "-DCMAKE_INSTALL_PREFIX:STRING=${CMAKE_INSTALL_PREFIX}"
)

ExternalProject_Add(cereal_ext
    GIT_REPOSITORY "https://github.com/USCiLab/cereal"
    GIT_TAG "v1.3.0"
    CMAKE_ARGS "-DJUST_INSTALL_CEREAL=1"
    CMAKE_CACHE_ARGS "-DCMAKE_INSTALL_PREFIX:STRING=${CMAKE_INSTALL_PREFIX}"
)

ExternalProject_Add(catch2_ext
     GIT_REPOSITORY "https://github.com/catchorg/Catch2"
     GIT_TAG "v2.13.2"
     CMAKE_ARGS "-DCATCH_INSTALL_DOCS=OFF \
         -DCATCH_INSTALL_EXTRAS=OFF"
     CMAKE_CACHE_ARGS "-DCMAKE_INSTALL_PREFIX:STRING=${CMAKE_INSTALL_PREFIX}"
)
 
