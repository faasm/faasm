#include <enclave/outside/attestation/AzureAttestationServiceClient.h>
#include <enclave/outside/attestation/EnclaveInfo.h>
#include <faabric/util/asio.h>
#include <faabric/util/logging.h>

#include <boost/beast/ssl.hpp>
#include <cppcodec/base64_url.hpp>
#include <jwt-cpp/traits/kazuho-picojson/defaults.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#define ATTESTATION_URI "/attest/SgxEnclave?api-version=2020-10-01"
#define CERTIFICATES_URI "/certs"

using namespace rapidjson;
using header = beast::http::field;
using BeastHttpRequest = faabric::util::BeastHttpRequest;
using BeastHttpResponse = faabric::util::BeastHttpResponse;

namespace sgx {

std::string AzureAttestationServiceClient::requestBodyFromEnclaveInfo(
  const EnclaveInfo& enclaveInfo)
{
    Document doc;
    doc.SetObject();
    Value outer;
    Value inner;

    Document::AllocatorType& allocator = doc.GetAllocator();

    // Specification for the JSON Format to attest SGX enclaves
    // https://docs.microsoft.com/en-us/rest/api/attestation/attestation/attest-sgx-enclave
    outer.SetObject();

    // draftPolicyForAttestation: attest against a provided draft policy rather
    // than one uploaded to the attestation service (unset)
    std::string draftPolicyForAttestation;
    outer.AddMember("draftPolicyForAttestation",
                    Value(draftPolicyForAttestation.c_str(),
                          draftPolicyForAttestation.size()),
                    allocator);

    // initTimeData: initialisation data provided when enclave is created
    // (unset)
    std::string initTimeData;
    inner.SetObject();
    inner.AddMember(
      "data", Value(initTimeData.c_str(), initTimeData.size()), allocator);
    inner.AddMember(
      "dataType", Value(initTimeData.c_str(), initTimeData.size()), allocator);
    outer.AddMember("initTimeData", inner, allocator);

    // quote: quote of the enclave to be attested
    std::vector<uint8_t> quote = enclaveInfo.getQuote();
    std::string quoteBase64 =
      cppcodec::base64_url::encode(quote.data(), quote.size());
    outer.AddMember(
      "quote", Value(quoteBase64.c_str(), quoteBase64.size()), allocator);

    // runtimeData: data provided by the enclave at quote generation time. This
    // field corresponds to the enclave held data variable that we can configure
    // before attestation.
    // 06/04/2022 - For the moment we don't include the enclave held data in
    // the request, as there is still not a clear use for it.
    std::vector<uint8_t> heldData = {};
    std::string enclaveHeldDataBase64 =
      cppcodec::base64_url::encode(heldData.data(), heldData.size());
    std::string dataType = "Binary";
    inner.SetObject();
    inner.AddMember(
      "data",
      Value(enclaveHeldDataBase64.c_str(), enclaveHeldDataBase64.size()),
      allocator);
    inner.AddMember(
      "dataType", Value(dataType.c_str(), dataType.size()), allocator);
    outer.AddMember("runtimeData", inner, allocator);

    doc.CopyFrom(outer, allocator);

    StringBuffer buffer;
    Writer<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);
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
                                   BeastHttpRequest& request)
{
    // We need to send the request over HTTPS

    // Resolve URL
    boost::asio::io_context ioc;
    boost::asio::ip::tcp::resolver resolver(ioc);
    auto results = resolver.resolve(url, "443");

    // Configure TLS context
    boost::asio::ssl::context ctx(boost::asio::ssl::context::tlsv13_client);
    ctx.set_verify_mode(boost::asio::ssl::verify_peer);
    ctx.set_default_verify_paths();

    boost::beast::ssl_stream<boost::beast::tcp_stream> stream(ioc, ctx);
    boost::beast::get_lowest_layer(stream).connect(results);
    stream.handshake(boost::asio::ssl::stream_base::client);

    // Add necessary headers
    request.set(boost::beast::http::field::host, url);
    request.set(boost::beast::http::field::user_agent,
                BOOST_BEAST_VERSION_STRING);
    request.set(boost::beast::http::field::accept, "*/*");

    beast::http::write(stream, request);

    // Process response
    beast::flat_buffer buffer;
    BeastHttpResponse response;
    beast::http::read(stream, buffer, response);

    // Close connection
    beast::error_code errorCode;
    stream.shutdown(errorCode);
    if (errorCode == boost::asio::error::eof ||
        errorCode == boost::asio::ssl::error::stream_truncated) {
        errorCode = {};
    }

    if (errorCode) {
        SPDLOG_ERROR("Error shutting down HTTP stream: {}", errorCode.value());
        throw beast::system_error(errorCode);
    }

    return response;
}

