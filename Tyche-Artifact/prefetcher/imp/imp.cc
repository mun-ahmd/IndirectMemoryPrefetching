/*
 * IMP: Indirect Memory Prefetcher
 *
 * Based on: "IMP: Indirect Memory Prefetcher"
 * by Xiangyao Yu, Christopher J. Hughes, Nadathur Satish, Srinivas Devadas
 * (MICRO-48, 2015)
 *
 * Key idea: Detects indirect memory access patterns of the form A[B[i]]
 * where B is an index array scanned sequentially and A is a data array
 * accessed indirectly. IMP detects the streaming pattern on B, learns the
 * relationship ADDR(A[B[i]]) = (B[i] << shift) + BaseAddr, and prefetches
 * A[B[i+delta]] using future index values.
 *
 * Components:
 *   1. Prefetch Table (PT) with Stream Table + Indirect Table
 *   2. Indirect Pattern Detector (IPD)
 *   3. Address Generator for indirect prefetches
 */

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <iostream>

#include "cache.h"
#include "memory_data.h"

extern MEMORY_DATA mem_data[NUM_CPUS];

// ====================================================================
// IMP Configuration Parameters (from Table 2 of the IMP paper)
// ====================================================================
#define IMP_PT_SIZE              16   // Prefetch Table entries
#define IMP_IPD_SIZE             4    // Indirect Pattern Detector entries
#define IMP_NUM_SHIFTS           4    // Number of shift values considered
#define IMP_BA_ARRAY_LEN         4    // Max misses tracked per IPD phase
#define IMP_MAX_PREF_DIST        16   // Maximum indirect prefetch distance
#define IMP_STREAM_CONF_THRESH   2    // Stream confidence to activate IPD feeding
#define IMP_STREAM_PF_THRESH     4    // Stream confidence to start stream prefetching
#define IMP_INDIRECT_CONF_THRESH 2    // Indirect confidence to start indirect prefetching
#define IMP_STREAM_CONF_MAX      7    // Max stream confidence (saturating counter)
#define IMP_INDIRECT_CONF_MAX    7    // Max indirect confidence (saturating counter)
#define IMP_STREAM_PF_DIST       4    // Stream (index array) prefetch distance

// Shift values: 2 => Coeff=4 (int32), 3 => Coeff=8 (double/ptr),
//               4 => Coeff=16 (struct), -3 => Coeff=1/8 (bitvector)
static const int SHIFT_VALUES[IMP_NUM_SHIFTS] = {2, 3, 4, -3};

// Memory read size constants (matching ins.h values)
static const uint8_t MEM_SZ_BYTE  = 1;
static const uint8_t MEM_SZ_HWORD = 2;
static const uint8_t MEM_SZ_WORD  = 3;
static const uint8_t MEM_SZ_DWORD = 4;

// ====================================================================
// Helper Functions
// ====================================================================

// Compute indirect address: ADDR(A[B[i]]) = (index << shift) + base
static inline uint64_t compute_indirect_addr(uint64_t index, int shift, uint64_t base) {
    if (shift >= 0)
        return (index << shift) + base;
    else
        return (index >> (-shift)) + base;
}

// Compute BaseAddr from a known (miss_addr, index, shift) triple
static inline uint64_t compute_base_addr(uint64_t miss_addr, uint64_t index, int shift) {
    if (shift >= 0)
        return miss_addr - (index << shift);
    else
        return miss_addr - (index >> (-shift));
}

// Check if address is valid for prefetching
static inline bool is_valid_pf_addr(uint64_t addr) {
    return addr != 0 && !(addr & 0xffffff0000000000ULL);
}

