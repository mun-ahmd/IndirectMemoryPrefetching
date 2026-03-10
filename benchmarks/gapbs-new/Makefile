# See LICENSE.txt for license details.

GAPBS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
REPO_ROOT ?= $(GAPBS_DIR)../..
QEMU_RT_INC ?= $(REPO_ROOT)/prodigy_compiler_support_public/runtime/qemu
CXX_FLAGS += -std=c++11 -O3 -Wall -I$(QEMU_RT_INC)
PAR_FLAG = -fopenmp

ifneq (,$(findstring icpc,$(CXX)))
	PAR_FLAG = -openmp
endif

ifneq (,$(findstring sunCC,$(CXX)))
	CXX_FLAGS = -std=c++11 -xO3 -m64 -xtarget=native
	PAR_FLAG = -xopenmp
endif

ifneq ($(SERIAL), 1)
	CXX_FLAGS += $(PAR_FLAG)
else
	PAR_FLAG = -fno-openmp
	CXX_FLAGS += $(PAR_FLAG)
endif

KERNELS = bfs bfs_gen pr pr_gen
SUITE = $(KERNELS) converter

.PHONY: all
all: $(SUITE)

# --- LoongArch and/or Prodigy: shared paths when either is set ---
ifneq ($(PRODIGY)$(LOONGARCH),)
GAPBS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
REPO_ROOT ?= $(GAPBS_DIR)../..
GCC_TOOLCHAIN ?= /opt/x-tools/loongarch64-unknown-linux-gnu
TARGET_FLG := --target=loongarch64-unknown-linux-gnu --gcc-toolchain=$(GCC_TOOLCHAIN)
SYSROOT_FLG := --sysroot=$(GCC_TOOLCHAIN)/loongarch64-unknown-linux-gnu/sysroot
# Use GCC cross-compiler if set (e.g. LA_CXX=loongarch64-unknown-linux-gnu-g++); else clang++-18
LA_CXX ?= clang++-18
LA_LIBS := -lpthread -lrt
# Avoid unsupported relocations and .eh_frame_hdr issues with LoongArch toolchain
LA_DISABLE_EH_FLAGS := -fno-exceptions -fno-unwind-tables -fno-asynchronous-unwind-tables
LA_LINKER_EH_FLAGS := -Wl,--no-eh-frame-hdr
# Optional: set USE_LLD=1 to use ld.lld instead of GNU ld. Only use if your
# LoongArch crt/sysroot are lld-compatible (e.g. LLVM-built); GCC toolchain
# crt files often cause "unsupported object file ABI version" with lld.
LA_USE_LLD := $(if $(filter 1,$(USE_LLD)),-fuse-ld=lld,)
# Disable linker relaxation so clang emits relocations that older GNU ld supports;
# otherwise the binary may be broken and hang before main.
LA_NO_RELAX := -mno-relax
# Clang-only flags (GCC cross-compiler is already targeting LoongArch)
LA_IS_GCC_CROSS := $(findstring loongarch64,$(LA_CXX))
LA_CLANG_ONLY := $(if $(LA_IS_GCC_CROSS),,$(TARGET_FLG) $(SYSROOT_FLG) $(LA_NO_RELAX) $(LA_USE_LLD))
endif

PRODIGY_RT_DIR ?= $(REPO_ROOT)/prodigy_compiler_support_public/runtime/qemu
# --- Prodigy compiler-pass support (make PRODIGY=1) ---
ifeq ($(PRODIGY),1)
PRODIGY_PASS ?= $(REPO_ROOT)/prodigy_compiler_support_public/prefetcher/LLVMPrefetcher.so

CC := clang-18
CXX := $(LA_CXX)
OPT := opt-18

DISABLE_EH_FLAGS := -fno-exceptions -fno-unwind-tables -fno-asynchronous-unwind-tables
LINKER_EH_FLAGS := -Wl
OPT_FLAGS := -load $(PRODIGY_PASS) --bugpoint-enable-legacy-pm -prefetcher-codegen
LIBS := $(LA_LIBS)

# Step 1: C++ to LLVM IR
%.ll: src/%.cc $(wildcard src/*.h)
	$(CXX) $(TARGET_FLG) $(SYSROOT_FLG) $(DISABLE_EH_FLAGS) $(CXX_FLAGS) -S -emit-llvm -O3 -fno-openmp -fno-discard-value-names $< -o $@

# Step 2: Run Prodigy pass
%_opt.ll: %.ll
	$(OPT) $(OPT_FLAGS) -fno-openmp $< -o $@

# Step 3: IR to object
%.o: %_opt.ll
	$(CC) $(TARGET_FLG) $(SYSROOT_FLG) $(LA_NO_RELAX) -fno-openmp -O3 -c $< -o $@

# Step 4: Link with QEMU runtime
%: %.o
	@echo "Linking $@ with QEMU runtime..."
	@$(CXX) -fno-openmp $(TARGET_FLG) $(SYSROOT_FLG) $(LA_NO_RELAX) $(LA_USE_LLD) $(LINKER_EH_FLAGS) $< -L$(PRODIGY_RT_DIR) -lprefetcher_qemu_rt $(LIBS) -o $@ 2>&1 | grep -v "unsupported relocation" || true
	@if [ -f $@ ]; then echo "Success! Binary created: $@"; else echo "Error: Binary was not created"; exit 1; fi
else ifeq ($(LOONGARCH),1)
# --- LoongArch without Prodigy (make LOONGARCH=1) ---
# Use LA_CXX=loongarch64-unknown-linux-gnu-g++ if clang++-18 is not installed
CXX := $(LA_CXX)
% : src/%.cc src/*.h
	$(CXX) $(LA_CLANG_ONLY) $(LA_DISABLE_EH_FLAGS) -L$(PRODIGY_RT_DIR) -lprefetcher_qemu_rt $(CXX_FLAGS) $< $(LA_LINKER_EH_FLAGS) $(LA_LIBS) -o $@
else
# --- Default: single-step build (native) ---
# Build native x86_64 libprefetcher_qemu_rt.so (existing .so may be LoongArch from build_loongarch.sh)
QEMU_RT_SRC := $(REPO_ROOT)/prodigy_compiler_support_public/runtime/qemu/prefetcher_qemu_rt.cpp
NATIVE_QEMU_RT := $(PRODIGY_RT_DIR)/libprefetcher_qemu_rt_native.so
$(NATIVE_QEMU_RT): $(QEMU_RT_SRC)
	@echo "Building native x86_64 prefetcher runtime..."
	$(CXX) -std=c++11 -O2 -fPIC -shared -I$(QEMU_RT_INC) $< -o $@
# Link with QEMU prefetcher runtime for create_params, register_node_with_size, etc.
% : src/%.cc src/*.h $(NATIVE_QEMU_RT)
	$(CXX) $(CXX_FLAGS) $< -L$(PRODIGY_RT_DIR) -lprefetcher_qemu_rt_native -o $@
endif

# Testing
include test/test.mk

# Benchmark Automation
include benchmark/bench.mk


.PHONY: clean
clean:
	rm -f $(SUITE) test/out/*
	rm -f $(addsuffix .ll, $(SUITE)) $(addsuffix _opt.ll, $(SUITE)) $(addsuffix .o, $(SUITE))
