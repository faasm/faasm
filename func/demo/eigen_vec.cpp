#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/src/Core/arch/WASM/PacketMath.h>

#include "faasm/faasm.h"
#include <stdio.h>
#include <iostream>

using namespace Eigen::internal;

void printPacket(const Packet4f &a) {
    printf("Packet4f[%.2f, %.2f, %.2f, %.2f]\n",
           wasm_f32x4_extract_lane(a, 0),
           wasm_f32x4_extract_lane(a, 1),
           wasm_f32x4_extract_lane(a, 2),
           wasm_f32x4_extract_lane(a, 3)
    );
}

bool compareLane(const Packet4f &a, const Packet4f &b, int idx) {
    return wasm_f32x4_extract_lane(a, 0) == wasm_f32x4_extract_lane(b, 0);
}

bool checkPacket4fEquals(const Packet4f &a, const Packet4f &b) {
    bool isEqual = compareLane(a, b, 0);
    isEqual &= compareLane(a, b, 1);
    isEqual &= compareLane(a, b, 2);
    isEqual &= compareLane(a, b, 3);

    if(!isEqual) {
        printf("Packets not equal: \n");
        printPacket(a);
        printf("!=\n");
        printPacket(b);
    }

    return isEqual;
}

bool checkMultiplication() {
    Packet4f a = wasm_f32x4_make(2.5f, 3.4f, 4.4f, 5.5f);
    Packet4f b = wasm_f32x4_make(1.0f, 2.0f, 3.0f, 4.0f);

    Packet4f expected = wasm_f32x4_make(2.5f, 6.8f, 13.2f, 22.0f);

    Packet4f c = pmul<Packet4f>(a, b);

    bool isEqual = checkPacket4fEquals(expected, c);

    return isEqual;
}

bool checkDivision() {
    Packet4f a = wasm_f32x4_make(10.0f, 27.0f, 15.0f, 0.0f);
    Packet4f b = wasm_f32x4_make(2.0f, 9.0f, 7.5f, 4.0f);

    Packet4f expected = wasm_f32x4_make(5.0f, 3.0f, 2.0f, 0.0f);

    Packet4f c = pdiv<Packet4f>(a, b);

    bool isEqual = checkPacket4fEquals(expected, c);

    return isEqual;
}

FAASM_MAIN_FUNC() {
    std::string instrSets = Eigen::SimdInstructionSetsInUse();

#ifdef __wasm__
    if(instrSets != "WASM") {
        printf("Expected WASM instruction sets but got %s\n", instrSets.c_str());
        return 1;
    }
#endif

    bool isEqual = checkMultiplication();
    isEqual &= checkDivision();

    if(!isEqual) {
        return 1;
    }

    return 0;
}
