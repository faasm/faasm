#include <enclave/outside/attestation.h>

#include <iomanip>
#include <sstream>

namespace sgx {

static std::string byteArrayToHexString(const uint8_t* data, int dataSize)
{
    std::stringstream ss;
    ss << std::hex;

    for (int i = 0; i < dataSize; ++i) {
        ss << std::setw(2) << std::setfill('0') << static_cast<int>(data[i]);
    }

    return ss.str();
}

template<typename T>
static std::string intToHexString(T i)
{
    std::stringstream ss;
    ss << std::hex;

    ss << std::setw(sizeof(T) * 2) << std::setfill('0') << i;
    return ss.str();
}

// TODO - also create from file so that we can test this even if we can't test
// the quote generation step
EnclaveInfo::EnclaveInfo(const sgx_report_t& enclaveReport,
                         const std::vector<uint8_t>& quoteBuffer,
                         const std::vector<uint8_t>& enclaveHeldData)
{
    // Hard-code enclave type to 2 for SGX enclaves
    enclaveType = 2;
    mrEnclaveHex =
      byteArrayToHexString(enclaveReport.body.mr_enclave.m, SGX_HASH_SIZE);
    mrEnclaveHex =
      byteArrayToHexString(enclaveReport.body.mr_signer.m, SGX_HASH_SIZE);
    productIdHex =
      intToHexString(static_cast<uint16_t>(enclaveReport.body.isv_prod_id));
    securityVersion = static_cast<uint32_t>(enclaveReport.body.isv_svn);
    attributes = static_cast<uint64_t>(enclaveReport.body.attributes.flags);
    quoteHex = byteArrayToHexString(quoteBuffer.data(), quoteBuffer.size());
    enclaveHeldDatHex =
      byteArrayToHexString(enclaveHeldData.data(), enclaveHeldData.size());
}
}
