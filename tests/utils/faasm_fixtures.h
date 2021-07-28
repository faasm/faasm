#pragma once

#include <conf/FaasmConfig.h>
#include <storage/S3Wrapper.h>

namespace tests {

class FaasmConfTestFixture
{
  public:
    FaasmConfTestFixture()
      : conf(conf::getFaasmConfig())
    {}
    ~FaasmConfTestFixture() { conf.reset(); }

  protected:
    conf::FaasmConfig& conf;
};

class S3TestFixture
{
  public:
    S3TestFixture()
      : conf(conf::getFaasmConfig())
    {
        conf.s3Bucket = "faasm-test";
        s3.createBucket(conf.s3Bucket);
    };

    ~S3TestFixture()
    {
        s3.deleteBucket(conf.s3Bucket);
        conf.reset();
    };

  protected:
    storage::S3Wrapper s3;
    conf::FaasmConfig& conf;
};

}
