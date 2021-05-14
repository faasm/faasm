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

    REQUIRE(conf.cgroupMode == "on");
    REQUIRE(conf.functionStorage == "local");
    REQUIRE(conf.fileserverUrl == "");
    REQUIRE(conf.netNsMode == "off");
    REQUIRE(conf.pythonPreload == "off");
    REQUIRE(conf.captureStdout == "off");
    REQUIRE(conf.wasmVm == "wavm");

    REQUIRE(conf.chainedCallTimeout == 300000);
}

TEST_CASE("Test overriding faasm config initialisation", "[conf]")
{
    std::string originalHostType = getFaasmConfig().hostType;

    std::string hostType = setEnvVar("HOST_TYPE", "magic");
    std::string funcStorage = setEnvVar("FUNCTION_STORAGE", "foobar");
    std::string fileserver = setEnvVar("FILESERVER_URL", "www.foo.com");
    std::string cgMode = setEnvVar("CGROUP_MODE", "off");
    std::string nsMode = setEnvVar("NETNS_MODE", "on");
    std::string pythonPre = setEnvVar("PYTHON_PRELOAD", "on");
    std::string captureStdout = setEnvVar("CAPTURE_STDOUT", "on");
    std::string wasmVm = setEnvVar("WASM_VM", "blah");

    std::string chainedTimeout = setEnvVar("CHAINED_CALL_TIMEOUT", "9999");

    std::string faasmLocalDir = setEnvVar("FAASM_LOCAL_DIR", "/tmp/blah");

    // Create new conf for test
    FaasmConfig conf;

    REQUIRE(conf.hostType == "magic");
    REQUIRE(conf.functionStorage == "foobar");
    REQUIRE(conf.fileserverUrl == "www.foo.com");
    REQUIRE(conf.cgroupMode == "off");
    REQUIRE(conf.netNsMode == "on");
    REQUIRE(conf.pythonPreload == "on");
    REQUIRE(conf.captureStdout == "on");
    REQUIRE(conf.wasmVm == "blah");
    REQUIRE(conf.chainedCallTimeout == 9999);
    REQUIRE(conf.functionDir == "/tmp/blah/wasm");
    REQUIRE(conf.objectFileDir == "/tmp/blah/object");
    REQUIRE(conf.runtimeFilesDir == "/tmp/blah/runtime_root");
    REQUIRE(conf.sharedFilesDir == "/tmp/blah/shared");
    REQUIRE(conf.sharedFilesStorageDir == "/tmp/blah/shared_store");

    // Be careful with host type as it must remain consistent for tests
    setEnvVar("HOST_TYPE", originalHostType);

    setEnvVar("FUNCTION_STORAGE", funcStorage);
    setEnvVar("FILESERVER_URL", fileserver);
    setEnvVar("CGROUP_MODE", cgMode);
    setEnvVar("NETNS_MODE", nsMode);
    setEnvVar("PYTHON_PRELOAD", pythonPre);
    setEnvVar("CAPTURE_STDOUT", captureStdout);
    setEnvVar("WASM_VM", wasmVm);

    setEnvVar("CHAINED_CALL_TIMEOUT", chainedTimeout);

    setEnvVar("FAASM_LOCAL_DIR", faasmLocalDir);
}
}
