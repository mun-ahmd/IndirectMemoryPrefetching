/*
 * QEMU Syscall-based Prefetcher Runtime
 *
 * Instead of Sniper's __asm__ hooks, all prefetcher metadata is communicated
 * via a reserved syscall number (0xDEAD). The QEMU plugin intercepts these
 * syscalls and embeds the hint data directly in the instruction trace.
 *
 * Protocol
 * --------
 * Each prefetcher function makes a syscall with:
 *   syscall(0xDEAD, cmd, a0, a1, a2, a3, a4)
 *
 * Where:
 *   - 0xDEAD is the reserved syscall number (LoongArch syscalls max at ~450)
 *   - cmd is one of PRODIGY_CMD_* constants
 *   - a0..a4 are up to 5 uint64 arguments
 *
 * QEMU returns -ENOSYS to the guest (which we ignore), but the plugin
 * intercepts the syscall before it's processed and marks the current
 * instruction trace record with the hint data.
 */

#ifndef PREFETCHER_QEMU_RT_H
#define PREFETCHER_QEMU_RT_H

#include <unistd.h>
#include <sys/syscall.h>
#include <cstdint>

#define PRODIGY_SYSCALL_NR 0xDEAD

/* Command IDs passed as first argument to syscall */
#define PRODIGY_CMD_CREATE_PARAMS           1
#define PRODIGY_CMD_CREATE_ENABLE           2
#define PRODIGY_CMD_REGISTER_NODE_WITH_SIZE 3
#define PRODIGY_CMD_REGISTER_TRAV_EDGE      4
#define PRODIGY_CMD_REGISTER_TRIG_EDGE      5
#define PRODIGY_CMD_ROI_START               6
#define PRODIGY_CMD_ROI_END                 7
#define PRODIGY_CMD_PF_SET_PARAM            8
#define PRODIGY_CMD_PF_SET_ENABLE           9
#define PRODIGY_CMD_PF_ENABLE              10
#define PRODIGY_CMD_PF_DISABLE             11
#define PRODIGY_CMD_PF_WAIT                12
#define PRODIGY_CMD_DELETE_PARAMS          13
#define PRODIGY_CMD_DELETE_ENABLE          14
#define PRODIGY_CMD_IDENTIFY_EDGE          15
#define PRODIGY_CMD_IDENTIFY_EDGE_SOURCE   16
#define PRODIGY_CMD_IDENTIFY_EDGE_TARGET   17

static inline long prodigy_hint(uint64_t cmd,
    uint64_t a0 = 0, uint64_t a1 = 0, uint64_t a2 = 0,
    uint64_t a3 = 0, uint64_t a4 = 0) {
    return syscall(PRODIGY_SYSCALL_NR, cmd, a0, a1, a2, a3, a4);
}

#endif /* PREFETCHER_QEMU_RT_H */
