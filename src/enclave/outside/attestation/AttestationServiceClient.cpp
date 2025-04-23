#include <enclave/outside/attestation/AttestationServiceClient.h>
#include <enclave/outside/attestation/EnclaveInfo.h>
#include <faabric/util/asio.h>
#include <faabric/util/logging.h>

#include <boost/beast/ssl.hpp>
#include <cppcodec/base64_url.hpp>
#include <jwt-cpp/traits/kazuho-picojson/defaults.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

using namespace rapidjson;
using header = beast::http::field;
using BeastHttpRequest = faabric::util::BeastHttpRequest;
using BeastHttpResponse = faabric::util::BeastHttpResponse;

namespace sgx {

// Even though we don't use Azure's Attestation service anymore, we use the
// same JWT format in case we ever want to revert back to using MAA
std::string AttestationServiceClient::requestBodyFromEnclaveInfo(
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
    std::string enclaveHeldDataBase64 =
      cppcodec::base64_url::encode(enclaveInfo.getEnclaveHeldData().data(),
                                   enclaveInfo.getEnclaveHeldData().size());
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

AttestationServiceClient::AttestationServiceClient(
  const std::string& attestationServiceUrlIn)
  : attestationServiceUrl(attestationServiceUrlIn)
  , verifyReportEndpoint("/verify-sgx-report")
{}

static BeastHttpResponse doRequest(const std::string& ip,
                                   const std::string& port,
                                   BeastHttpRequest& request)
{
    // We need to send the request over HTTPS
    boost::asio::io_context ioc;

    // Configure TLS context
    boost::asio::ssl::context ctx(boost::asio::ssl::context::tlsv13_client);
    ctx.set_default_verify_paths();
    // We don't check the AS' certificates here, but we will validate the
    // signature inside the enclave
    ctx.set_verify_mode(boost::asio::ssl::verify_none);

    boost::beast::ssl_stream<boost::beast::tcp_stream> stream(ioc, ctx);
    boost::asio::ip::tcp::endpoint endpoint(asio::ip::make_address(ip),
                                            std::stoi(port));
    beast::get_lowest_layer(stream).connect(endpoint);
    stream.handshake(boost::asio::ssl::stream_base::client);

    // Add necessary headers
    request.set(boost::beast::http::field::host, ip);
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

std::string AttestationServiceClient::attestEnclave(
  const std::vector<uint8_t>& quote,
  sgx_report_t& report)
{
    std::vector<uint8_t> heldData(SGX_REPORT_DATA_SIZE);
    std::memcpy(heldData.data(), &report.body.report_data, heldData.size());

    EnclaveInfo enclaveInfo(report, quote, heldData);
    return attestEnclave(enclaveInfo);
}

std::pair<std::string, std::string> splitIpPort(const std::string& input)
{
    size_t colonPos = input.find(':');
    if (colonPos == std::string::npos || colonPos == 0 ||
        colonPos == input.size() - 1) {
        SPDLOG_ERROR("Invalid ip:port format for input: {}", input);
        throw std::invalid_argument("Invalid ip:port format");
    }

    std::string ip = input.substr(0, colonPos);
    std::string port = input.substr(colonPos + 1);
    return { ip, port };
}

std::string AttestationServiceClient::attestEnclave(
  const EnclaveInfo& enclaveInfo)
{
    // Prepare HTTP request
    BeastHttpRequest request(beast::http::verb::post, "/verify-sgx-report", 11);
    request.set(header::content_type, "application/json");
    std::string requestBodyJson = requestBodyFromEnclaveInfo(enclaveInfo);
    request.content_length(requestBodyJson.size());
    request.body() = requestBodyJson;

    std::string host = this->attestationServiceUrl;
    if (host.starts_with("https://")) {
        host = host.substr(std::string("https://").length());
    }

    auto [ip, port] = splitIpPort(host);
    auto response = doRequest(ip, port, request);

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

std::pair<std::string, std::string>
AttestationServiceClient::getTokenFromJwtResponse(
  const std::string& jwtResponse)
{
    rapidjson::Document doc;
    doc.Parse(jwtResponse.c_str());
    std::string encryptedJwt = doc["encrypted_token"].GetString();
    std::string serverPubKey = doc["server_pubkey"].GetString();
    return std::pair<std::string, std::string>(encryptedJwt, serverPubKey);
}
}
