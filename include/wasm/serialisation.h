#ifndef FAASM_SERIALISATION_H
#define FAASM_SERIALISATION_H

#include <cereal/types/vector.hpp>

namespace wasm {
class MemorySerialised
{
  public:
    size_t numPages;
    std::vector<uint8_t> data;

    template<class Archive>
    void serialize(Archive& ar)
    {
        ar(numPages, data);
    }
};
}

#endif
