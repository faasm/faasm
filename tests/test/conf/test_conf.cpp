#include <catch2/catch.hpp>

#include <conf/FaasmConfig.h>

#include <faabric/util/config.h>
#include <faabric/util/environment.h>

using namespace faabric::util;
using namespace conf;

namespace tests {
TEST_CASE("Test default faasm config initialisation", "[conf]")
{
    FaasmConfig conf;
    conf.reset();

    std::string cgroupExpected = "on";
    if (conf.hostType == "ci") {
        cgroupExpected = "off";
    }
    REQUIRE(conf.cgroupMode == cgroupExpected);
    REQUIRE(conf.netNsMode == "off");
    REQUIRE(conf.maxNetNs == 100);

    REQUIRE(conf.pythonPreload == "off");
    REQUIRE(conf.captureStdout == "off");

    REQUIRE(conf.chainedCallTimeout == 300000);

    REQUIRE(conf.wasmVm == "wavm");

    REQUIRE(conf.s3Bucket == "faasm");
    REQUIRE(conf.s3Host == "minio");
    REQUIRE(conf.s3Port == "9000");
    REQUIRE(conf.s3User == "minio");
    REQUIRE(conf.s3Password == "minio123");

    REQUIRE(conf.attestationServiceUrl == "");
}

TEST_CASE("Test overriding faasm config initialisation", "[conf]")
{
    std::string originalHostType = getFaasmConfig().hostType;

    std::string hostType = setEnvVar("HOST_TYPE", "magic");
    std::string cgMode = setEnvVar("CGROUP_MODE", "off");
    std::string nsMode = setEnvVar("NETNS_MODE", "on");
    std::string maxNetNs = setEnvVar("MAX_NET_NAMESPACES", "300");

    std::string pythonPre = setEnvVar("PYTHON_PRELOAD", "on");
    std::string captureStdout = setEnvVar("CAPTURE_STDOUT", "on");
    std::string wasmVm = setEnvVar("FAASM_WASM_VM", "blah");

    std::string chainedTimeout = setEnvVar("CHAINED_CALL_TIMEOUT", "9999");

    std::string faasmLocalDir = setEnvVar("FAASM_LOCAL_DIR", "/tmp/blah");

    std::string s3Bucket = setEnvVar("S3_BUCKET", "dummy-bucket");
    std::string s3Host = setEnvVar("S3_HOST", "dummy-host");
    std::string s3Port = setEnvVar("S3_PORT", "123456");
    std::string s3User = setEnvVar("S3_USER", "dummy-user");
    std::string s3Password = setEnvVar("S3_PASSWORD", "dummy-password");

    std::string attestationServiceUrl =
      setEnvVar("AZ_ATTESTATION_PROVIDER_URL", "");

    // Create new conf for test
    FaasmConfig conf;

    REQUIRE(conf.hostType == "magic");
    REQUIRE(conf.cgroupMode == "off");
    REQUIRE(conf.netNsMode == "on");
    REQUIRE(conf.maxNetNs == 300);

    REQUIRE(conf.pythonPreload == "on");
    REQUIRE(conf.captureStdout == "on");
    REQUIRE(conf.wasmVm == "blah");

    REQUIRE(conf.chainedCallTimeout == 9999);

    REQUIRE(conf.functionDir == "/tmp/blah/wasm");
    REQUIRE(conf.objectFileDir == "/tmp/blah/object");
    REQUIRE(conf.runtimeFilesDir == "/tmp/blah/runtime_root");
    REQUIRE(conf.sharedFilesDir == "/tmp/blah/shared");

    REQUIRE(conf.s3Bucket == "dummy-bucket");
    REQUIRE(conf.s3Host == "dummy-host");
    REQUIRE(conf.s3Port == "123456");
    REQUIRE(conf.s3User == "dummy-user");
    REQUIRE(conf.s3Password == "dummy-password");

    REQUIRE(conf.attestationServiceUrl == "");

    // Be careful with host type as it must remain consistent for tests
    setEnvVar("HOST_TYPE", originalHostType);

    setEnvVar("CGROUP_MODE", cgMode);
    setEnvVar("NETNS_MODE", nsMode);
    setEnvVar("MAX_NET_NAMESPACES", maxNetNs);

    setEnvVar("PYTHON_PRELOAD", pythonPre);
    setEnvVar("CAPTURE_STDOUT", captureStdout);
    setEnvVar("FAASM_WASM_VM", wasmVm);

    setEnvVar("CHAINED_CALL_TIMEOUT", chainedTimeout);

    setEnvVar("FAASM_LOCAL_DIR", faasmLocalDir);

    setEnvVar("S3_BUCKET", s3Bucket);
    setEnvVar("S3_HOST", s3Host);
    setEnvVar("S3_PORT", s3Port);
    setEnvVar("S3_USER", s3User);
    setEnvVar("S3_PASSWORD", s3Password);

    setEnvVar("ATTESTATION_SERVICE_URL", attestationServiceUrl);
}
}
