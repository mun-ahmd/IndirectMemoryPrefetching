# QEMU Runtime Library Build Instructions

## Host Architecture Build

The runtime library is built automatically as part of the main CMake build process for the host architecture.

## LoongArch Cross-Compilation

To build the runtime library for LoongArch (for use with QEMU emulation), use the provided build script:

```bash
cd runtime/qemu
./build_loongarch.sh [GCC_TOOLCHAIN_PATH]
```

If `GCC_TOOLCHAIN_PATH` is not provided, it defaults to `/home/muneeb/repos/IndirectMemoryPrefetching/loongarch-tools`.

The script will:
1. Use `clang-18` and `clang++-18` for cross-compilation
2. Set the target to `loongarch64-unknown-linux-gnu`
3. Use the provided GCC toolchain and sysroot
4. Build `libprefetcher_qemu_rt.so` for LoongArch

After building, the LoongArch library will be placed in the `runtime/qemu/` directory.

**Note:** The LLVM pass (`LLVMPrefetcher.so`) should always be built for the host architecture, as it runs on the host machine during compilation. Only the runtime library needs to be cross-compiled for LoongArch.
