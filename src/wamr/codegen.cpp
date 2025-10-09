#include <faabric/util/files.h>
#include <faabric/util/logging.h>
#include <wamr/WAMRWasmModule.h>

#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>
#include <stdexcept>

#include <aot_export.h>
#include <wasm_export.h>

namespace wasm {
std::vector<uint8_t> wamrCodegen(std::vector<uint8_t>& wasmBytesIn, bool isSgx)
{
    // WAMR may make modifications to the byte buffer when instantiating a
    // module and generating bytecode. Thus, we take a copy here
    std::vector<uint8_t> wasmBytes = wasmBytesIn;

    SPDLOG_TRACE("Starting WAMR codegen on {} bytes", wasmBytes.size());

    // Make sure WAMR is initialised
    WAMRWasmModule::initialiseWAMRGlobally();

    if (!faabric::util::isWasm(wasmBytes)) {
        throw std::runtime_error("File is not a wasm binary");
    }

    // WAMR logging, set this to 4+ if debugging issues
    bh_log_set_verbose_level(2);

    // Load the module
    const size_t errorBufferSize = 128;
    char errorBuffer[errorBufferSize];
    memset(errorBuffer, '\0', errorBufferSize);
    // Sadly, non-pointer types are not portably provided by the WASM headers
    using wasm_module = std::pointer_traits<wasm_module_t>::element_type;
    std::unique_ptr<wasm_module, decltype(&wasm_runtime_unload)> wasmModule(
      wasm_runtime_load(
        wasmBytes.data(), wasmBytes.size(), errorBuffer, errorBufferSize),
      &wasm_runtime_unload);

    if (wasmModule == nullptr) {
        std::string tmpStr(errorBuffer, errorBufferSize);
        SPDLOG_ERROR("Failed to load wasm module: {}", tmpStr.c_str());
        throw std::runtime_error("Failed to load wasm module");
    }

    SPDLOG_TRACE("WAMR codegen imported {} bytes of wasm file",
                 wasmBytes.size());

    using aot_comp_data = std::pointer_traits<aot_comp_data_t>::element_type;
    std::unique_ptr<aot_comp_data, decltype(&aot_destroy_comp_data)>
      compileData(aot_create_comp_data(wasmModule.get(), "x86_64", false),
                  &aot_destroy_comp_data);
    if (compileData == nullptr) {
        SPDLOG_ERROR("WAMR failed to create compilation data: {}",
                     aot_get_last_error());
        throw std::runtime_error("Failed to create compilation data");
    }

    SPDLOG_TRACE("WAMR codegen generated compilation data");

    AOTCompOption option = { 0 };
    option.opt_level = 3;
    option.size_level = 3;
    option.output_format = AOT_FORMAT_FILE;
    // Switching this flag between 0 and 1 can make some WAMR generated code
    // seg-fault unexpectedly, so modify with care
    option.bounds_checks = 0;
    option.enable_bulk_memory = true;
    option.enable_bulk_memory_opt = true;
    // We need this for threads
    option.enable_thread_mgr = true;
    option.enable_ref_types = true;
    option.is_jit_mode = false;
    option.enable_simd = true;

    if (isSgx) {
        // Setting size_level = 1 sometimes gives errors during re-location
        // due to the size of the .rodata. This temporarily fixes it, but i
        // may be just a temporary workaround
        option.size_level = 0;
        option.is_sgx_platform = true;
        option.enable_thread_mgr = false;
    }

    using aot_comp_context =
      std::pointer_traits<aot_comp_context_t>::element_type;
    std::unique_ptr<aot_comp_context, decltype(&aot_destroy_comp_context)>
      compileContext(aot_create_comp_context(compileData.get(), &option),
                     &aot_destroy_comp_context);
    if (compileContext == nullptr) {
        SPDLOG_ERROR("WAMR failed to create compilation context: {}",
                     aot_get_last_error());
        throw std::runtime_error("Failed to create WAMR compilation context");
    }

    SPDLOG_TRACE("WAMR codegen created compilation context");

    bool compileSuccess = aot_compile_wasm(compileContext.get());
    if (!compileSuccess) {
        SPDLOG_ERROR("Failed to run codegen on wasm: {}", aot_get_last_error());
        throw std::runtime_error("Failed to run codegen");
    }

    SPDLOG_TRACE("WAMR codegen successfully compiled wasm");

    // TODO - avoid using a temp file here
    // WAMR doesn't let us generate an array of bytes, so we have to write to a
    // temp file, then load the bytes again
    boost::filesystem::path temp = boost::filesystem::unique_path();
    bool aotSuccess =
      aot_emit_aot_file(compileContext.get(), compileData.get(), temp.c_str());
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
