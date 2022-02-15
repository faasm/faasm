%DEFINE CPU_INTEL_ID_EBX "Genu"
%DEFINE CPU_INTEL_ID_ECX "ntel"
%DEFINE CPU_INTEL_ID_EDX "ineI"
%DEFINE CPU_ID_FEATURE_FLAGS_LEAF 0x7
%DEFINE CPU_ID_FEATURE_FLAGS_LEAF_BIT_SGX 0x2
%DEFINE CPU_ID_SGX_CAP_LEAF 0x12
%DEFINE FAASM_SGX_CPU_SGX_NOT_AVAILABLE 0x8
%DEFINE FAASM_SGX_CPU_SGX_V1_LEAF_NOT_AVAILABLE 0x9
%DEFINE FAASM_SGX_CPU_SGX_V2_LEAF_NOT_AVAILABLE 0xA
%DEFINE FAASM_SGX_CPU_NOT_AN_INTEL_CPU 0xB
%DEFINE FAASM_SGX_CPU_FEATURE_LEAF_NOT_AVAILABLE 0xC

section .rodata
section .data

global faasm_sgx_get_sgx_support

section .text
faasm_sgx_get_sgx_support:;There is no need to safe base pointer, hence we won't do this.
    mov r8, rbx
    xor rax, rax
    xor rcx, rcx
    cpuid
    cmp ebx, CPU_INTEL_ID_EBX
    jne .not_an_intel_cpu
    cmp ecx, CPU_INTEL_ID_ECX
    jne .not_an_intel_cpu
    cmp edx, CPU_INTEL_ID_EDX
    je .check_feature_leaf_avail
.not_an_intel_cpu:
    mov rax, FAASM_SGX_CPU_NOT_AN_INTEL_CPU
    mov rbx, r8
    ret
.check_feature_leaf_avail:
    cmp eax, CPU_ID_FEATURE_FLAGS_LEAF
    jge .check_feature_leaf
.error_feature_leaf_not_avail:
    mov rax, FAASM_SGX_CPU_FEATURE_LEAF_NOT_AVAILABLE
    mov rbx, r8
    ret
.check_feature_leaf:
    mov eax, CPU_ID_FEATURE_FLAGS_LEAF
    xor rcx, rcx
    cpuid
    test ebx, ebx
    je .error_feature_leaf_not_avail
    shr ebx, CPU_ID_FEATURE_FLAGS_LEAF_BIT_SGX
    and ebx, 0x00000001
    test ebx, ebx
    jne .check_sgx_leaf
    mov rax, FAASM_SGX_CPU_SGX_NOT_AVAILABLE
    mov rbx, r8
    ret
.check_sgx_leaf:
    mov eax, CPU_ID_SGX_CAP_LEAF
    xor rcx, rcx
    cpuid
    mov rbx, r8
    and eax, 0x00000003
    test eax, eax
    jne .check_sgx_v2_avail
    mov rax, FAASM_SGX_CPU_SGX_V1_LEAF_NOT_AVAILABLE
    ret
.check_sgx_v2_avail:
    sub eax, 0x00000001
    test eax, eax
    jne .end
    mov rax, FAASM_SGX_CPU_SGX_V2_LEAF_NOT_AVAILABLE
    ret
.end:
    xor rax, rax ;FAASM_SGX_SUCCESS
    ret
