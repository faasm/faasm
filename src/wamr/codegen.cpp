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
    // Make sure WAMR is initialised
    WAMRWasmModule::initialiseWAMRGlobally();

    if (!faabric::util::isWasm(wasmBytes)) {
        throw std::runtime_error("File is not a wasm binary");
    }

    // Load the module
    char errorBuffer[128];
    // Sadly, non-pointer types are not portably provided by the WASM headers
    using wasm_module = std::pointer_traits<wasm_module_t>::element_type;
    std::unique_ptr<wasm_module, decltype(&wasm_runtime_unload)> wasmModule(
      wasm_runtime_load(
        wasmBytes.data(), wasmBytes.size(), errorBuffer, sizeof(errorBuffer)),
      &wasm_runtime_unload);

    if (wasmModule == nullptr) {
        SPDLOG_ERROR("Failed to import module: {}", errorBuffer);
        throw std::runtime_error("Failed to load module");
    }

    using aot_comp_data = std::pointer_traits<aot_comp_data_t>::element_type;
    std::unique_ptr<aot_comp_data, decltype(&aot_destroy_comp_data)>
      compileData(aot_create_comp_data(wasmModule.get()),
                  &aot_destroy_comp_data);
    if (compileData == nullptr) {
        SPDLOG_ERROR("Failed to generat AOT data: {}", aot_get_last_error());
        throw std::runtime_error("Failed to generate AOT data");
    }

    AOTCompOption option = { 0 };
    option.opt_level = 3;
    option.size_level = 3;
    option.output_format = AOT_FORMAT_FILE;
    option.bounds_checks = 2;
    option.enable_simd = true;
    option.enable_bulk_memory = false;

    if (isSgx) {
        option.size_level = 1;
        option.is_sgx_platform = true;
    }

    using aot_comp_context =
      std::pointer_traits<aot_comp_context_t>::element_type;
    std::unique_ptr<aot_comp_context, decltype(&aot_destroy_comp_context)>
      compileContext(aot_create_comp_context(compileData.get(), &option),
                     &aot_destroy_comp_context);

    if (compileContext == nullptr) {
        SPDLOG_ERROR("Failed to generat AOT context: {}", aot_get_last_error());
        throw std::runtime_error("Failed to generate AOT context");
    }

    bool compileSuccess = aot_compile_wasm(compileContext.get());
    if (!compileSuccess) {
        SPDLOG_ERROR("Failed to run codegen on wasm: {}", aot_get_last_error());
        throw std::runtime_error("Failed to run codegen");
    }

    // TODO - avoid using a temp file here
    // Note - WAMR doesn't let us generate an array of bytes, so we have to
    // write to a temp file, then load the bytes again
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
