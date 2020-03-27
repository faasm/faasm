# Project directories
PROJ_ROOT=${CURDIR}/..
LLVM_PROJ_DIR=$(PROJ_ROOT)/third-party/llvm-project

TOOLCHAIN_DIR=$(PROJ_ROOT)/toolchain
# TOOLCHAIN_FILE=$(TOOLCHAIN_DIR)/FaasmToolchain.cmake
TOOLCHAIN_FILE=$(TOOLCHAIN_DIR)/WasiToolchain.cmake

# Install dirs
FAASM_LOCAL_DIR=/usr/local/faasm
PREFIX=$(FAASM_LOCAL_DIR)/toolchain
FAASM_SYSROOT=/usr/local/faasm/llvm-sysroot

# NOTE - upgrading LLVM itself is done by updating the submodule in this project
CLANG_VERSION=10.0.0

BUILD_DIR=$(LLVM_PROJ_DIR)/build
LLVM_CONFIG=$(BUILD_DIR)/llvm/bin/llvm-config
AR=$(BUILD_DIR)/llvm/bin/llvm-ar

WASI_LIBC_DIR=$(PROJ_ROOT)/third-party/wasi-libc

# -------------------------------------------
# This is adapted from the wasi-sdk Makefile found here:
# https://github.com/WebAssembly/wasi-sdk/blob/master/Makefile
# -------------------------------------------

default: build

.PHONY: clean-libc
clean-libc:
	rm -rf $(BUILD_DIR)/libc.BUILT $(WASI_LIBC_DIR)/build

.PHONY: clean-libs
clean-libs: clean-libc
	rm -rf $(BUILD_DIR)/compiler-rt $(BUILD_DIR)/compiler-rt.BUILT
	rm -rf $(BUILD_DIR)/libcxx $(BUILD_DIR)/libcxx.BUILT
	rm -rf $(BUILD_DIR)/libcxxabi $(BUILD_DIR)/libcxxabi.BUILT

.PHONY: clean-all
clean-all:
	rm -rf $(BUILD_DIR) $(PREFIX)

$(BUILD_DIR)/llvm.BUILT:
	mkdir -p $(BUILD_DIR)/llvm
	cd $(BUILD_DIR)/llvm; cmake -G Ninja \
		-DCMAKE_BUILD_TYPE=MinSizeRel \
		-DCMAKE_INSTALL_PREFIX=$(PREFIX) \
		-DLLVM_TARGETS_TO_BUILD=WebAssembly \
		-DLLVM_DEFAULT_TARGET_TRIPLE=wasm32-wasi \
		-DLLVM_EXTERNAL_CLANG_SOURCE_DIR=$(LLVM_PROJ_DIR)/clang \
		-DLLVM_EXTERNAL_LLD_SOURCE_DIR=$(LLVM_PROJ_DIR)/lld \
		-DLLVM_ENABLE_PROJECTS="lld;clang" \
		-DDEFAULT_SYSROOT=$(FAASM_SYSROOT) \
		$(LLVM_PROJ_DIR)/llvm
	ninja -v -C $(BUILD_DIR)/llvm \
		install-clang \
		install-clang-resource-headers \
		install-lld \
		install-llc \
		install-llvm-ar \
		install-llvm-ranlib \
		install-llvm-dwarfdump \
		install-llvm-nm \
		install-llvm-size \
		install-llvm-config
	touch $(BUILD_DIR)/llvm.BUILT

# WASI libc
$(BUILD_DIR)/libc.BUILT: $(BUILD_DIR)/llvm.BUILT
	cd $(WASI_LIBC_DIR); $(MAKE) \
		THREAD_MODEL=faasm \
		WASM_CC=$(PREFIX)/bin/clang \
		WASM_AR=$(PREFIX)/bin/llvm-ar \
		WASM_NM=$(PREFIX)/bin/llvm-nm \
		SYSROOT=$(FAASM_SYSROOT)
	# mv $(FAASM_SYSROOT)/lib/wasm32-wasi/* $(FAASM_SYSROOT)/lib/
	# rm -r $(FAASM_SYSROOT)/lib/wasm32-wasi
	touch $(BUILD_DIR)/libc.BUILT

