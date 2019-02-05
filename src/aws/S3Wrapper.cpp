#include "aws.h"
#include "S3Wrapper.h"

#include <util/logging.h>
#include <util/bytes.h>

#include <aws/s3/model/GetObjectRequest.h>
#include <aws/s3/model/ListObjectsRequest.h>
#include <aws/s3/model/PutObjectRequest.h>

namespace awswrapper {
    S3Wrapper::S3Wrapper() : client(Aws::S3::S3Client(getClientConf(REQUEST_TIMEOUT_MS))) {

    }

    S3Wrapper &S3Wrapper::getThreadLocal() {
        static thread_local S3Wrapper s3;
        return s3;
    }


    std::vector<std::string> S3Wrapper::listKeys(const std::string &bucketName) {
        S3Wrapper &s3 = S3Wrapper::getThreadLocal();

        Aws::S3::Model::ListObjectsRequest request;
        request.WithBucket(Aws::String(bucketName));

        auto response = s3.client.ListObjects(request);

        if (!response.IsSuccess()) {
            const auto &err = response.GetError();
            handleError(err);
        }

        Aws::Vector<Aws::S3::Model::Object> keyObjects = response.GetResult().GetContents();
        std::vector<std::string> keys;
        for (auto const &keyObject : keyObjects) {
            const Aws::String &awsStr = keyObject.GetKey();
            keys.emplace_back(awsStr.c_str());
        }

        return keys;
    }

    void S3Wrapper::addKeyBytes(const std::string &bucketName, const std::string &keyName,
            const std::vector<uint8_t> &data) {
        const char *charPtr = reinterpret_cast<const char*>(data.data());
        this->addKey(bucketName, keyName, charPtr, data.size());
    }

    void S3Wrapper::addKeyStr(const std::string &bucketName, const std::string &keyName, const std::string &data) {
        this->addKey(bucketName, keyName, data.c_str(), data.size());
    }

    std::vector<uint8_t> S3Wrapper::getKeyBytes(const std::string &bucketName, const std::string &keyName) {
        const std::string &byteStr = this->getKey(bucketName, keyName);

        const std::vector<uint8_t> &bytes = util::stringToBytes(byteStr);

        return bytes;
    }

    std::string S3Wrapper::getKeyStr(const std::string &bucketName, const std::string &keyName) {
        return this->getKey(bucketName, keyName);
    }

    void S3Wrapper::addKey(const std::string &bucketName, const std::string &keyName, const char* data, size_t dataLen)
    {
        Aws::S3::Model::PutObjectRequest request;
        request.WithBucket(Aws::String(bucketName)).WithKey(Aws::String(keyName));

        std::shared_ptr<Aws::IOStream> dataStream = Aws::MakeShared<Aws::StringStream>(data);
        dataStream->write(data, dataLen);
        request.SetBody(dataStream);

        auto response = client.PutObject(request);

        if (!response.IsSuccess()) {
            const auto &err = response.GetError();
            handleError(err);
        }
    }

    std::string S3Wrapper::getKey(const std::string &bucketName, const std::string &keyName) {
        Aws::S3::Model::GetObjectRequest request;
        request.WithBucket(Aws::String(bucketName)).WithKey(Aws::String(keyName));

        auto response = client.GetObject(request);

        if (!response.IsSuccess()) {
            const auto &err = response.GetError();
            handleError(err);
        }

        std::ostringstream ss;
        auto responseStream = response.GetResult().GetBody().rdbuf();
        ss << responseStream;

        return ss.str();
    }
}
