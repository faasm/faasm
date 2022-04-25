#include <enclave/outside/attestation/EnclaveInfo.h>
#include <faabric/util/bytes.h>
#include <faabric/util/logging.h>

#include <fstream>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

namespace sgx {

EnclaveInfo::EnclaveInfo(const sgx_report_t& enclaveReport,
                         const std::vector<uint8_t>& quoteBuffer,
                         const std::vector<uint8_t>& enclaveHeldDataIn)
  : quote(quoteBuffer)
  , enclaveHeldData(enclaveHeldDataIn)
{
    // Hard-code enclave type to 2 for SGX enclaves
    enclaveType = 2;
    mrEnclaveHex = faabric::util::byteArrayToHexString(
      enclaveReport.body.mr_enclave.m, SGX_HASH_SIZE);
    mrSignerHex = faabric::util::byteArrayToHexString(
      enclaveReport.body.mr_signer.m, SGX_HASH_SIZE);
    productIdHex = faabric::util::intToHexString(
      static_cast<uint16_t>(enclaveReport.body.isv_prod_id));
    securityVersion = static_cast<uint32_t>(enclaveReport.body.isv_svn);
    attributes = static_cast<uint64_t>(enclaveReport.body.attributes.flags);
}

EnclaveInfo::EnclaveInfo(const std::string& jsonPath)
{
    // Load and parse JSON
    std::string jsonStr;
    std::ifstream jsonFile(jsonPath);
    jsonFile >> jsonStr;
    rapidjson::Document d;
    d.Parse(jsonStr.c_str());

    // Initialise all attributes
    enclaveType = d["Type"].GetInt();
    mrEnclaveHex = d["MrEnclaveHex"].GetString();
    mrSignerHex = d["MrSignerHex"].GetString();
    productIdHex = d["ProductIdHex"].GetString();
    securityVersion = d["SecurityVersion"].GetInt();
    attributes = d["Attributes"].GetInt();
    std::string quoteHex = d["QuoteHex"].GetString();
    quote = faabric::util::hexStringToByteArray(quoteHex);
    std::string enclaveHeldDataHex = d["EnclaveHeldDataHex"].GetString();
    enclaveHeldData = faabric::util::hexStringToByteArray(enclaveHeldDataHex);
}

const std::vector<uint8_t>& EnclaveInfo::getQuote() const
{
    return quote;
}

const std::vector<uint8_t>& EnclaveInfo::getEnclaveHeldData() const
{
    return enclaveHeldData;
}

void EnclaveInfo::toJson(const std::string& jsonPath)
{
    // Prepare JSON document
    rapidjson::Document d;
    d.SetObject();
    rapidjson::Value outer;
    rapidjson::Document::AllocatorType& allocator = d.GetAllocator();

    // Dump contents to JSON
    outer.SetObject();
    outer.AddMember("Type", rapidjson::Value(enclaveType), allocator);
    outer.AddMember("MrEnclaveHex",
                    rapidjson::Value(mrEnclaveHex.c_str(), mrEnclaveHex.size()),
                    allocator);
    outer.AddMember("MrSignerHex",
                    rapidjson::Value(mrSignerHex.c_str(), mrSignerHex.size()),
                    allocator);
    outer.AddMember("ProductIdHex",
                    rapidjson::Value(productIdHex.c_str(), productIdHex.size()),
                    allocator);
    outer.AddMember(
      "SecurityVersion", rapidjson::Value(securityVersion), allocator);
    outer.AddMember("Attributes", rapidjson::Value(attributes), allocator);
    std::string quoteHex =
      faabric::util::byteArrayToHexString(quote.data(), quote.size());
    outer.AddMember("QuoteHex",
                    rapidjson::Value(quoteHex.c_str(), quoteHex.size()),
                    allocator);
    std::string enclaveHeldDatHex = faabric::util::byteArrayToHexString(
      enclaveHeldData.data(), enclaveHeldData.size());
    outer.AddMember(
      "EnclaveHeldDataHex",
      rapidjson::Value(enclaveHeldDatHex.c_str(), enclaveHeldDatHex.size()),
      allocator);

    // Pring JSON to file
    d.CopyFrom(outer, allocator);
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    d.Accept(writer);
    std::string jsonStr(buffer.GetString(), buffer.GetSize());
    std::ofstream of(jsonPath);
    of << jsonStr;
    if (!of.good()) {
        SPDLOG_ERROR(
          "Error writing enclave json to file ({}): {}", jsonPath, jsonStr);
        throw std::runtime_error("Error writing enclave json to file");
    }
}
}
