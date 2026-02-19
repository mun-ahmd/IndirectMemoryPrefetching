# cmake file

set(PRJ_RT_NAME prefetcher_qemu_rt)

set(SOURCES ${PRJ_RT_NAME}.cpp)

add_library(${PRJ_RT_NAME} SHARED ${SOURCES})

target_include_directories(${PRJ_RT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

# Check if we should cross-compile for LoongArch
# This can be enabled by setting -DLOONGARCH_CROSS_COMPILE=ON
# and providing -DGCC_TOOLCHAIN_PATH=/path/to/loongarch-tools
# Note: For LoongArch cross-compilation, use the build_loongarch.sh script instead,
# as CMake's cross-compilation setup interferes with LLVM pass builds.
if(LOONGARCH_CROSS_COMPILE)
    message(WARNING "LOONGARCH_CROSS_COMPILE is set, but CMake cross-compilation may interfere with LLVM pass builds.")
    message(WARNING "For LoongArch runtime library, use: runtime/qemu/build_loongarch.sh")
    message(STATUS "Building ${PRJ_RT_NAME} for host architecture (use build_loongarch.sh for LoongArch)")
else()
    message(STATUS "Building ${PRJ_RT_NAME} for host architecture")
endif()

install(
  TARGETS ${PRJ_RT_NAME}
  EXPORT ${PRJ_NAME}
  ARCHIVE DESTINATION "runtime/lib"
  LIBRARY DESTINATION "runtime/lib")
