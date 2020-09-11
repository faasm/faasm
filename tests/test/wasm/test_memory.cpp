#include <catch/catch.hpp>
#include <wavm/WAVMWasmModule.h>
#include <faabric/util/bytes.h>
#include <faabric/util/func.h>
#include <faabric/util/config.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <faabric/util/files.h>

using namespace WAVM;

namespace tests {
    TEST_CASE("Test mmapping a file", "[wasm]") {
        faabric::Message call;
        call.set_user("demo");
        call.set_function("echo");

        wasm::WAVMWasmModule module;
        module.bindToFunction(call);

        // File we know to exist
        std::string fileName = "/usr/local/faasm/llvm-sysroot/include/sys/mman.h";
        int fd = open(fileName.c_str(), O_RDONLY);
        if (fd == -1) {
            FAIL("Could not open file");
        }

        struct stat sb{};
        fstat(fd, &sb);
        size_t bufferSize = sb.st_size;

        U32 mappedWasmPtr = module.mmapFile(fd, bufferSize);
        U8 *hostPtr = &Runtime::memoryRef<U8>(module.defaultMemory, mappedWasmPtr);
        
        // Get a section of bytes from the start
        int byteLen = 500;
        std::vector<U8> actual(hostPtr, hostPtr + byteLen);
        
        // Close the file
        close(fd);
        
        // Read in by other means
        std::vector<uint8_t> full = faabric::util::readFileToBytes(fileName);
        std::vector<uint8_t> expected(full.data(), full.data() + byteLen);

        // Check the bytes match
        REQUIRE(expected == actual);
    }
}
