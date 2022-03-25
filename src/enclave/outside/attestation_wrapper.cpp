#include <enclave/outside/system.h>
#include <faabric/util/logging.h>

#include <cstdio>
#include <cstdlib>

// An enclave quote is a cryptographic measure of the contents loaded in
// the enclave. Enclave quotes can be validated through a third-party to
// attest that code is running in an untampered genuine enclave.
// Note that, for performance reasons, we could generate the quote as we
// initialise WAMR and not as a different ECall.
/*
static bool validateQuote(EnclaveQuote enclaveQuote,
                      std::string attestationProviderUrl,
                      bool includeDetails)
{

sgx_report_data_t hash;
sha256sum(&enclaveHeldData[0], enclaveHeldData.size(), hash.d);

sgx_report_t app_report;
if(true != create_app_enclave_report(enclavePath.c_str(), qe_target_info,
&app_report, &hash)) { throw runtime_error("Call to create_app_report()
failed\n");
}

//get quote size
uint32_t quote_size = 0;
qe3_ret = sgx_qe_get_quote_size(&quote_size);
if (SGX_QL_SUCCESS != qe3_ret) {
    throw runtime_error("Error in sgx_qe_get_quote_size." + qe3_ret);
}

//allocate the a buffer for the quote
uint8_t* p_quote_buffer = (uint8_t*)malloc(quote_size);
if (NULL == p_quote_buffer) {
    if (NULL != p_quote_buffer) {
        free(p_quote_buffer);
    }
    throw runtime_error("\nCouldn't allocate quote_buffer\n");
}
memset(p_quote_buffer, 0, quote_size);

//fill the quote with actual data
qe3_ret = sgx_qe_get_quote(&app_report,
                           quote_size,
                           p_quote_buffer);
if (SGX_QL_SUCCESS != qe3_ret) {
    if (NULL != p_quote_buffer) {
        free(p_quote_buffer);
    }
    throw runtime_error("Error in sgx_qe_get_quote." + qe3_ret);
}

//fill the enclave info
EnclaveInfo enclaveInfo;

const int hex_buffer_size = 1024*64;
char hex_buffer[hex_buffer_size];

enclaveInfo.Type = (int) 2; //for OE, see
https://github.com/openenclave/openenclave/blob/3e15573418caed43f9094ff8aec36cdde4f278f7/include/openenclave/bits/types.h#L127
enclaveInfo.MrEnclaveHex = format_hex_buffer(hex_buffer, hex_buffer_size,
app_report.body.mr_enclave.m, SGX_HASH_SIZE); enclaveInfo.MrSignerHex =
format_hex_buffer(hex_buffer, hex_buffer_size, app_report.body.mr_signer.m,
SGX_HASH_SIZE); enclaveInfo.ProductIdHex =  uint16_to_buffer(hex_buffer,
hex_buffer_size,(uint16_t)app_report.body.isv_prod_id, 16);
enclaveInfo.SecurityVersion = (int) app_report.body.isv_svn;
enclaveInfo.Attributes = (uint64_t)app_report.body.attributes.flags;
enclaveInfo.QuoteHex = format_hex_buffer(hex_buffer, hex_buffer_size,
p_quote_buffer, quote_size); enclaveInfo.EnclaveHeldDataHex =
format_hex_buffer(hex_buffer, hex_buffer_size, &enclaveHeldData[0],
enclaveHeldData.size());

return enclaveInfo;
return true;
}
*/
namespace sgx {

bool attestEnclaveWrapper(std::string attestationProviderUrl,
                          bool includeDetails,
                          std::string enclavePath,
                          std::vector<uint8_t> enclaveHeldData)
{
    // TODO - include other parameters
    // Execute the attestation binary command
    char attCmd[128];
    int retVal = sprintf(attCmd,
                         "%s %i %s %s",
                         FAASM_DCAP_ATTESTATION_BINARY,
                         sgx::getGlobalEnclaveId(),
                         attestationProviderUrl.c_str(),
                         enclavePath.c_str());
    if (retVal < 0) {
        SPDLOG_ERROR("Error preparing attestation command: '{} {} {}'",
                     FAASM_DCAP_ATTESTATION_BINARY,
                     attestationProviderUrl,
                     enclavePath);
        return false;
    }
    SPDLOG_INFO("Running attestation command: {}", attCmd);

    retVal = std::system(attCmd);
    if (retVal < 0) {
        SPDLOG_ERROR("Error running attestation command: {}", attCmd);
        return false;
    }

    SPDLOG_INFO("Succesfuly attested enclave {} with provider {}",
                enclavePath,
                attestationProviderUrl);
    return true;
}
}
