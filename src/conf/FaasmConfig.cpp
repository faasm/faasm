#include <conf/FaasmConfig.h>
#include <faabric/util/environment.h>
#include <faabric/util/logging.h>

using namespace faabric::util;

namespace conf {
FaasmConfig& getFaasmConfig()
{
    static FaasmConfig conf;
    return conf;
}

FaasmConfig::FaasmConfig()
{
    this->initialise();
}

void FaasmConfig::initialise()
{
    hostType = getEnvVar("HOST_TYPE", "default");

    cgroupMode = getEnvVar("CGROUP_MODE", "on");
    netNsMode = getEnvVar("NETNS_MODE", "off");
    maxNetNs = this->getIntParam("MAX_NET_NAMESPACES", "100");

    pythonPreload = getEnvVar("PYTHON_PRELOAD", "off");
    captureStdout = getEnvVar("CAPTURE_STDOUT", "off");

    wasmVm = getEnvVar("FAASM_WASM_VM", "wavm");
    chainedCallTimeout = this->getIntParam("CHAINED_CALL_TIMEOUT", "300000");

    std::string faasmLocalDir =
      getEnvVar("FAASM_LOCAL_DIR", "/usr/local/faasm");
    functionDir = fmt::format("{}/{}", faasmLocalDir, "wasm");
    objectFileDir = fmt::format("{}/{}", faasmLocalDir, "object");
    runtimeFilesDir = fmt::format("{}/{}", faasmLocalDir, "runtime_root");
    sharedFilesDir = fmt::format("{}/{}", faasmLocalDir, "shared");

    s3Bucket = getEnvVar("S3_BUCKET", "faasm");
    s3Host = getEnvVar("S3_HOST", "minio");
    s3Port = getEnvVar("S3_PORT", "9000");
    s3User = getEnvVar("S3_USER", "minio");
    s3Password = getEnvVar("S3_PASSWORD", "minio123");

    attestationServiceUrl = getEnvVar("ATTESTATION_SERVICE_URL", "");
    acclessEnabled = getEnvVar("ACCLESS_ENABLED", "off");
}

int FaasmConfig::getIntParam(const char* name, const char* defaultValue)
{
    int value = stoi(faabric::util::getEnvVar(name, defaultValue));

    return value;
};

void FaasmConfig::reset()
{
    this->initialise();
}

void FaasmConfig::print()
{
    SPDLOG_INFO("--- HOST ---");
    SPDLOG_INFO("Cgroup mode:          {}", cgroupMode);
    SPDLOG_INFO("Host type:            {}", hostType);
    SPDLOG_INFO("Network ns mode:      {}", netNsMode);
    SPDLOG_INFO("Max. network ns:      {}", maxNetNs);

    SPDLOG_INFO("--- MISC ---");
    SPDLOG_INFO("Capture stdout:       {}", captureStdout);
    SPDLOG_INFO("Chained call timeout: {}", chainedCallTimeout);
    SPDLOG_INFO("Python preload:       {}", pythonPreload);
    SPDLOG_INFO("Wasm VM:              {}", wasmVm);
    SPDLOG_INFO("Att. service URL:     {}", attestationServiceUrl);
    SPDLOG_INFO("Accless mode:         {}", acclessEnabled);

    SPDLOG_INFO("--- STORAGE ---");
    SPDLOG_INFO("Function dir:         {}", functionDir);
    SPDLOG_INFO("Object file dir:      {}", objectFileDir);
    SPDLOG_INFO("Runtime files dir:    {}", runtimeFilesDir);
    SPDLOG_INFO("Shared files dir:     {}", sharedFilesDir);
}
}
