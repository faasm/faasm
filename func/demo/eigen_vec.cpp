#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/src/Core/arch/WASM/Complex.h>
#include <eigen3/Eigen/src/Core/arch/WASM/PacketMath.h>

#include "faasm/faasm.h"
#include <iostream>
#include <stdio.h>

using namespace Eigen::internal;

bool approxCompare(float a, float b)
{
    return truncf(1000.0f * a) == truncf(1000.0f * b);
}

#define compareLane(a, b, idx)                                                 \
    approxCompare(wasm_f32x4_extract_lane(a, idx),                             \
                  wasm_f32x4_extract_lane(b, idx));

void printPacket(const Packet4f& a)
{
    printf("Packet4f[%.3f, %.3f, %.3f, %.3f]",
           wasm_f32x4_extract_lane(a, 0),
           wasm_f32x4_extract_lane(a, 1),
           wasm_f32x4_extract_lane(a, 2),
           wasm_f32x4_extract_lane(a, 3));
}

void printComplex(const Packet2cf& a)
{
    printf("Packet2cf[%.3f + %.3fi, %.3f + %.3fi]",
           wasm_f32x4_extract_lane(a.v, 0),
           wasm_f32x4_extract_lane(a.v, 1),
           wasm_f32x4_extract_lane(a.v, 2),
           wasm_f32x4_extract_lane(a.v, 3));
}

bool checkPacket4fEquals(const Packet4f& a, const Packet4f& b)
{
    bool isEqual = compareLane(a, b, 0) isEqual &= compareLane(a, b, 1)
      isEqual &= compareLane(a, b, 2) isEqual &= compareLane(a, b, 3)

        if (!isEqual)
    {
        printf("Packets not equal: ");
        printPacket(a);
        printf(" != ");
        printPacket(b);
        printf("\n");
    }

    return isEqual;
}

bool checkPacket2cfEquals(const Packet2cf& a, const Packet2cf& b)
{
    bool isEqual = compareLane(a.v, b.v, 0) isEqual &= compareLane(a.v, b.v, 1)
      isEqual &= compareLane(a.v, b.v, 2) isEqual &= compareLane(a.v, b.v, 3)

        if (!isEqual)
    {
        printf("Complex numbers not equal: ");
        printComplex(a);
        printf("!= ");
        printComplex(b);
        printf("\n");
    }

    return isEqual;
}

bool checkMultiplication()
{
    Packet4f a = wasm_f32x4_make(2.5f, 3.4f, 4.4f, 5.5f);
    Packet4f b = wasm_f32x4_make(1.0f, 2.0f, 3.0f, 4.0f);

    Packet4f expected = wasm_f32x4_make(2.5f, 6.8f, 13.2f, 22.0f);

    Packet4f c = pmul<Packet4f>(a, b);

    bool isEqual = checkPacket4fEquals(expected, c);

    if (isEqual) {
        printf("Vectorized multiplication OK\n");
    } else {
        printf("Vectorized multiplication NOT OK\n");
    }

    return isEqual;
}

bool checkAddition()
{
    Packet4f a = wasm_f32x4_make(10.0f, 27.0f, 15.0f, 0.0f);
    Packet4f b = wasm_f32x4_make(2.0f, -9.0f, 7.5f, 4.0f);

    Packet4f expected = wasm_f32x4_make(12.0f, 18.0f, 22.5f, 4.0f);

    Packet4f c = padd<Packet4f>(a, b);

    bool isEqual = checkPacket4fEquals(expected, c);

    if (isEqual) {
        printf("Vectorized addition OK\n");
    } else {
        printf("Vectorized addition NOT OK\n");
    }

    return isEqual;
}