// Read an index value from simulated memory; element size inferred from stride
static uint64_t read_index_value(uint32_t cpu, uint64_t addr, int64_t stride) {
    int64_t abs_stride = (stride >= 0) ? stride : -stride;
    switch (abs_stride) {
        case 1:  return (uint64_t)(uint8_t) mem_data[cpu].read(addr, MEM_SZ_BYTE,  false);
        case 2:  return (uint64_t)(uint16_t)mem_data[cpu].read(addr, MEM_SZ_HWORD, false);
        case 8:  return (uint64_t)          mem_data[cpu].read(addr, MEM_SZ_DWORD, false);
        case 4:
        default: return (uint64_t)(uint32_t)mem_data[cpu].read(addr, MEM_SZ_WORD,  false);
    }
}

// ====================================================================
// Prefetch Table (PT) Entry
// Each entry combines a Stream Table portion and an Indirect Table portion.
// The Stream Table detects sequential (streaming) access to an index array.
// The Indirect Table stores the learned indirect pattern parameters.
// ====================================================================
struct PT_Entry {
    // --- Stream Table ---
    bool     stream_valid    = false;
    uint64_t pc              = 0;     // PC of the stream access instruction
    uint64_t addr            = 0;     // Most recently accessed address in the stream
    int64_t  stride          = 0;     // Stream stride in bytes
    uint8_t  stream_conf     = 0;     // Saturating confidence counter

    // --- Indirect Table ---
    bool     indirect_enable = false; // True once IPD detects an indirect pattern
    int      shift_idx       = -1;    // Index into SHIFT_VALUES[]
    uint64_t base_addr       = 0;     // BaseAddr of the indirect pattern
    uint8_t  indirect_conf   = 0;     // Confidence counter for indirect pattern
    uint64_t index_value     = 0;     // Last index value stored for verification
    bool     index_valid     = false; // Whether index_value is pending verification
    bool     prefetch_active = false; // True when indirect confidence >= threshold
    uint8_t  pref_distance   = 1;     // Current indirect prefetch distance (increases)

    // --- LRU ---
    uint64_t last_used_cycle = 0;

    void reset() {
        stream_valid = false; pc = 0; addr = 0; stride = 0;
        stream_conf = 0;
        reset_indirect();
        last_used_cycle = 0;
    }

    void reset_indirect() {
        indirect_enable = false; shift_idx = -1; base_addr = 0;
        indirect_conf = 0; index_value = 0; index_valid = false;
        prefetch_active = false; pref_distance = 1;
    }
};

// ====================================================================
// Indirect Pattern Detector (IPD) Entry
// Detects indirect patterns by correlating stream index values with
// subsequent cache misses. Uses two index values (idx1, idx2) and
// checks if any (shift, BaseAddr) pair satisfies Eq. (2) for both.
// ====================================================================
struct IPD_Entry {
    bool     valid    = false;
    uint32_t pt_idx   = 0;      // Associated PT entry index

    uint64_t idx1     = 0;      // First index value B[i]
    uint64_t idx2     = 0;      // Second index value B[i+1]
    bool     has_idx2 = false;

    // BaseAddr candidates from misses after idx1: [shift_idx][miss_idx]
    uint64_t ba_idx1[IMP_NUM_SHIFTS][IMP_BA_ARRAY_LEN];
    uint8_t  miss_cnt_1 = 0;    // Misses recorded after idx1
    uint8_t  miss_cnt_2 = 0;    // Misses recorded after idx2

    // Exponential backoff for failed detections
    uint32_t backoff_count = 0;
    uint64_t backoff_until = 0; // Cycle until which we wait before retrying

    void reset() {
        valid = false; pt_idx = 0;
        idx1 = 0; idx2 = 0; has_idx2 = false;
        miss_cnt_1 = 0; miss_cnt_2 = 0;
        memset(ba_idx1, 0, sizeof(ba_idx1));
        backoff_count = 0; backoff_until = 0;
    }
};

// ====================================================================
// Per-CPU IMP State
// ====================================================================
static PT_Entry  imp_pt [NUM_CPUS][IMP_PT_SIZE];
static IPD_Entry imp_ipd[NUM_CPUS][IMP_IPD_SIZE];

// Backoff state per PT entry (tracks failed IPD attempts)
static uint32_t pt_backoff_count[NUM_CPUS][IMP_PT_SIZE];
static uint64_t pt_backoff_until[NUM_CPUS][IMP_PT_SIZE];

