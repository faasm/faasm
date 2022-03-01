set(CPU_VENDOR)
set(CPU_FAMILY)
set(CPU_MODEL)
set(CPU_PROPS)

file(READ "/proc/cpuinfo" _cpuinfo)
string(REGEX REPLACE ".*vendor_id[ \t]*:[ \t]+([a-zA-Z0-9_-]+).*" "\\1"
    CPU_VENDOR "${_cpuinfo}")
string(REGEX REPLACE ".*cpu family[ \t]*:[ \t]+([a-zA-Z0-9_-]+).*" "\\1"
    CPU_FAMILY "${_cpuinfo}")
string(REGEX REPLACE ".*model[ \t]*:[ \t]+([a-zA-Z0-9_-]+).*" "\\1"
    CPU_MODEL "${_cpuinfo}")
string(REGEX REPLACE ".*flags[ \t]*:[ \t]+([^\n]+).*" "\\1"
    CPU_PROPS "${_cpuinfo}")

message("Found CPU: Vendor = ${CPU_VENDOR} Family = ${CPU_FAMILY} Model = ${CPU_MODEL} Props = ${CPU_PROPS}")

# See these files for examples of extending this list:
# https://github.com/VcDevel/Vc/blob/1.4/cmake/OptimizeForArchitecture.cmake
# https://github.com/scivision/cmake-cpu-detect/blob/main/DetectHostArch.cmake
# See the LLVM source for list of supported arch, e.g. this test:
# https://github.com/llvm/llvm-project/blob/main/clang/test/Driver/x86-march.c

set(CPU_COMPILE_FLAGS)
if(CPU_VENDOR STREQUAL "GenuineIntel")
    if(CPU_FAMILY EQUAL 6)
        if(CPU_MODEL EQUAL 58 OR CPU_MODEL EQUAL 62)
            # Ivy bridge
            set(CPU_COMPILE_FLAGS -march=ivybridge -mtune=ivybridge)
        elseif(CPU_MODEL EQUAL 78 OR CPU_MODEL EQUAL 94)
            # Skylake
            set(CPU_COMPILE_FLAGS -march=skylake -mtune=skylake)
        elseif(CPU_MODEL EQUAL 106 OR CPU_MODEL EQUAL 108)
            # Icelake server
            set(CPU_COMPILE_FLAGS -march=icelake-server -mtune=icelake-server)
        elseif(CPU_MODEL EQUAL 126 OR CPU_MODEL EQUAL 125)
            # Icelake client
            set(CPU_COMPILE_FLAGS -march=icelake-client -mtune=icelake-client)
        endif()
    endif()
endif()

if(CPU_COMPILE_FLAGS)
    message("Setting following CPU-specific compile flags: ${CPU_COMPILE_FLAGS}")
    add_compile_options(${CPU_COMPILE_FLAGS})
else()
    message("Could not provide any CPU-specific compile flags")
endif()
