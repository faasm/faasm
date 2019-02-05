#include "S3FunctionLoader.h"

#include <util/func.h>
#include <util/bytes.h>
#include <util/files.h>

namespace wasm {
    S3FunctionLoader::S3FunctionLoader() : conf(util::getSystemConfig()), s3(awswrapper::S3Wrapper::getThreadLocal()) {

    }

    std::vector<uint8_t> S3FunctionLoader::loadFunctionBytes(const message::Message &msg) {
        const std::string &funcPath = util::getFunctionFile(msg);
        const std::vector<uint8_t> &bytes = s3.getKeyBytes(conf.bucketName, funcPath);
        
        return bytes;
    }

    std::vector<uint8_t> S3FunctionLoader::loadFunctionObjectBytes(const message::Message &msg) {
        const std::string &objPath = util::getFunctionObjectFile(msg);
        const std::vector<uint8_t> &bytes = s3.getKeyBytes(conf.bucketName, objPath);

        return bytes;
    }

    void S3FunctionLoader::uploadFunction(message::Message &msg) {
        // Note, when uploading, the input data is the function body
        const std::string &inputBytes = msg.inputdata();

        const std::string &funcPath = util::getFunctionFile(msg);
        s3.addKeyStr(conf.bucketName, funcPath, inputBytes);
    }

    void S3FunctionLoader::uploadObjectBytes(const message::Message &msg, const std::vector<uint8_t> &objBytes) {
        const std::string objPath = util::getFunctionObjectFile(msg);
        s3.addKeyBytes(conf.bucketName, objPath, objBytes);
    }

}
