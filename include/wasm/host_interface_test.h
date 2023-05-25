#pragma once

namespace wasm {
/* We use one host interface call, `__faasm_host_interface_test` to test
 * different behaviours of host interface calls like, for example, throwing
 * an exception and testing that it propagates correctly through the WASM
 * runtime all the way to Faasm. With this enum we indicate the test number.
 * Note that, most likely, this header file needs to be duplicated in Faasm (?)
 */
enum HostInterfaceTest {
    NoTest = 0,
    ExceptionPropagationTest = 1,
};

void doHostInterfaceTest(int testNum);
}
