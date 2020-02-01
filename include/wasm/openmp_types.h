#pragma once

/*
 * Wasm definition of an openmp ident
 * Defined in openmp/runtime/src/kmp.h
 */
struct wasm_ident {
    I32 reserved_1;
    I32 flags;
    I32 reserved_2;
    I32 reserved_3;
    char const *psource;
};

// Types of source locations that call fork (defined in kmp.h)
#define KMP_IDENT_WORK_LOOP 2
