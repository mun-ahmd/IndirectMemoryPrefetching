/*
 * Prodigy Prefetcher Hint Command IDs
 *
 * These constants match the PRODIGY_CMD_* definitions in
 * prodigy_compiler_support_public/runtime/qemu/prefetcher_qemu_rt.h
 */

#ifndef PRODIGY_HINTS_H
#define PRODIGY_HINTS_H

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

#endif /* PRODIGY_HINTS_H */