// ====================================================================
// Statistics
// ====================================================================
static uint64_t stat_stream_pf[NUM_CPUS];
static uint64_t stat_indirect_pf[NUM_CPUS];
static uint64_t stat_patterns_detected[NUM_CPUS];
static uint64_t stat_ipd_failures[NUM_CPUS];

// ====================================================================
// PT Lookup Helpers
// ====================================================================

// Find PT entry by PC; returns index or -1
static int pt_find_by_pc(uint32_t cpu, uint64_t pc) {
    for (int i = 0; i < IMP_PT_SIZE; i++) {
        if (imp_pt[cpu][i].stream_valid && imp_pt[cpu][i].pc == pc)
            return i;
    }
    return -1;
}

// Find LRU PT entry for replacement
static int pt_find_lru(uint32_t cpu) {
    // Prefer invalid entries
    for (int i = 0; i < IMP_PT_SIZE; i++) {
        if (!imp_pt[cpu][i].stream_valid)
            return i;
    }
    // Prefer entries without active indirect prefetching (LRU among those)
    int lru_idx = -1;
    uint64_t min_cycle = UINT64_MAX;
    for (int i = 0; i < IMP_PT_SIZE; i++) {
        if (!imp_pt[cpu][i].prefetch_active && imp_pt[cpu][i].last_used_cycle < min_cycle) {
            min_cycle = imp_pt[cpu][i].last_used_cycle;
            lru_idx = i;
        }
    }
    if (lru_idx >= 0) return lru_idx;
    // All entries have active prefetching; pick global LRU
    for (int i = 0; i < IMP_PT_SIZE; i++) {
        if (imp_pt[cpu][i].last_used_cycle < min_cycle) {
            min_cycle = imp_pt[cpu][i].last_used_cycle;
            lru_idx = i;
        }
    }
    return (lru_idx >= 0) ? lru_idx : 0;
}

// ====================================================================
// IPD Helpers
// ====================================================================

// Find IPD entry tracking a given PT index
static int ipd_find_for_pt(uint32_t cpu, uint32_t pt_idx) {
    for (int i = 0; i < IMP_IPD_SIZE; i++) {
        if (imp_ipd[cpu][i].valid && imp_ipd[cpu][i].pt_idx == pt_idx)
            return i;
    }
    return -1;
}

// Allocate a free IPD entry for a given PT index
static int ipd_alloc(uint32_t cpu, uint32_t pt_idx) {
    for (int i = 0; i < IMP_IPD_SIZE; i++) {
        if (!imp_ipd[cpu][i].valid) {
            imp_ipd[cpu][i].reset();
            imp_ipd[cpu][i].valid = true;
            imp_ipd[cpu][i].pt_idx = pt_idx;
            return i;
        }
    }
    return -1; // Full
}

