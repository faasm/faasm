#include <conf/FaasmConfig.h>
#include <faabric/util/bytes.h>
#include <faabric/util/logging.h>
#include <storage/S3Wrapper.h>

#include <boost/algorithm/string/trim.hpp>

namespace storage {

enum class S3Error
{
    BucketAlreadyOwnedByYou,
    BucketNotEmpty,
    NoSuchBucket,
    NoSuchKey,
    // Used as a catch-all, ideally remove
    UnrecognisedError,
};

std::unordered_map<std::string, S3Error> errorToStringMap = {
    { "BucketAlreadyOwnedByYou", S3Error::BucketAlreadyOwnedByYou },
    { "BucketNotEmpty", S3Error::BucketNotEmpty },
    { "NoSuchBucket", S3Error::NoSuchBucket },
    { "NoSuchKey", S3Error::NoSuchKey },
};

S3Error parseError(const std::string& errorStr)
{
    if (errorToStringMap.find(errorStr) == errorToStringMap.end()) {
        SPDLOG_WARN("Unregognised error: {}", errorStr);
        return S3Error::UnrecognisedError;
    }

    return errorToStringMap.at(errorStr);
}

#define CHECK_ERRORS(response, bucketName, keyName)                            \
    {                                                                          \
        if (!response) {                                                       \
            if (std::string(bucketName).empty()) {                             \
                SPDLOG_ERROR("General S3 error: {} ({})",                      \
                             response.code,                                    \
                             response.message);                                \
            } else if (std::string(keyName).empty()) {                         \
                SPDLOG_ERROR("S3 error with bucket {}: {} ({})",               \
                             bucketName,                                       \
                             response.code,                                    \
                             response.message);                                \
            } else {                                                           \
                SPDLOG_ERROR("S3 error with bucket/key {}/{}: {} ({})",        \
                             bucketName,                                       \
                             keyName,                                          \
                             response.code,                                    \
                             response.message);                                \
            }                                                                  \
            throw std::runtime_error("S3 error");                              \
        }                                                                      \
    };

// TODO: consider deleting this method
void initFaasmS3()
{
    const auto& conf = conf::getFaasmConfig();
    SPDLOG_INFO(
      "Initialising Faasm S3 setup at {}:{}", conf.s3Host, conf.s3Port);

    S3Wrapper s3wrapper;
    s3wrapper.createBucket(conf.s3Bucket);

    // Check we can write/ read
    s3wrapper.addKeyStr(conf.s3Bucket, "ping", "pong");
    std::string response = s3wrapper.getKeyStr(conf.s3Bucket, "ping");
    if (response != "pong") {
        std::string errorMsg =
          fmt::format("Unable to write/ read to/ from S3 ({})", response);
        SPDLOG_ERROR(errorMsg);
        throw std::runtime_error(errorMsg);
    }

    SPDLOG_INFO("Successfully pinged S3 at {}:{}", conf.s3Host, conf.s3Port);
}

void shutdownFaasmS3()
{
    ;
}

S3Wrapper::S3Wrapper()
  : faasmConf(conf::getFaasmConfig())
  , baseUrl(minio::s3::BaseUrl(
      fmt::format("{}:{}", faasmConf.s3Host, faasmConf.s3Port),
      false,
      {}))
  // TODO: consider a better for of authentication
  , provider(minio::creds::StaticProvider("minio", "minio123"))
  , client(baseUrl, &provider)
{}

void S3Wrapper::createBucket(const std::string& bucketName)
{
    SPDLOG_DEBUG("Creating bucket {}", bucketName);
    minio::s3::MakeBucketArgs args;
    args.bucket = bucketName;
    auto response = client.MakeBucket(args);

    if (!response) {
        auto error = parseError(response.code);
        if (error == S3Error::BucketAlreadyOwnedByYou) {
            SPDLOG_DEBUG("Bucket already exists {}", bucketName);
            return;
        }

        CHECK_ERRORS(response, bucketName, "");
    }
}

void S3Wrapper::deleteBucket(const std::string& bucketName, bool recursive)
{
    SPDLOG_DEBUG("Deleting bucket {}", bucketName);
    minio::s3::RemoveBucketArgs args;
    args.bucket = bucketName;
    auto response = client.RemoveBucket(args);

    if (!response) {
        auto error = parseError(response.code);
        if (error == S3Error::NoSuchBucket) {
            SPDLOG_DEBUG("Bucket already deleted {}", bucketName);
            return;
        }

        if (error == S3Error::BucketNotEmpty) {
            if (recursive) {
                SPDLOG_ERROR("Caught an erroneous recursive loop");
                throw std::runtime_error("Erroneous recurvie loop!");
            }

            SPDLOG_DEBUG("Bucket {} not empty, deleting keys", bucketName);

            std::vector<std::string> keys = listKeys(bucketName);
            for (const auto& key : keys) {
                deleteKey(bucketName, key);
            }

            // Recursively delete
            deleteBucket(bucketName, true);
            return;
        }

        CHECK_ERRORS(response, bucketName, "");
    }
}

std::vector<std::string> S3Wrapper::listBuckets()
{
    SPDLOG_TRACE("Listing buckets");

    auto response = client.ListBuckets();
    CHECK_ERRORS(response, "", "");

    std::vector<std::string> bucketNames;
    for (auto const& bucketObject : response.buckets) {
        bucketNames.emplace_back(bucketObject.name);
    }

    return bucketNames;
}

std::vector<std::string> S3Wrapper::listKeys(const std::string& bucketName,
                                             const std::string& prefix)
{
    SPDLOG_TRACE("Listing keys in bucket {} (prefix: {})", bucketName, prefix);

    minio::s3::ListObjectsArgs args;
    args.bucket = bucketName;
    if (!prefix.empty()) {
        args.prefix = prefix;
    }
    args.recursive = true;
    auto response = client.ListObjects(args);

    std::vector<std::string> keys;
    for (; response; response++) {
        minio::s3::Item item = *response;
        if (!item.name.empty()) {
            keys.push_back(item.name);
        }
    }

    return keys;
}

void S3Wrapper::deleteKey(const std::string& bucketName,
                          const std::string& keyName)
{
    SPDLOG_TRACE("Deleting S3 key {}/{}", bucketName, keyName);
    minio::s3::RemoveObjectArgs args;
    args.bucket = bucketName;
    args.object = keyName;

    auto response = client.RemoveObject(args);

    if (!response) {
        auto error = parseError(response.code);

        if (error == S3Error::NoSuchKey) {
            SPDLOG_DEBUG("Key already deleted {}", keyName);
            return;
        }

        if (error == S3Error::NoSuchBucket) {
            SPDLOG_DEBUG("Bucket already deleted {}", bucketName);
            return;
        }

        CHECK_ERRORS(response, bucketName, keyName);
    }
}

class ByteStreamBuf : public std::streambuf
{
  public:
    ByteStreamBuf(const std::vector<uint8_t>& data)
    {
        // Set the beginning and end of the buffer
        char* begin =
          reinterpret_cast<char*>(const_cast<uint8_t*>(data.data()));
        this->setg(begin, begin, begin + data.size());
    }
};

void S3Wrapper::addKeyBytes(const std::string& bucketName,
                            const std::string& keyName,
                            const std::vector<uint8_t>& data)
{
    SPDLOG_TRACE("Writing S3 key {}/{} as bytes", bucketName, keyName);

    ByteStreamBuf buffer(data);
    std::istream iss(&buffer);

    minio::s3::PutObjectArgs args(iss, data.size(), 0);
    args.bucket = bucketName;
    args.object = keyName;

    auto response = client.PutObject(args);

    CHECK_ERRORS(response, bucketName, keyName);
}

void S3Wrapper::addKeyStr(const std::string& bucketName,
                          const std::string& keyName,
                          const std::string& data)
{
    std::istringstream iss(data);

    minio::s3::PutObjectArgs args(iss, data.size(), 0);
    args.bucket = bucketName;
    args.object = keyName;

    SPDLOG_TRACE("Writing S3 key {}/{} as string", bucketName, keyName);
    auto response = client.PutObject(args);

    CHECK_ERRORS(response, bucketName, keyName);
}

std::vector<uint8_t> S3Wrapper::getKeyBytes(const std::string& bucketName,
                                            const std::string& keyName,
                                            bool tolerateMissing)
{
    SPDLOG_TRACE("Getting S3 key {}/{} as bytes", bucketName, keyName);

    std::vector<uint8_t> data;

    minio::s3::GetObjectArgs args;
    args.bucket = bucketName;
    args.object = keyName;

    args.datafunc = [&data](minio::http::DataFunctionArgs args) -> bool {
        data.insert(data.end(), args.datachunk.begin(), args.datachunk.end());
        return true;
    };

    auto response = client.GetObject(args);
    if (!response) {
        auto error = parseError(response.code);
        if (tolerateMissing && (error == S3Error::NoSuchKey)) {
            SPDLOG_TRACE(
              "Tolerating missing S3 key {}/{}", bucketName, keyName);
            return std::vector<uint8_t>();
        }

        CHECK_ERRORS(response, bucketName, keyName);
    }

    return data;
}

std::string S3Wrapper::getKeyStr(const std::string& bucketName,
                                 const std::string& keyName,
                                 bool tolerateMissing)
{
    SPDLOG_TRACE("Getting S3 key {}/{} as string", bucketName, keyName);

    std::string data;

    minio::s3::GetObjectArgs args;
    args.bucket = bucketName;
    args.object = keyName;

    args.datafunc = [&data](minio::http::DataFunctionArgs args) -> bool {
        data.append(args.datachunk);
        return true;
    };

    auto response = client.GetObject(args);
    if (!response) {
        auto error = parseError(response.code);
        if (tolerateMissing && (error == S3Error::NoSuchKey)) {
            SPDLOG_TRACE(
              "Tolerating missing S3 key {}/{}", bucketName, keyName);
            return "";
        }

        CHECK_ERRORS(response, bucketName, keyName);
    }

    return data;
}
}
