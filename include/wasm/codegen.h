#pragma once

#include <string>

namespace wasm {
    void codeGenForFile(const std::string &inputFile);

    void codeGenForDir(const std::string &dirPath);
}