// ====================================================================
// IPD: Process a cache miss
// For each active IPD entry, pair the miss with stored index values
// and attempt to detect a (shift, BaseAddr) pattern.
// ====================================================================
static void ipd_on_miss(uint32_t cpu, uint64_t miss_addr) {
    for (int i = 0; i < IMP_IPD_SIZE; i++) {
        IPD_Entry &ipd = imp_ipd[cpu][i];
        if (!ipd.valid) continue;

        if (!ipd.has_idx2) {
            // === Phase 1: Collecting misses after idx1 ===
            if (ipd.miss_cnt_1 < IMP_BA_ARRAY_LEN) {
                for (int s = 0; s < IMP_NUM_SHIFTS; s++) {
                    ipd.ba_idx1[s][ipd.miss_cnt_1] =
                        compute_base_addr(miss_addr, ipd.idx1, SHIFT_VALUES[s]);
                }
                ipd.miss_cnt_1++;
            }
        } else {
            // === Phase 2: Collecting misses after idx2, checking matches ===
            if (ipd.miss_cnt_2 < IMP_BA_ARRAY_LEN) {
                for (int s = 0; s < IMP_NUM_SHIFTS; s++) {
                    uint64_t ba2 = compute_base_addr(miss_addr, ipd.idx2, SHIFT_VALUES[s]);

                    // Check if this BaseAddr matches any from the idx1 phase
                    for (uint8_t m = 0; m < ipd.miss_cnt_1; m++) {
                        if (ba2 == ipd.ba_idx1[s][m]) {
                            // === PATTERN DETECTED ===
                            PT_Entry &pt = imp_pt[cpu][ipd.pt_idx];
                            pt.indirect_enable = true;
                            pt.shift_idx       = s;
                            pt.base_addr       = ba2;
                            pt.indirect_conf   = 0;
                            pt.index_valid     = false;
                            pt.prefetch_active = false;
                            pt.pref_distance   = 1;

                            stat_patterns_detected[cpu]++;

                            // Clear backoff for this PT entry on success
                            pt_backoff_count[cpu][ipd.pt_idx] = 0;
                            pt_backoff_until[cpu][ipd.pt_idx] = 0;

                            // Release IPD entry
                            ipd.valid = false;
                            return;
                        }
                    }
                }
                ipd.miss_cnt_2++;
            }
        }
    }
}

// ====================================================================
// IPD: Handle a new stream access (called when a confirmed stream
// element is accessed). Manages the idx1 -> idx2 -> release lifecycle.
// ====================================================================
static void ipd_on_stream_access(uint32_t cpu, uint32_t pt_idx,
                                 uint64_t index_value, uint64_t cycle) {
    PT_Entry &pt = imp_pt[cpu][pt_idx];

    // Don't feed IPD if indirect pattern already detected
    if (pt.indirect_enable) return;

    // Respect exponential backoff
    if (cycle < pt_backoff_until[cpu][pt_idx]) return;

    int ipd_idx = ipd_find_for_pt(cpu, pt_idx);

    if (ipd_idx == -1) {
        // No existing IPD entry; allocate one and store idx1
        ipd_idx = ipd_alloc(cpu, pt_idx);
        if (ipd_idx >= 0) {
            imp_ipd[cpu][ipd_idx].idx1       = index_value;
            imp_ipd[cpu][ipd_idx].has_idx2   = false;
            imp_ipd[cpu][ipd_idx].miss_cnt_1 = 0;
            imp_ipd[cpu][ipd_idx].miss_cnt_2 = 0;
        }
    } else {
        IPD_Entry &ipd = imp_ipd[cpu][ipd_idx];

        if (!ipd.has_idx2) {
            // This is idx2 (second stream element)
            ipd.idx2     = index_value;
            ipd.has_idx2 = true;
            ipd.miss_cnt_2 = 0;
        } else {
            // Third stream element arrived without detection => give up
            stat_ipd_failures[cpu]++;
            ipd.valid = false;

            // Exponential backoff: wait 2^backoff_count stream accesses
            uint32_t &bc = pt_backoff_count[cpu][pt_idx];
            uint32_t wait = (1u << bc);
            if (bc < 10) bc++;
            pt_backoff_until[cpu][pt_idx] = cycle + wait * 100;
        }
    }
}

// ====================================================================
// Prefetcher Interface Implementation
// ====================================================================

void CACHE::prefetcher_initialize() {
    std::cout << NAME << " IMP (Indirect Memory Prefetcher)"
              << " cpu: " << cpu << std::endl;
    std::cout << "  PT size:          " << IMP_PT_SIZE << std::endl;
    std::cout << "  IPD size:         " << IMP_IPD_SIZE << std::endl;
    std::cout << "  Shift values:     {2, 3, 4, -3}" << std::endl;
    std::cout << "  Max pref distance:" << IMP_MAX_PREF_DIST << std::endl;

    for (int i = 0; i < IMP_PT_SIZE; i++) {
        imp_pt[cpu][i].reset();
        pt_backoff_count[cpu][i] = 0;
        pt_backoff_until[cpu][i] = 0;
    }
    for (int i = 0; i < IMP_IPD_SIZE; i++)
        imp_ipd[cpu][i].reset();

    stat_stream_pf[cpu]        = 0;
    stat_indirect_pf[cpu]      = 0;
    stat_patterns_detected[cpu]= 0;
    stat_ipd_failures[cpu]     = 0;
}