bool checkXOR()
{
    const v128_t mask = wasm_f32x4_const(-0.0f, 0.0f, -0.0f, 0.0f);
    Packet4f a = wasm_f32x4_make(5.5f, 6.6f, 7.7f, 8.8f);
    Packet4f expected = wasm_f32x4_make(-5.5f, 6.6f, -7.7f, 8.8f);

    Packet4f actual = wasm_v128_xor(a, mask);

    bool isEqual = checkPacket4fEquals(actual, expected);

    if (isEqual) {
        printf("Vectorized XOR OK\n");
    } else {
        printf("Vectorized XOR NOT OK\n");
    }

    return isEqual;
}

bool checkDivision()
{
    Packet4f a = wasm_f32x4_make(10.0f, 27.0f, 15.0f, 0.0f);
    Packet4f b = wasm_f32x4_make(2.0f, 9.0f, 7.5f, 4.0f);

    Packet4f expected = wasm_f32x4_make(5.0f, 3.0f, 2.0f, 0.0f);

    Packet4f c = pdiv<Packet4f>(a, b);

    bool isEqual = checkPacket4fEquals(expected, c);

    if (isEqual) {
        printf("Vectorized division OK\n");
    } else {
        printf("Vectorized division NOT OK\n");
    }

    return isEqual;
}

bool checkTranspose()
{
    PacketBlock<Packet4f, 4> pb{};
    pb.packet[0] = wasm_f32x4_make(1, 2, 3, 4);
    pb.packet[1] = wasm_f32x4_make(5, 6, 7, 8);
    pb.packet[2] = wasm_f32x4_make(9, 10, 11, 12);
    pb.packet[3] = wasm_f32x4_make(13, 14, 15, 16);

    ptranspose(pb);

    bool isEqual =
      checkPacket4fEquals(pb.packet[0], wasm_f32x4_make(1, 5, 9, 13));
    isEqual &= checkPacket4fEquals(pb.packet[1], wasm_f32x4_make(2, 6, 10, 14));
    isEqual &= checkPacket4fEquals(pb.packet[2], wasm_f32x4_make(3, 7, 11, 15));
    isEqual &= checkPacket4fEquals(pb.packet[3], wasm_f32x4_make(4, 8, 12, 16));

    if (isEqual) {
        printf("Vectorized transpose OK\n");
    } else {
        printf("Vectorized transpose NOT OK\n");
    }

    return isEqual;
}

bool checkComplexConj()
{
    Packet2cf a(wasm_f32x4_make(4, 8, 10, 2));
    Packet2cf actual = pconj(a);

    Packet2cf expected(wasm_f32x4_make(4, -8, 10, -2));

    bool isEqual = checkPacket2cfEquals(actual, expected);

    if (isEqual) {
        printf("Vectorized complex conjugate OK\n");
    } else {
        printf("Vectorized complex conjugate NOT OK\n");
    }

    return isEqual;
}

bool checkComplexMult()
{
    Packet2cf a(wasm_f32x4_make(1, 2, 3, 4));
    Packet2cf b(wasm_f32x4_make(5, 6, 8, 7));

    Packet2cf expected(wasm_f32x4_make(-7, 16, -4, 53));

    Packet2cf actual = pmul(a, b);
    bool isEqual = checkPacket2cfEquals(actual, expected);

    if (isEqual) {
        printf("Vectorized complex multiplication OK\n");
    } else {
        printf("Vectorized complex multiplication NOT OK\n");
    }

    return isEqual;
}

int main(int argc, char* argv[])()
{
    std::string instrSets = Eigen::SimdInstructionSetsInUse();

#ifdef __wasm__
    if (instrSets != "WASM") {
        printf("Expected WASM instruction sets but got %s\n",
               instrSets.c_str());
        return 1;
    }
#endif

    bool isEqual = checkMultiplication();
    isEqual &= checkAddition();
    isEqual &= checkXOR();
    isEqual &= checkDivision();
    isEqual &= checkTranspose();
    isEqual &= checkComplexConj();
    isEqual &= checkComplexMult();

    if (!isEqual) {
        return 1;
    }

    return 0;
}
