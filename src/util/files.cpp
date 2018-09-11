#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>


namespace util {
    std::string readFileToString(const std::string &path) {
        std::ifstream stream(path);
        std::stringstream buffer;
        buffer << stream.rdbuf();
        buffer.flush();

        return buffer.str();
    }

    std::vector<uint8_t> readFileToBytes(const std::string &path) {
        std::ifstream file(path, std::ios::binary);

        // Stop eating new lines in binary mode
        file.unsetf(std::ios::skipws);

        // Reserve space
        std::streampos fileSize;
        file.seekg(0, std::ios::end);
        fileSize = file.tellg();

        std::vector<uint8_t> result;
        result.reserve(fileSize);

        // Read the data
        file.seekg(0, std::ios::beg);
        result.insert(result.begin(),
                std::istreambuf_iterator<char>(file),
                std::istreambuf_iterator<char>());

        return result;
    }

    void writeBytesToFile(const std::string &path, const std::vector<uint8_t> &data) {
        std::ofstream outfile(path, std::ios::out | std::ios::binary);
        outfile.write((char*) data.data(), data.size());
    }
}