# Musl libc
#$(BUILD_DIR)/libc.BUILT: $(BUILD_DIR)/llvm.BUILT
#	cd $(PROJ_ROOT); bash bin/build_musl.sh llvm
#	cd $(FAASM_SYSROOT)/lib; $(AR) rc libpthread.a
#	cd $(FAASM_SYSROOT)/lib; $(AR) rc libdl.a
#	touch $(BUILD_DIR)/libc.BUILT

$(BUILD_DIR)/compiler-rt.BUILT: $(BUILD_DIR)/libc.BUILT
	mkdir -p $(BUILD_DIR)/compiler-rt
	cd $(BUILD_DIR)/compiler-rt; cmake -G Ninja \
		-DCMAKE_BUILD_TYPE=RelWithDebInfo \
		-DCMAKE_TOOLCHAIN_FILE=$(TOOLCHAIN_FILE) \
		-DCOMPILER_RT_BAREMETAL_BUILD=On \
		-DCOMPILER_RT_BUILD_XRAY=OFF \
		-DCOMPILER_RT_INCLUDE_TESTS=OFF \
		-DCOMPILER_RT_HAS_FPIC_FLAG=OFF \
		-DCOMPILER_RT_ENABLE_IOS=OFF \
		-DCOMPILER_RT_DEFAULT_TARGET_ONLY=ON \
		-DLLVM_CONFIG_PATH=$(LLVM_CONFIG) \
		-DCMAKE_INSTALL_PREFIX=$(PREFIX)/lib/clang/$(CLANG_VERSION)/ \
		-DCMAKE_VERBOSE_MAKEFILE:BOOL=ON \
		-DCOMPILER_RT_OS_DIR=wasi \
		$(LLVM_PROJ_DIR)/compiler-rt/lib/builtins
	ninja -v -C $(BUILD_DIR)/compiler-rt install
	cp -R $(BUILD_DIR)/llvm/lib/clang $(PREFIX)/lib/
	# cp $(PREFIX)/lib/clang/$(CLANG_VERSION)/lib/wasi/* $(PREFIX)/lib/clang/$(CLANG_VERSION)/lib/
	touch $(BUILD_DIR)/compiler-rt.BUILT

$(BUILD_DIR)/libcxx.BUILT: $(BUILD_DIR)/compiler-rt.BUILT
	mkdir -p $(BUILD_DIR)/libcxx
	cd $(BUILD_DIR)/libcxx; cmake -G Ninja \
		-DCMAKE_TOOLCHAIN_FILE=$(TOOLCHAIN_FILE) \
		-DCMAKE_BUILD_TYPE=RelWithDebugInfo \
		-DCMAKE_CXX_COMPILER_WORKS=ON \
                -DCMAKE_C_COMPILER_WORKS=ON \
		-DCMAKE_VERBOSE_MAKEFILE:BOOL=ON \
		-DCMAKE_INSTALL_PREFIX=$(FAASM_SYSROOT) \
		-DLLVM_COMPILER_CHECKED=ON \
		-DLLVM_CONFIG_PATH=$(LLVM_CONFIG) \
		-DLIBCXX_HAVE_CXX_ATOMICS_WITHOUT_LIB=ON \
		-DLIBCXX_USE_COMPILER_RT=ON \
		-DLIBCXX_ENABLE_THREADS:BOOL=ON \
		-DLIBCXX_HAS_PTHREAD_API:BOOL=ON \
		-DLIBCXX_HAS_EXTERNAL_THREAD_API:BOOL=OFF \
		-DLIBCXX_BUILD_EXTERNAL_THREAD_LIBRARY:BOOL=OFF \
		-DLIBCXX_HAS_WIN32_THREAD_API:BOOL=OFF \
		-DLIBCXX_ENABLE_SHARED:BOOL=OFF \
		-DLIBCXX_ENABLE_PIC:BOOL=OFF \
		-DLIBCXX_ENABLE_EXPERIMENTAL_LIBRARY:BOOL=OFF \
		-DLIBCXX_ENABLE_EXCEPTIONS:BOOL=OFF \
		-DLIBCXX_ENABLE_FILESYSTEM:BOOL=OFF \
		-DLIBCXX_CXX_ABI=libcxxabi \
		-DLIBCXX_CXX_ABI_INCLUDE_PATHS=$(LLVM_PROJ_DIR)/libcxxabi/include \
		-DLIBCXX_HAS_MUSL_LIBC:BOOL=ON \
		-DLIBCXX_ABI_VERSION=2 \
		-DLIBCXX_LIBDIR_SUFFIX=/wasm32-wasi \
		--debug-trycompile \
		$(LLVM_PROJ_DIR)/libcxx
	ninja -v -C $(BUILD_DIR)/libcxx install
	touch $(BUILD_DIR)/libcxx.BUILT

