#pragma once

#include <conf/FaasmConfig.h>
#include <faabric/util/config.h>
#include <faabric/util/func.h>
#include <storage/FileLoader.h>
#include <storage/S3Wrapper.h>

namespace storage {
class S3FileLoader final : public FileLoader
{
  public:
    S3FileLoader();

    std::vector<uint8_t> loadFunctionWasm(const faabric::Message& msg) override;

    std::vector<uint8_t> loadSharedObjectWasm(const std::string& path) override
    {
        throw std::runtime_error("loadSharedObjectWasm not implemented for S3");
    }

    std::vector<uint8_t> loadFunctionObjectFile(
      const faabric::Message& msg) override;

    std::vector<uint8_t> loadFunctionWamrAotFile(
      const faabric::Message& msg) override
    {
        throw std::runtime_error(
          "loadFunctionWamrAotFile not implemented for S3");
    }

    std::vector<uint8_t> loadSharedObjectObjectFile(
      const std::string& path) override
    {
        throw std::runtime_error(
          "loadSharedObjectObjectFile not implemented for S3");
    }

    std::vector<uint8_t> loadSharedFile(const std::string& path) override
    {
        throw std::runtime_error("loadSharedFile not implemented for S3");
    }

    std::vector<uint8_t> loadFunctionObjectHash(
      const faabric::Message& msg) override;

    std::vector<uint8_t> loadFunctionWamrAotHash(
      const faabric::Message& msg) override
    {
        throw std::runtime_error(
          "loadFunctionWamrAotHash not implemented for S3");
    }

    std::vector<uint8_t> loadSharedObjectObjectHash(
      const std::string& path) override
    {
        throw std::runtime_error(
          "loadSharedObjectObjectHash not implemented for S3");
    }

    void uploadFunctionObjectHash(const faabric::Message& msg,
                                  const std::vector<uint8_t>& hash) override;

    void uploadFunctionWamrAotHash(const faabric::Message& msg,
                                   const std::vector<uint8_t>& hash) override
    {
        throw std::runtime_error(
          "uploadFunctionWamrAotHash not implemented for S3");
    }

    void uploadSharedObjectObjectHash(const std::string& path,
                                      const std::vector<uint8_t>& hash) override
    {
        throw std::runtime_error(
          "uploadSharedObjectObjectHash not implemented for S3");
    }

    void uploadSharedObjectAotHash(const std::string& path,
                                   const std::vector<uint8_t>& hash) override
    {
        throw std::runtime_error(
          "uploadSharedObjectAotHash not implemented for S3");
    }

    void uploadFunction(faabric::Message& msg) override;

    void uploadPythonFunction(faabric::Message& msg) override
    {
        throw std::runtime_error(
          "uploadPythonFunction not yet implemented for S3");
    }

    void uploadFunctionObjectFile(
      const faabric::Message& msg,
      const std::vector<uint8_t>& objBytes) override;

    void uploadFunctionAotFile(const faabric::Message& msg,
                               const std::vector<uint8_t>& objBytes) override
    {
        throw std::runtime_error(
          "uploadFunctionAotFile not implemented for S3");
    }

    void uploadSharedObjectObjectFile(
      const std::string& path,
      const std::vector<uint8_t>& objBytes) override;

    void uploadSharedObjectAotFile(
      const std::string& path,
      const std::vector<uint8_t>& objBytes) override
    {
        throw std::runtime_error(
          "uploadSharedObjectAotFile not implemented for S3");
    }

    void uploadSharedFile(const std::string& path,
                          const std::vector<uint8_t>& fileBytes) override
    {
        throw std::runtime_error("uploadSharedFile not implemented for S3");
    }

    void flushFunctionFiles() override
    {
        throw std::runtime_error("flushFunctionFiles not implemented for S3");
    }

  private:
    conf::FaasmConfig& conf;
    storage::S3Wrapper s3;

    std::vector<uint8_t> loadFileBytes(const std::string& path);
};
};
