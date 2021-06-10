#include "FileserverFileLoader.h"

#include <conf/FaasmConfig.h>
#include <conf/function_utils.h>

#include <faabric/util/bytes.h>
#include <faabric/util/config.h>
#include <faabric/util/files.h>
#include <faabric/util/func.h>
#include <faabric/util/logging.h>

#include <boost/filesystem.hpp>
#include <cpprest/astreambuf.h>
#include <cpprest/containerstream.h>
#include <cpprest/filestream.h>
#include <cpprest/http_client.h>
#include <cpprest/json.h>
#include <cpprest/uri.h>

#include <iostream>

using namespace utility;
using namespace web;
using namespace web::http;
using namespace web::http::client;
using namespace concurrency::streams;

namespace storage {

FileserverFileLoader::FileserverFileLoader(bool useFilesystemCacheIn)
  : useFilesystemCache(useFilesystemCacheIn)
{}

std::string FileserverFileLoader::getFileserverUrl()
{
    return conf::getFaasmConfig().fileserverUrl;
}

std::vector<uint8_t> FileserverFileLoader::doLoad(
  const std::string& urlPath,
  const std::string& headerPath,
  const std::string& storagePath)
{

    // Shortcut if already exists
    if (useFilesystemCache && boost::filesystem::exists(storagePath)) {
        if (boost::filesystem::is_directory(storagePath)) {
            throw SharedFileIsDirectoryException(storagePath);
        } else {
            SPDLOG_DEBUG("Loading from filesystem at {}", storagePath);
            return faabric::util::readFileToBytes(storagePath);
        }
    }

    conf::FaasmConfig& conf = conf::getFaasmConfig();
    std::string host = conf.fileserverUrl;

    SPDLOG_DEBUG("Creating client at {}", host);
    http_client client(U(host.c_str()));

    // Build the request
    uri_builder builder;
    builder.set_path(urlPath, false);
    http_request request(methods::GET);
    request.set_request_uri(builder.to_uri());

    // Add header if necessary
    if (!headerPath.empty()) {
        request.headers().add(FILE_PATH_HEADER, headerPath);
    }

    std::vector<uint8_t> bytesData;
    client.request(request)
      .then([](http_response response) {
          if (response.status_code() != 200) {
              SPDLOG_ERROR("GET request for file failed: {}",
                           response.status_code());
              throw std::runtime_error("File GET request failed");
          }

          return response.extract_vector();
      })
      .then([&](std::vector<uint8_t> responseData) {
          size_t nBytes = responseData.size();

          // Check response data
          if (nBytes == 0) {
              std::string errMsg =
                "Empty response for file at " + host + "/" + urlPath;
              SPDLOG_ERROR(errMsg);
              throw conf::InvalidFunctionException(errMsg);
          }

          // Check whether it's a directory
          // Note - we don't want to convert every file response to a string, so
          // check the length first
          std::string isDirResponse = IS_DIR_RESPONSE;
          if (nBytes == isDirResponse.size()) {
              std::string actualResp =
                faabric::util::bytesToString(responseData);
              if (actualResp == IS_DIR_RESPONSE) {
                  throw SharedFileIsDirectoryException(headerPath);
              }
          }

          // Write bytes to file
          faabric::util::writeBytesToFile(storagePath, responseData);

          // Copy out the data
          // TODO avoid this copy?
          bytesData = responseData;
      })
      .wait();

    return bytesData;
}

std::vector<uint8_t> FileserverFileLoader::loadFunctionWasm(
  const faabric::Message& msg)
{
    std::string urlPath = fmt::format("f/{}/{}", msg.user(), msg.function());
    std::string filePath = conf::getFunctionFile(msg);
    return doLoad(urlPath, "", filePath);
}

std::vector<uint8_t> FileserverFileLoader::loadFunctionObjectFile(
  const faabric::Message& msg)
{
    std::string urlPath = fmt::format("fo/{}/{}", msg.user(), msg.function());
    std::string objectFilePath = conf::getFunctionObjectFile(msg);
    return doLoad(urlPath, "", objectFilePath);
}

std::vector<uint8_t> FileserverFileLoader::loadFunctionWamrAotFile(
  const faabric::Message& msg)
{
    throw std::runtime_error(
      "Not yet implemented WAMR AOT file loading from fileserver");
}

std::vector<uint8_t> FileserverFileLoader::loadSharedObjectObjectFile(
  const std::string& path)
{
    std::string urlPath = "sobjobj";
    std::string objFilePath = conf::getSharedObjectObjectFile(path);
    return doLoad(urlPath, path, objFilePath);
}

std::vector<uint8_t> FileserverFileLoader::loadSharedObjectWasm(
  const std::string& path)
{
    std::string urlPath = "sobjwasm";
    return doLoad(urlPath, path, path);
}

std::vector<uint8_t> FileserverFileLoader::loadSharedFile(
  const std::string& path)
{
    std::string urlPath = "file";
    const std::string fullPath = conf::getSharedFileFile(path);
    return doLoad(urlPath, path, fullPath);
}

void FileserverFileLoader::flushFunctionFiles()
{
    // Note that because we're loading files from a file server, we can safely
    // delete the function and object files on this host as they are not the
    // master copies.
    conf::FaasmConfig& conf = conf::getFaasmConfig();

    // Nuke the function directory
    boost::filesystem::remove_all(conf.functionDir);

    // Nuke the machine code directory
    boost::filesystem::remove_all(conf.objectFileDir);
}

// --------------------------------------------------------
// TODO: implement hashing and hash storage in fileserver mode
// --------------------------------------------------------
std::vector<uint8_t> FileserverFileLoader::loadFunctionObjectHash(
  const faabric::Message& msg)
{
    std::vector<uint8_t> empty;
    return empty;
}

std::vector<uint8_t> FileserverFileLoader::loadFunctionWamrAotHash(
  const faabric::Message& msg)
{
    std::vector<uint8_t> empty;
    return empty;
}

std::vector<uint8_t> FileserverFileLoader::loadSharedObjectObjectHash(
  const std::string& path)
{
    std::vector<uint8_t> empty;
    return empty;
}

void FileserverFileLoader::uploadFunctionObjectHash(
  const faabric::Message& msg,
  const std::vector<uint8_t>& hash)
{}

void FileserverFileLoader::uploadFunctionWamrAotHash(
  const faabric::Message& msg,
  const std::vector<uint8_t>& hash)
{}

void FileserverFileLoader::uploadSharedObjectObjectHash(
  const std::string& path,
  const std::vector<uint8_t>& hash)
{}

void FileserverFileLoader::uploadSharedObjectAotHash(
  const std::string& path,
  const std::vector<uint8_t>& hash)
{}

}
