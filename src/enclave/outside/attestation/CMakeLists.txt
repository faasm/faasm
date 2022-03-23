add_library(attestation
	generateQuote.cpp
)

target_include_directories(attestation PUBLIC
    	${SGX_SDK_PATH}/include
    	${SGX_DCAP_QUOTE_GENERATION_INCLUDE}/ql/inc
)
target_link_libraries(attestation
	faasm::common_deps
	${SGX_CAPABLE_LIB}
	${SGX_UNTRUSTED_RUNTIME_LIB}
	${SGX_DCAP_LIB}
	# ${SGX_UAE_SERVICE_LIB}
	Threads::Threads
)
