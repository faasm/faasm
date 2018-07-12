
#ifndef FAASM_ENV_H
#define FAASM_ENV_H

#include <string>

std::string getEnvVar(std::string const& key, std::string const& dflt) {
    char const* val = getenv(key.c_str());
    return val == nullptr ? dflt : std::string(val);
}

#endif //FAASM_ENV_H