std::string AzureAttestationServiceClient::attestEnclave(
    const std::vector<uint8_t>& quote,
    sgx_report_t& report)
{
    std::vector<uint8_t> heldData(sizeof(sgx_report_data_t));
    std::memcpy(heldData.data(), &report.body.report_data, heldData.size());

    EnclaveInfo enclaveInfo(report, quote, heldData);
    return attestEnclave(enclaveInfo);
}

std::string AzureAttestationServiceClient::attestEnclave(
  const EnclaveInfo& enclaveInfo)
{
    // Prepare HTTP request
    BeastHttpRequest request(beast::http::verb::post, ATTESTATION_URI, 11);
    request.set(header::content_type, "application/json");
    std::string requestBodyJson = requestBodyFromEnclaveInfo(enclaveInfo);
    request.content_length(requestBodyJson.size());
    request.body() = requestBodyJson;

    std::string host = attestationServiceUrl;
    if (host.starts_with("https://")) {
        host = host.substr(std::string("https://").length());
    }

    auto response = doRequest(host, request);

    // Process output
    if (response.result() != beast::http::status::ok) {
        SPDLOG_ERROR("Error querying Azure to validate SGX quote ({}): {}",
                     response.result_int(),
                     response.body());
        throw std::runtime_error("Error validaing enclave quote");
    }
    SPDLOG_DEBUG("Received JWT from Azure Attestation Service");

    return response.body();
}

std::string AzureAttestationServiceClient::getTokenFromJwtResponse(const std::string& jwtResponse)
{
    rapidjson::Document doc;
    doc.Parse(jwtResponse.c_str());
    return doc["token"].GetString();
}

void AzureAttestationServiceClient::validateJkuUri(const DecodedJwt& decodedJwt)
{
    std::string header = decodedJwt.get_header();
    Document doc;
    doc.Parse(header.c_str());
    std::string jwtJkuUri = doc["jku"].GetString();

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
    BeastHttpRequest request(beast::http::verb::get, CERTIFICATES_URI, 11);
    request.set("tenantName", tenantName);

    std::string host = attestationServiceUrl;
    if (host.starts_with("https://")) {
        host = host.substr(std::string("https://").length());
    }

    auto response = doRequest(host, request);

    // Process output
    if (response.result() != beast::http::status::ok) {
        SPDLOG_ERROR("Error querying Azure Attestation Service for the"
                     "trusted signing keys ({}): {}",
                     response.result_int(),
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
    std::string x5c;
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

DecodedJwt AzureAttestationServiceClient::getDecodedJwtFromJwtResponse(const std::string& jwtResponse)
{
    std::string jwt = getTokenFromJwtResponse(jwtResponse);
    return jwt::decode(jwt);
}

void AzureAttestationServiceClient::validateJwtToken(
  const std::string& jwtToken)
{
    auto decodedJwt = getDecodedJwtFromJwtResponse(jwtToken);

    validateJkuUri(decodedJwt);
    validateJwtSignature(decodedJwt);

    SPDLOG_INFO("Validated JWT from attestation service");
}
}
