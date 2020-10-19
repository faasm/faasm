include(FindGit)
find_package(Git)
include (ExternalProject)

include_directories(${CMAKE_INSTALL_PREFIX}/include)

ExternalProject_Add(wavm_ext
    GIT_REPOSITORY "https://github.com/faasm/WAVM.git"
    GIT_TAG "faasm"
    CMAKE_ARGS "-DDLL_EXPORT= \
        -DDLL_IMPORT="
    CMAKE_CACHE_ARGS " \
        -DWAVM_ENABLE_UNWIND=ON \
        -DCMAKE_INSTALL_PREFIX:STRING=${CMAKE_INSTALL_PREFIX}"
)

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

ExternalProject_Add(faabric_ext
    GIT_REPOSITORY "https://github.com/faasm/faabric.git"
    GIT_TAG "v0.0.8"
    CMAKE_ARGS "-DFAABRIC_BUILD_TESTS=OFF"
    CMAKE_CACHE_ARGS "-DCMAKE_INSTALL_PREFIX:STRING=${CMAKE_INSTALL_PREFIX}"
)


ExternalProject_Add(catch_ext
    GIT_REPOSITORY "https://github.com/catchorg/Catch2"
    GIT_TAG "v2.13.2"
    CMAKE_ARGS "-DCATCH_INSTALL_DOCS=OFF \
        -DCATCH_INSTALL_EXTRAS=OFF"
    CMAKE_CACHE_ARGS "-DCMAKE_INSTALL_PREFIX:STRING=${CMAKE_INSTALL_PREFIX}"
)

