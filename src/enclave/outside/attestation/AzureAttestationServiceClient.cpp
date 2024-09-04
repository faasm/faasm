#include <enclave/outside/attestation/AzureAttestationServiceClient.h>
#include <enclave/outside/attestation/EnclaveInfo.h>
#include <faabric/util/asio.h>
#include <faabric/util/logging.h>

#include <cppcodec/base64_url.hpp>
#include <jwt-cpp/traits/kazuho-picojson/defaults.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#define ATT_URI_SUFFIX ":443/attest/SgxEnclave?api-version=2020-10-01"

using namespace rapidjson;
using header = beast::http::field;
using BeastHttpRequest = faabric::util::BeastHttpRequest;
using BeastHttpResponse = faabric::util::BeastHttpResponse;

namespace sgx {

std::string AzureAttestationServiceClient::requestBodyFromEnclaveInfo(
  const EnclaveInfo& enclaveInfo)
{
    Document d;
    d.SetObject();
    Value outer, inner;

    Document::AllocatorType& allocator = d.GetAllocator();

    // Specification for the JSON Format to attest SGX enclaves
    // https://docs.microsoft.com/en-us/rest/api/attestation/attestation/attest-sgx-enclave
    outer.SetObject();

    // draftPolicyForAttestation: attest against a provided draft policy rather
    // than one uploaded to the attestation service (unset)
    std::string draftPolicyForAttestation = "";
    outer.AddMember("draftPolicyForAttestation",
                    Value(draftPolicyForAttestation.c_str(),
                          draftPolicyForAttestation.size()),
                    allocator);

    // initTimeData: initialisation data provided when enclave is created
    // (unset)
    std::string initTimeData = "";
    inner.SetObject();
    inner.AddMember(
      "data", Value(initTimeData.c_str(), initTimeData.size()), allocator);
    inner.AddMember(
      "dataType", Value(initTimeData.c_str(), initTimeData.size()), allocator);
    outer.AddMember("initTimeData", inner, allocator);

    // quote: quote of the enclave to be attested
    std::vector<uint8_t> quote = enclaveInfo.getQuote();
    std::string quoteBase64 =
      cppcodec::base64_url::encode(&quote[0], quote.size());
    outer.AddMember(
      "quote", Value(quoteBase64.c_str(), quoteBase64.size()), allocator);

    // runtimeData: data provided by the enclave at quote generation time. This
    // field corresponds to the enclave held data variable that we can configure
    // before attestation.
    // 06/04/2022 - For the moment we don't include the enclave held data in
    // the request, as there is still not a clear use for it.
    std::vector<uint8_t> heldData = {};
    std::string enclaveHeldDataBase64 =
      cppcodec::base64_url::encode(&heldData[0], heldData.size());
    std::string dataType = "Binary";
    inner.SetObject();
    inner.AddMember(
      "data",
      Value(enclaveHeldDataBase64.c_str(), enclaveHeldDataBase64.size()),
      allocator);
    inner.AddMember(
      "dataType", Value(dataType.c_str(), dataType.size()), allocator);
    outer.AddMember("runtimeData", inner, allocator);

    d.CopyFrom(outer, allocator);

    StringBuffer buffer;
    Writer<rapidjson::StringBuffer> writer(buffer);
    d.Accept(writer);
    return std::string(buffer.GetString());
}

AzureAttestationServiceClient::AzureAttestationServiceClient(
  const std::string& attestationServiceUrlIn)
  : attestationServiceUrl(attestationServiceUrlIn)
  , certificateEndpoint(attestationServiceUrlIn + "/certs")
  , tenantName(attestationServiceUrlIn.substr(std::string("https://").length()))
  , cachedJwks(fetchJwks())
{}

static BeastHttpResponse doRequest(const std::string& url,
                                   const BeastHttpRequest& request)
{
    // Send HTTP request and wait for task to complete
    boost::asio::io_context ioc;
    boost::asio::ip::tcp::resolver resolver(ioc);
    beast::tcp_stream stream(ioc);
    auto results = resolver.resolve(url);
    stream.connect(results);
    beast::http::write(stream, request);

    // Process response
    beast::flat_buffer buffer;
    BeastHttpResponse response;
    beast::http::read(stream, buffer, response);

    // Close connection
    beast::error_code errorCode;
    stream.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both,
                             errorCode);

    if (errorCode && errorCode != beast::errc::not_connected) {
        SPDLOG_ERROR("Caught exception while querying Azure Attestation Service"
                     "to validate SGX quote");
        throw beast::system_error(errorCode);
    }

    return response;
}

