#include <faabric/util/files.h>
#include <faabric/util/logging.h>
#include <wamr/WAMRWasmModule.h>

#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>
#include <stdexcept>
#include <type_traits>

#include <aot_export.h>
#include <wasm_export.h>

namespace wasm {
std::vector<uint8_t> wamrCodegen(std::vector<uint8_t>& wasmBytes, bool isSgx)
{
    SPDLOG_TRACE("Starting WAMR codegen on {} bytes", wasmBytes.size());

    // Make sure WAMR is initialised
    WAMRWasmModule::initialiseWAMRGlobally();

    if (!faabric::util::isWasm(wasmBytes)) {
        throw std::runtime_error("File is not a wasm binary");
    }

    // WAMR logging, set this to 4+ if debugging issues
    bh_log_set_verbose_level(2);

    // Load the module
    char errorBuffer[128];
    wasm_module_t wasmModule = wasm_runtime_load(
      wasmBytes.data(), wasmBytes.size(), errorBuffer, sizeof(errorBuffer));

    if (wasmModule == nullptr) {
        SPDLOG_ERROR("Failed to load wasm module: {}", errorBuffer);
        throw std::runtime_error("Failed to load wasm module");
    }

    SPDLOG_TRACE("WAMR codegen imported {} bytes of wasm file",
                 wasmBytes.size());

    aot_comp_data_t compData;
    if ((compData = aot_create_comp_data(wasmModule)) == nullptr) {
        SPDLOG_ERROR("WAMR failed to create compilation data: {}",
                     aot_get_last_error());
        throw std::runtime_error("Failed to create compilation data");
    }

    SPDLOG_TRACE("WAMR codegen generated compilation data");

    AOTCompOption option = { 0 };
    option.opt_level = 3;
    option.size_level = 3;
    option.output_format = AOT_FORMAT_FILE;
    option.bounds_checks = 2;
    option.is_jit_mode = false;
    option.enable_simd = false;

    if (isSgx) {
        option.size_level = 1;
        option.is_sgx_platform = true;
    }

    aot_comp_context_t compContext;
    if ((compContext = aot_create_comp_context(compData, &option)) == nullptr) {
        SPDLOG_ERROR("WAMR failed to create compilation context: {}",
                     aot_get_last_error());
        throw std::runtime_error("Failed to create WAMR compilation context");
    }

    SPDLOG_TRACE("WAMR codegen created compilation context");

    bool compileSuccess = aot_compile_wasm(compContext);
    if (!compileSuccess) {
        SPDLOG_ERROR("Failed to run codegen on wasm: {}", aot_get_last_error());
        throw std::runtime_error("Failed to run codegen");
    }

    SPDLOG_TRACE("WAMR codegen successfully compiled wasm");

    // TODO - avoid using a temp file here
    // WAMR doesn't let us generate an array of bytes, so we have to write to a
    // temp file, then load the bytes again
    boost::filesystem::path temp = boost::filesystem::unique_path();
    bool aotSuccess = aot_emit_aot_file(compContext, compData, temp.c_str());
    if (!aotSuccess) {
        SPDLOG_ERROR("Failed to write AOT file to {}", temp.string());
        throw std::runtime_error("Failed to emit AOT file");
    }

    // Read object bytes back in
    std::vector<uint8_t> objBytes =
      faabric::util::readFileToBytes(temp.c_str());

    // Delete the file
    boost::filesystem::remove(temp.c_str());

    return objBytes;
}
}