$(BUILD_DIR)/libcxxabi.BUILT: $(BUILD_DIR)/libcxx.BUILT
	mkdir -p $(BUILD_DIR)/libcxxabi
	cd $(BUILD_DIR)/libcxxabi; cmake -G Ninja \
		-DCMAKE_BUILD_TYPE=RelWithDebugInfo \
		-DCMAKE_C_FLAGS="-I$(FAASM_SYSROOT)/include" \
		-DCMAKE_CXX_FLAGS="-I$(FAASM_SYSROOT)/include/c++/v1" \
		-DCMAKE_TOOLCHAIN_FILE=$(TOOLCHAIN_FILE) \
	        -DLLVM_CONFIG_PATH=$(LLVM_CONFIG) \
		-DCMAKE_CXX_COMPILER_WORKS=ON \
		-DCMAKE_C_COMPILER_WORKS=ON \
		-DCMAKE_INSTALL_PREFIX=$(FAASM_SYSROOT) \
		-DLLVM_COMPILER_CHECKED=ON \
                -DLIBCXXABI_USE_COMPILER_RT=ON \
		-DLIBCXXABI_ENABLE_EXCEPTIONS:BOOL=OFF \
		-DLIBCXXABI_ENABLE_SHARED:BOOL=OFF \
		-DLIBCXXABI_SILENT_TERMINATE:BOOL=ON \
		-DLIBCXXABI_ENABLE_THREADS:BOOL=ON \
		-DLIBCXXABI_HAS_PTHREAD_API:BOOL=ON \
		-DLIBCXXABI_HAS_EXTERNAL_THREAD_API:BOOL=OFF \
		-DLIBCXXABI_BUILD_EXTERNAL_THREAD_LIBRARY:BOOL=OFF \
		-DLIBCXXABI_HAS_WIN32_THREAD_API:BOOL=OFF \
		-DLIBCXXABI_ENABLE_PIC:BOOL=OFF \
		-DLIBCXXABI_LIBCXX_PATH=$(LLVM_PROJ_DIR)/libcxx \
		-DLIBCXXABI_LIBCXX_INCLUDES=$(FAASM_SYSROOT)/include/c++/v1 \
		-DUNIX:BOOL=ON \
                -DLIBCXXABI_LIBDIR_SUFFIX=/wasm32-wasi \
		--debug-trycompile \
		$(LLVM_PROJ_DIR)/libcxxabi
	ninja -v -C $(BUILD_DIR)/libcxxabi install
	touch $(BUILD_DIR)/libcxxabi.BUILT

.PHONY: extras
extras: $(BUILD_DIR)/libcxxabi.BUILT
	cp $(TOOLCHAIN_DIR)/sysroot_extras/* $(FAASM_SYSROOT)/lib/wasm32-wasi/

libc: $(BUILD_DIR)/libc.BUILT

libcxx: $(BUILD_DIR)/libcxx.BUILT

libcxxabi: $(BUILD_DIR)/libcxxabi.BUILT

compiler-rt: $(BUILD_DIR)/compiler-rt.BUILT

build: $(BUILD_DIR)/llvm.BUILT $(BUILD_DIR)/libc.BUILT $(BUILD_DIR)/compiler-rt.BUILT $(BUILD_DIR)/libcxxabi.BUILT $(BUILD_DIR)/libcxx.BUILT extras
