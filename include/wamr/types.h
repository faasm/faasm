#pragma once

#include <wasmtime_ssp.h>

struct iovec_app_t
{
    uint32_t buffOffset;
    uint32_t buffLen;
};

struct wasi_prestat_app_t
{
    __wasi_preopentype_t pr_type;
    uint32_t pr_name_len;
};