uint64_t CACHE::prefetcher_cache_operate(uint64_t addr, uint64_t ip,
                                         uint8_t cache_hit, uint8_t type,
                                         uint64_t metadata_in)
{
    // =================================================================
    // Step 1: Indirect pattern verification
    // For PT entries in the confidence-building phase, check if this
    // access matches the expected indirect address for the stored index.
    // =================================================================
    for (int i = 0; i < IMP_PT_SIZE; i++) {
        PT_Entry &pt = imp_pt[cpu][i];
        if (!pt.stream_valid || !pt.indirect_enable || pt.prefetch_active || !pt.index_valid)
            continue;

        uint64_t expected = compute_indirect_addr(
            pt.index_value, SHIFT_VALUES[pt.shift_idx], pt.base_addr);

        // Compare at cacheline granularity
        if ((addr >> OFFSET_BITS) == (expected >> OFFSET_BITS)) {
            pt.indirect_conf = std::min((int)pt.indirect_conf + 1, IMP_INDIRECT_CONF_MAX);
            pt.index_valid = false; // Consumed this verification

            if (pt.indirect_conf >= IMP_INDIRECT_CONF_THRESH)
                pt.prefetch_active = true;
        }
    }

    // =================================================================
    // Step 2: Stream access handling
    // Look up the PT by PC to detect/update streaming patterns.
    // =================================================================
    int pt_idx = pt_find_by_pc(cpu, ip);

    if (pt_idx >= 0) {
        // === PC matches an existing PT entry ===
        PT_Entry &pt = imp_pt[cpu][pt_idx];
        int64_t new_stride = (int64_t)addr - (int64_t)pt.addr;

        if (new_stride != 0) {
            bool stride_match = (new_stride == pt.stride) && (pt.stream_conf >= 1);

            if (stride_match) {
                // Stream continues with same stride
                pt.stream_conf = std::min((int)pt.stream_conf + 1, IMP_STREAM_CONF_MAX);
            } else if (pt.stream_conf <= 1) {
                // Still learning the stride
                pt.stride = new_stride;
                pt.stream_conf = 1;
            } else if (pt.indirect_enable) {
                // Stream broke but indirect pattern exists (nested loop case,
                // Section 3.3.1): keep the pattern, just reset stream position.
                pt.stride = new_stride;
                pt.stream_conf = 1;
                pt.pref_distance = 1; // Reset prefetch distance
            } else {
                // Stride mismatch with high confidence: decrement
                pt.stream_conf = (pt.stream_conf > 1) ? pt.stream_conf - 1 : 0;
                pt.stride = new_stride;
            }

            pt.addr = addr;
            pt.last_used_cycle = current_cycle;

            // --- Stream prefetching (bring index array ahead into cache) ---
            if (pt.stream_conf >= IMP_STREAM_PF_THRESH) {
                for (int d = 1; d <= IMP_STREAM_PF_DIST; d++) {
                    uint64_t stream_pf_addr = addr + pt.stride * d;
                    if (is_valid_pf_addr(stream_pf_addr)) {
                        int succ = prefetch_line(stream_pf_addr, true, 0);
                        if (succ) stat_stream_pf[cpu]++;
                    }
                }
            }

            // --- IPD feeding / Indirect prefetching ---
            if (pt.stream_conf >= IMP_STREAM_CONF_THRESH) {
                // Read index value B[i] from memory
                uint64_t idx_val = read_index_value(cpu, addr, pt.stride);

                if (pt.indirect_enable && pt.prefetch_active) {
                    // === INDIRECT PREFETCHING ===
                    // Read B[i + delta] for each delta up to pref_distance
                    // and issue prefetches for A[B[i + delta]]
                    for (uint8_t d = 1; d <= pt.pref_distance; d++) {
                        uint64_t future_idx_addr = addr + (int64_t)pt.stride * d;
                        uint64_t future_idx = read_index_value(cpu, future_idx_addr, pt.stride);
                        uint64_t pf_addr = compute_indirect_addr(
                            future_idx, SHIFT_VALUES[pt.shift_idx], pt.base_addr);

                        if (is_valid_pf_addr(pf_addr)) {
                            int succ = prefetch_line(pf_addr, true, 0);
                            if (succ) stat_indirect_pf[cpu]++;
                        }
                    }

                    // Linearly increase prefetch distance (Section 3.2.3)
                    if (pt.pref_distance < IMP_MAX_PREF_DIST)
                        pt.pref_distance++;

                } else if (pt.indirect_enable && !pt.prefetch_active) {
                    // === CONFIDENCE BUILDING PHASE ===
                    // Store index value for verification against subsequent accesses
                    if (pt.index_valid) {
                        // Previous index wasn't verified (overwritten) => decrement
                        if (pt.indirect_conf > 0)
                            pt.indirect_conf--;
                    }
                    pt.index_value = idx_val;
                    pt.index_valid = true;

                } else {
                    // === NO INDIRECT PATTERN YET: Feed IPD ===
                    ipd_on_stream_access(cpu, pt_idx, idx_val, current_cycle);
                }
            }
        } else {
            // Same address accessed again (stride == 0): just update LRU
            pt.last_used_cycle = current_cycle;
        }
    } else {
        // === No PT entry for this PC: allocate new one ===
        int new_idx = pt_find_lru(cpu);

        // Invalidate any IPD entries pointing to the replaced entry
        for (int i = 0; i < IMP_IPD_SIZE; i++) {
            if (imp_ipd[cpu][i].valid && imp_ipd[cpu][i].pt_idx == (uint32_t)new_idx)
                imp_ipd[cpu][i].valid = false;
        }

        imp_pt[cpu][new_idx].reset();
        imp_pt[cpu][new_idx].stream_valid    = true;
        imp_pt[cpu][new_idx].pc              = ip;
        imp_pt[cpu][new_idx].addr            = addr;
        imp_pt[cpu][new_idx].stride          = 0;
        imp_pt[cpu][new_idx].stream_conf     = 0;
        imp_pt[cpu][new_idx].last_used_cycle = current_cycle;

        pt_backoff_count[cpu][new_idx] = 0;
        pt_backoff_until[cpu][new_idx] = 0;
    }

    // =================================================================
    // Step 3: Feed cache misses to the IPD
    // =================================================================
    if (!cache_hit) {
        ipd_on_miss(cpu, addr);
    }

    return metadata_in;
}

uint64_t CACHE::prefetcher_cache_fill(uint64_t addr, uint32_t set, uint32_t way,
                                      uint8_t prefetch, uint64_t evicted_addr,
                                      uint64_t metadata_in, int64_t ret_val)
{
    // IMP does not use cache fill events; all logic is in cache_operate.
    // In a more advanced implementation, ret_val (the loaded data value)
    // could be used to read index values on fill instead of from mem_data.
    return metadata_in;
}

void CACHE::prefetcher_cycle_operate() {
    // IMP's main logic is event-driven in cache_operate.
    // No per-cycle work needed in this implementation.
}

void CACHE::prefetcher_final_stats() {
    std::cout << std::endl;
    std::cout << "=== IMP Final Stats (CPU " << cpu << ") ===" << std::endl;
    std::cout << "  Indirect patterns detected:  " << stat_patterns_detected[cpu] << std::endl;
    std::cout << "  Stream prefetches issued:    " << stat_stream_pf[cpu] << std::endl;
    std::cout << "  Indirect prefetches issued:  " << stat_indirect_pf[cpu] << std::endl;
    std::cout << "  IPD detection failures:      " << stat_ipd_failures[cpu] << std::endl;
    std::cout << "===============================" << std::endl;
}
