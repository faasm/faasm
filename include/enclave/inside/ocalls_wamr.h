#pragma once

#include <sgx.h>
#include <sgx_defs.h>

/* This header file defines OCalls that need to be accessible (i.e. included)
 * in WAMR code. This is needed to patch a discrepancy between the way WAMR
 * uses OCalls (directly including them from the Eder8r's generated files) and
 * the way Faasm uses them (with an extra header file). The difference in
 * approaches yields a signature difference for non-void functions, that
 * prevents us from defining OCalls to be used by WAMR. User-defined functions
 * are used in WAMR as callbacks for things like `printf` or `vprintf`
 */

extern sgx_status_t SGX_CDECL
ocallLogWamr(int* retVal, const char* msg);
