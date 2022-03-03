#pragma once

// Return true if the CPU is SGX-capable _and_ SGX-enabled. The relevant header
// file in the SGX SDK is: sgxsdk/include/sgx_capable.h
bool isSgxEnabled();
