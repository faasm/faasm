;
; Created by Joshua Heinemann on 26.06.20.
; TU-Braunschweig (heineman@ibr.cs.tu-bs.de)
;

%DEFINE CPU_ID_FEATURE_FLAGS_LEAF 0x7
%DEFINE CPU_ID_FEATURE_FLAGS_LEAF_BIT_SGX 0x2
%DEFINE CPU_ID_SGX_CAP_LEAF 0x12
%DEFINE FAASM_SGX_SUCCESS 0x0
%DEFINE FAASM_SGX_SGX_NOT_AVAILABLE 0x8
%DEFINE FAASM_SGX_SGX_V1_LEAF_NOT_AVAILABLE 0x9
%DEFINE FAASM_SGX_SGX_V2_LEAF_NOT_AVAILABLE 0xa

section .rodata
section .data

global faasm_sgx_get_sgx_support

section .text
faasm_sgx_get_sgx_support:
    push rbx
    mov eax, CPU_ID_FEATURE_FLAGS_LEAF
    xor ecx, ecx
    cpuid
    shr ebx, CPU_ID_FEATURE_FLAGS_LEAF_BIT_SGX
    and ebx, 0x00000001
    test ebx, ebx
    jne .check_sgx_leaf
    mov rax, FAASM_SGX_SGX_NOT_AVAILABLE
    pop rbx
    ret
.check_sgx_leaf:
    mov eax, CPU_ID_SGX_CAP_LEAF
    xor ecx, ecx
    cpuid
    pop rbx
    and eax, 0x00000011
    test eax, eax
    jne .check_sgx_v2_avail
    mov eax, FAASM_SGX_SGX_V1_LEAF_NOT_AVAILABLE
    ret
.check_sgx_v2_avail:
    sub eax, 0x00000001
    test eax, eax
    jne .end
    mov eax, FAASM_SGX_SGX_V2_LEAF_NOT_AVAILABLE
    ret
.end:
    xor eax, eax ;FAASM_SGX_SUCCESS
    ret