std::string AzureAttestationServiceClient::attestEnclave(
  const EnclaveInfo& enclaveInfo)
{
    // Prepare HTTP request
    std::string uri = attestationServiceUrl + ATT_URI_SUFFIX;
    BeastHttpRequest request(beast::http::verb::post, uri, 11);
    request.set(header::content_type, "application/json");
    std::string requestBodyJson = requestBodyFromEnclaveInfo(enclaveInfo);
    request.content_length(requestBodyJson.size());
    request.body() = requestBodyJson;

    auto response = doRequest(attestationServiceUrl, request);

    // Process output
    if (response.result() != beast::http::status::ok) {
        SPDLOG_ERROR("Error querying Azure to validate SGX quote: {}",
                     response.body());
        throw std::runtime_error("Error validaing enclave quote");
    }
    SPDLOG_DEBUG("Received JWT from Azure Attestation Service");

    return response.body();
}

static std::string getTokenFromJwtResponse(const std::string& jwtResponse)
{
    rapidjson::Document d;
    d.Parse(jwtResponse.c_str());
    return d["token"].GetString();
}

void AzureAttestationServiceClient::validateJkuUri(const DecodedJwt& decodedJwt)
{
    std::string header = decodedJwt.get_header();
    Document d;
    d.Parse(header.c_str());
    std::string jwtJkuUri = d["jku"].GetString();

    if (jwtJkuUri != certificateEndpoint) {
        SPDLOG_ERROR("Error parsing JKU field in JWT for enclave attestation "
                     "(expected: {} - got: {})",
                     certificateEndpoint,
                     jwtJkuUri);
        throw std::runtime_error("Unexpected JKU field in JWT");
    }

    SPDLOG_DEBUG("Validated JKU origin URI");
}

JwksSet AzureAttestationServiceClient::fetchJwks()
{
    // Retrieve trusted signing keys from the attestation service
    BeastHttpRequest request(beast::http::verb::get, "/certs", 11);
    request.set("tenantName", tenantName);
    auto response = doRequest(attestationServiceUrl, request);

    // Process output
    if (response.result() != beast::http::status::ok) {
        SPDLOG_ERROR("Error querying Azure Attestation Service for the"
                     "trusted signing keys ({}): {}",
                     tenantName,
                     response.body());
        throw std::runtime_error(
          "Exception querying Azure Attestation Service");
    }

    return jwt::parse_jwks(response.body());
}

void AzureAttestationServiceClient::validateJwtSignature(
  const DecodedJwt& decodedJwt)
{
    // Get the Json Web Key (JWK) for the id that signed the token. We first
    // check against our cached key set, and refresh it only upon failure. Use
    // the JWK to get the signing certificate.
    std::string x5c = "";
    try {
        auto jwk = cachedJwks.get_jwk(decodedJwt.get_key_id());
        x5c = jwk.get_x5c_key_value();
    } catch (const jwt::error::claim_not_present_exception& e) {
        SPDLOG_WARN("JWK for JWT's Id not found in cached JWKS. Will refresh"
                    "the cache once");
        cachedJwks = fetchJwks();
        auto jwk = cachedJwks.get_jwk(decodedJwt.get_key_id());
        x5c = jwk.get_x5c_key_value();
    }
    std::string jwtIssuer = decodedJwt.get_issuer();

    // Verify signature
    if (!x5c.empty() && !jwtIssuer.empty()) {
        auto verifier =
          jwt::verify()
            .allow_algorithm(jwt::algorithm::rs256(
              jwt::helper::convert_base64_der_to_pem(x5c), "", "", ""))
            .with_issuer(jwtIssuer)
            .leeway(60UL);

        // Will throw exception upon unsuccesful verification
        verifier.verify(decodedJwt);
    } else {
        SPDLOG_ERROR("The issuer (issuer: {}) and/or the certificate (x5c: {}) "
                     "for the JWT are empty",
                     jwtIssuer,
                     x5c);
        throw std::runtime_error("Error verifying JWT signature");
    }
    SPDLOG_DEBUG("Validated JWT's signature");

    // Verify issuer matches our attestation provider
    std::string jwtIss = decodedJwt.get_payload_claim("iss").as_string();
    if (attestationServiceUrl != jwtIss) {
        SPDLOG_ERROR("Error verifying the JWT issuer against our records"
                     " (expected: {} - got: {})",
                     attestationServiceUrl,
                     jwtIss);
        throw std::runtime_error("Error verifying JWT issuer");
    }
    SPDLOG_DEBUG("Validated JWT's issuer");
}

void AzureAttestationServiceClient::validateJwtToken(
  const std::string& jwtResponse)
{
    std::string jwtToken = getTokenFromJwtResponse(jwtResponse);
    auto decodedJwt = jwt::decode(jwtToken);

    validateJkuUri(decodedJwt);
    validateJwtSignature(decodedJwt);

    SPDLOG_INFO("Validated JWT from attestation service");
}
}
