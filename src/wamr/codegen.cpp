#include "WAMRWasmModule.h"

#include <util/files.h>
#include <stdexcept>
#include <util/logging.h>

#include <wasm_export.h>
#include <aot_export.h>

#include <boost/filesystem.hpp>


namespace wasm {
    std::vector<uint8_t> wamrCodegen(std::vector<uint8_t> &wasmBytes) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        if (util::isWasm(wasmBytes)) {

        } else {
            throw std::runtime_error("File is not a wasm binary");
        }

        // Load the module
        char error_buf[128];
        wasm_module_t wasm_module = wasm_runtime_load(
                wasmBytes.data(), wasmBytes.size(),
                error_buf, sizeof(error_buf)
        );

        if (wasm_module == nullptr) {
            logger->error("Failed to import module: {}", error_buf);
            throw std::runtime_error("Failed to load module");
        }

        aot_comp_data_t comp_data = aot_create_comp_data(wasm_module);
        if(comp_data == nullptr) {
            logger->error("Failed to generat AOT data: {}", aot_get_last_error());
            throw std::runtime_error("Failed to generate AOT data");
        }

        AOTCompOption option = { 0 };

        option.opt_level = 3;
        option.size_level = 3;
        option.output_format = AOT_FORMAT_FILE;
        option.bounds_checks = 2;

        aot_comp_context_t comp_ctx =  aot_create_comp_context(comp_data, &option);
            
        if(comp_ctx == nullptr) {
            logger->error("Failed to generat AOT context: {}", aot_get_last_error());
                throw std::runtime_error("Failed to generate AOT context");
        }

        bool compileSuccess = aot_compile_wasm(comp_ctx);
        if(!compileSuccess) {
            logger->error("Failed to run codegen on wasm: {}", aot_get_last_error());
            throw std::runtime_error("Failed to run codegen");
        }

        // TODO - avoid using a temp file here
        // Note - WAMR doesn't let us generate an array of bytes, so we have to
        // write to a temp file, then load the bytes again
        boost::filesystem::path temp = boost::filesystem::unique_path();
        bool aotSuccess = aot_emit_aot_file(comp_ctx, comp_data, temp.c_str());
        if(!aotSuccess) {
            logger->error("Failed to write AOT file to {}", temp.string());
            throw std::runtime_error("Failed to emit AOT file");
        }

        // Read object bytes back in
        std::vector<uint8_t> objBytes = util::readFileToBytes(temp.c_str());
        return objBytes;
    }
}
