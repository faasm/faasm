#include <enclave/outside/getSgxSupport.h>
#include <faabric/util/logging.h>

#include <sgx_capable.h>

namespace sgx {

bool isSgxEnabled()
{
    // First check if the machine is capable
    int isSgxCapable;
    sgx_status_t sgxStatus = sgx_is_capable(&isSgxCapable);
    if (sgxStatus != SGX_SUCCESS) {
        SPDLOG_ERROR("Call to check if system is SGX-capable failed");
        return false;
    } else if (isSgxCapable == 0) {
        SPDLOG_ERROR("SGX is not available in the system");
        return false;
    }

    // Second, check if the mahcine is enabled
    sgx_device_status_t sgxDeviceStatus;
    sgxStatus = sgx_cap_get_status(&sgxDeviceStatus);
    if (sgxStatus != SGX_SUCCESS) {
        SPDLOG_ERROR("Call to check if SGX is enabled in the system failed");
        return false;
    }

    switch (sgxDeviceStatus) {
        case SGX_ENABLED:
            SPDLOG_INFO("SGX is enabled and ready to be used");
            return true;
        case SGX_DISABLED_REBOOT_REQUIRED: {
            SPDLOG_ERROR("A reboot is required to finish enabling SGX");
            break;
        }
        case SGX_DISABLED_LEGACY_OS: {
            SPDLOG_ERROR(
              "SGX is disabled and no SCI is available to enable it");
            break;
        }
        case SGX_DISABLED: {
            SPDLOG_ERROR("SGX is not enabled on this platform");
            break;
        }
        case SGX_DISABLED_SCI_AVAILABLE: {
            SPDLOG_ERROR("SGX is disabled but a SCI is available to enable it");
            break;
        }
        case SGX_DISABLED_MANUAL_ENABLE: {
            SPDLOG_ERROR(
              "SGX is disabled but can be enabled manually in the BIOS");
            break;
        }
        case SGX_DISABLED_HYPERV_ENABLED: {
            SPDLOG_ERROR("Detected an unsupported version of hypervisor");
            break;
        }
        case SGX_DISABLED_UNSUPPORTED_CPU: {
            SPDLOG_ERROR("SGX is not supported by this CPU");
            break;
        }
    }

    return false;
}
}
