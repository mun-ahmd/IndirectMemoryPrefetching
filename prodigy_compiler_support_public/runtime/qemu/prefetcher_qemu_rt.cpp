/*
 * QEMU Syscall-based Prefetcher Runtime
 *
 * Drop-in replacement for the Sniper-based default runtime.
 * Every function the LLVM codegen pass can emit a call to is implemented here.
 * Instead of Sniper asm hooks, each function makes a syscall with a reserved
 * syscall number; QEMU intercepts these syscalls and embeds the hint data
 * directly in the instruction trace.
 *
 * See prefetcher_qemu_rt.h for the syscall protocol and command IDs.
 */

#include "prefetcher_qemu_rt.h"

#include <cstdint>
#include <cstdio>

extern "C" {

/* ------------------------------------------------------------------ */
/*  create / delete                                                    */
/* ------------------------------------------------------------------ */

int create_params(int num_nodes_pf, int num_edges_pf, int num_triggers_pf) {
    prodigy_hint(PRODIGY_CMD_CREATE_PARAMS,
                 (uint64_t)num_nodes_pf,
                 (uint64_t)num_edges_pf,
                 (uint64_t)num_triggers_pf);
    return 0;
}

int create_enable() {
    prodigy_hint(PRODIGY_CMD_CREATE_ENABLE);
    return 0;
}

int delete_params() {
    prodigy_hint(PRODIGY_CMD_DELETE_PARAMS);
    return 0;
}

int delete_enable() {
    prodigy_hint(PRODIGY_CMD_DELETE_ENABLE);
    return 0;
}

/* ------------------------------------------------------------------ */
/*  node registration                                                  */
/* ------------------------------------------------------------------ */

int register_node_with_size(uintptr_t base, int64_t size,
                            int64_t elem_size, int64_t node_id) {
    prodigy_hint(PRODIGY_CMD_REGISTER_NODE_WITH_SIZE,
                 (uint64_t)base,
                 (uint64_t)size,
                 (uint64_t)elem_size,
                 (uint64_t)node_id);
    return 0;
}

/* ------------------------------------------------------------------ */
/*  traversal edge registration                                        */
/* ------------------------------------------------------------------ */

__attribute__((noinline))
int register_trav_edge1(uintptr_t baseaddr_from, uintptr_t baseaddr_to,
                        uint32_t func_id, int edge_id) {
    prodigy_hint(PRODIGY_CMD_REGISTER_TRAV_EDGE,
                 (uint64_t)baseaddr_from,
                 (uint64_t)baseaddr_to,
                 (uint64_t)func_id,
                 (uint64_t)edge_id);
    return 0;
}

int register_trav_edge2(int64_t id_from, int64_t id_to, uint32_t func_id) {
    prodigy_hint(PRODIGY_CMD_REGISTER_TRAV_EDGE,
                 (uint64_t)id_from,
                 (uint64_t)id_to,
                 (uint64_t)func_id);
    return 0;
}

/* ------------------------------------------------------------------ */
/*  trigger edge registration                                          */
/* ------------------------------------------------------------------ */

int register_trig_edge1(uintptr_t baseaddr_from, uintptr_t baseaddr_to,
                        uint32_t func_id, uint32_t squash_func_id) {
    prodigy_hint(PRODIGY_CMD_REGISTER_TRIG_EDGE,
                 (uint64_t)baseaddr_from,
                 (uint64_t)baseaddr_to,
                 (uint64_t)func_id,
                 (uint64_t)squash_func_id);
    return 0;
}

int register_trig_edge2(int64_t id_from, int64_t id_to,
                        uint32_t func_id, uint32_t squash_func_id) {
    prodigy_hint(PRODIGY_CMD_REGISTER_TRIG_EDGE,
                 (uint64_t)id_from,
                 (uint64_t)id_to,
                 (uint64_t)func_id,
                 (uint64_t)squash_func_id);
    return 0;
}

/* ------------------------------------------------------------------ */
/*  edge identification (profiling)                                    */
/* ------------------------------------------------------------------ */

int register_identify_edge(uintptr_t baseaddr_from, uintptr_t baseaddr_to,
                           uint32_t func_id) {
    prodigy_hint(PRODIGY_CMD_IDENTIFY_EDGE,
                 (uint64_t)baseaddr_from,
                 (uint64_t)baseaddr_to,
                 (uint64_t)func_id);
    return 0;
}

int register_identify_edge_source(uintptr_t baseaddr_from, int edge_id) {
    prodigy_hint(PRODIGY_CMD_IDENTIFY_EDGE_SOURCE,
                 (uint64_t)baseaddr_from,
                 (uint64_t)edge_id);
    return 0;
}

int register_identify_edge_target(uintptr_t baseaddr_to, int edge_id) {
    prodigy_hint(PRODIGY_CMD_IDENTIFY_EDGE_TARGET,
                 (uint64_t)baseaddr_to,
                 (uint64_t)edge_id);
    return 0;
}

/* ------------------------------------------------------------------ */
/*  simulator control                                                  */
/* ------------------------------------------------------------------ */

int sim_user_pf_set_param() {
    prodigy_hint(PRODIGY_CMD_PF_SET_PARAM);
    return 0;
}

int sim_user_pf_set_enable() {
    prodigy_hint(PRODIGY_CMD_PF_SET_ENABLE);
    return 0;
}

int sim_user_pf_enable() {
    prodigy_hint(PRODIGY_CMD_PF_ENABLE);
    return 0;
}

int sim_user_pf_disable() {
    prodigy_hint(PRODIGY_CMD_PF_DISABLE);
    return 0;
}

int sim_user_wait() {
    prodigy_hint(PRODIGY_CMD_PF_WAIT);
    return 0;
}

int sim_roi_start() {
    prodigy_hint(PRODIGY_CMD_ROI_START);
    return 0;
}

int sim_roi_end() {
    prodigy_hint(PRODIGY_CMD_ROI_END);
    return 0;
}

} /* extern "C" */
