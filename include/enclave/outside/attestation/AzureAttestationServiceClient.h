#pragma once

#include <enclave/outside/attestation/EnclaveInfo.h>

#include <jwt-cpp/jwt.h>
#include <string>

namespace sgx {

typedef jwt::decoded_jwt<jwt::traits::kazuho_picojson> DecodedJwt;
typedef jwt::jwks<jwt::traits::kazuho_picojson> JwksSet;

/*
 * This class interfaces with a remote instance of an Azure Attestation Serivce.
 * It provides methods to attest an enclave using the attestation service, and
 * also to manipulate the JWT returned as validation.
 */
class AzureAttestationServiceClient
{
  private:
    std::string attestationServiceUrl;
    std::string certificateEndpoint;
    std::string tenantName;

    // Set of known trusted signing keys (JSON Web Key Sets, JWKS). We fill the
    // cache when instantiating the client and lazily upon signature
    // verification.
    JwksSet cachedJwks;

    // Fetch the JSON Web Key Set (JWKS) from the remote attestation service and
    // populate the local cache.
    JwksSet fetchJwks();

    // Validate that the JKU (JWT Set URL) parameter points to the expected
    // certificate  endpoint.
    void validateJkuUri(const DecodedJwt& decodedJwt);

    // Validate the signature of a JWT against the set of known trusted
    // signatures.
    void validateJwtSignature(const DecodedJwt& decodedJwt);

  public:
    // Generate the request body to remotely attest an enclave from the locally
    // generated quote.
    static std::string requestBodyFromEnclaveInfo(
      const EnclaveInfo& enclaveInfo);

    AzureAttestationServiceClient(const std::string& attestationServiceUrlIn);

    // This method sends the enclave quote to the remote attestation service.
    // If the report passes the attestation checks according to the attestation
    // policy in the remote service, we receive a JWT in response. Otherwise
    // this method throws an exception.
    std::string attestEnclave(const EnclaveInfo& enclaveInfo);

    // Upon succcesful attestation, the attestation service returns a JWT. This
    // method validates the token's integrity and signature.
    void validateJwtToken(const std::string& jwtToken);
};
}
