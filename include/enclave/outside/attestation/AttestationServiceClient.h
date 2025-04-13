#pragma once

#include <enclave/outside/attestation/EnclaveInfo.h>

#include <jwt-cpp/traits/kazuho-picojson/defaults.h>
#include <string>

namespace sgx {

typedef jwt::decoded_jwt<jwt::traits::kazuho_picojson> DecodedJwt;
typedef jwt::jwks<jwt::traits::kazuho_picojson> JwksSet;

/*
 * This class interfaces with a remote instance of an Azure Attestation Serivce.
 * It provides methods to attest an enclave using the attestation service, and
 * also to manipulate the JWT returned as validation.
 */
class AttestationServiceClient
{
  public:
    // Generate the request body to remotely attest an enclave from the locally
    // generated quote.
    static std::string requestBodyFromEnclaveInfo(
      const EnclaveInfo& enclaveInfo);

    AttestationServiceClient(const std::string& attestationServiceUrlIn);

    // This method sends the enclave quote to the remote attestation service.
    // If the report passes the attestation checks according to the attestation
    // policy in the remote service, we receive a JWT in response. Otherwise
    // this method throws an exception.
    std::string attestEnclave(const EnclaveInfo& enclaveInfo);
    std::string attestEnclave(const std::vector<uint8_t>& quote,
                              sgx_report_t& report);

    std::pair<std::string, std::string> getTokenFromJwtResponse(
      const std::string& jwtResponse);
    DecodedJwt getDecodedJwtFromJwtResponse(const std::string& jwtResponse);

  private:
    std::string attestationServiceUrl;

    // Endpoint in the attestation service where to POST our attestation report
    // for verification purposes
    std::string verifyReportEndpoint;
};
}
