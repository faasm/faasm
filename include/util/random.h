#pragma once

#include <set>

namespace util {
    int randomInteger(int iStart = 0, int iEnd = 1000000);

    template<typename T>
    T randomSetElement(std::set<T> in) {
        typename std::set<T>::iterator iter = in.begin();

        int rint = randomInteger(0, in.size());
        std::advance(iter, rint);

        return *iter;
    }
}