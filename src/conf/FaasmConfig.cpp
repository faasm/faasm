#include <conf/FaasmConfig.h>
#include <faabric/util/environment.h>

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
    // Threads
    moduleThreadPoolSize = this->getIntParam("FAASM_THREAD_POOL_SIZE", "0");
    if (moduleThreadPoolSize == 0) {
        moduleThreadPoolSize = faabric::util::getUsableCores();
    }
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
}
