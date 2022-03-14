#pragma once

#include <string>

namespace sgx {

/*
 * This class contains the necessary attributes and methods to generate a
 * quote from a running enclave. Quotes are necessary for the remote attestation
 * protocol.
 */
class EnclaveQuote
{
  public:
    EnclaveQuote();

    static EnclaveQuote createFromFile(std::string filePath);

  private:
    int enclaveType;
    std::string mrEnclaveHex;
    std::string mrSignerHex;
    std::string productIdHex;
    uint8_t securityVersion;
    uint32_t attributes;
    std::string enclaveQuoteHex;
    std::string enclaveHeldDataHex;;

    // static EnclaveInfo CreateFromFile(string filePath);

    /*
    AttestSgxEnclaveRequestBody* GetMaaBody();
    bool CompareToMaaServiceJwtToken(string serviceJwtToken, bool includeDetails);
    */
};
}
