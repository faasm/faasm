#pragma once

#include <string>

namespace conf {
class FaasmConfig
{
  public:
    // Threading
    int moduleThreadPoolSize;

    FaasmConfig();

    void reset();

  private:
    int getIntParam(const char* name, const char* defaultValue);

    void initialise();
};

FaasmConfig& getFaasmConfig();
}
