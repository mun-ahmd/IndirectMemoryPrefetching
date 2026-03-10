/*
 * QEMU Runtime: Sniper-compatible API
 *
 * Drop-in replacement for sniper6.1/include/sim_api.h and pf_interface.h.
 * Mimics the Sniper simulator APIs but delegates to the QEMU syscall-based
 * runtime (prefetcher_qemu_rt.cpp) instead of using magic instructions.
 *
 * Use this header when building applications for QEMU tracing instead of
 * Sniper cycle-accurate simulation.
 */

#ifndef QEMU_PF_SIM_API_H
#define QEMU_PF_SIM_API_H

#include <atomic>
#include <cassert>
#include <condition_variable>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <mutex>
#include <thread>

/* ------------------------------------------------------------------ */
/*  QEMU runtime function declarations (from prefetcher_qemu_rt.cpp)   */
/* ------------------------------------------------------------------ */

extern "C" {

int create_params(int num_nodes_pf, int num_edges_pf, int num_triggers_pf);
int create_enable();
int delete_params();
int delete_enable();
int register_node_with_size(uintptr_t base, int64_t size, int64_t elem_size,
                            int64_t node_id);
int register_trav_edge1(uintptr_t baseaddr_from, uintptr_t baseaddr_to,
                        uint32_t func_id, int edge_id);
int register_trav_edge2(int64_t id_from, int64_t id_to, uint32_t func_id);
int register_trig_edge1(uintptr_t baseaddr_from, uintptr_t baseaddr_to,
                       uint32_t func_id, uint32_t squash_func_id);
int register_trig_edge2(int64_t id_from, int64_t id_to,
                       uint32_t func_id, uint32_t squash_func_id);
int sim_user_pf_set_param();
int sim_user_pf_set_enable();
int sim_user_pf_enable();
int sim_user_pf_disable();
int sim_user_wait();
int sim_roi_start();
int sim_roi_end();
int sim_user_pf_enable_trig(uint64_t core_id, uint64_t trigger_id);
int sim_user_pf_disable_trig(uint64_t core_id, uint64_t trigger_id);

} /* extern "C" */

/* ------------------------------------------------------------------ */
/*  sim_api.h equivalents - replace magic instructions with runtime   */
/* ------------------------------------------------------------------ */

#define SIM_CMD_ROI_TOGGLE      0
#define SIM_CMD_ROI_START       1
#define SIM_CMD_ROI_END         2
#define SIM_CMD_MHZ_SET         3
#define SIM_CMD_MARKER          4
#define SIM_CMD_USER            5
#define SIM_CMD_INSTRUMENT_MODE 6
#define SIM_CMD_MHZ_GET         7
#define SIM_CMD_IN_SIMULATOR    8
#define SIM_CMD_PROC_ID         9
#define SIM_CMD_THREAD_ID       10
#define SIM_CMD_NUM_PROCS       11
#define SIM_CMD_NUM_THREADS     12
#define SIM_CMD_NAMED_MARKER    13
#define SIM_CMD_SET_THREAD_NAME 14

#define SIM_OPT_INSTRUMENT_DETAILED    0
#define SIM_OPT_INSTRUMENT_WARMUP      1
#define SIM_OPT_INSTRUMENT_FASTFORWARD 2

/* QEMU: no magic instructions; call runtime directly */
static inline unsigned long SimMagic0(unsigned long cmd) {
  (void)cmd;
  return 0;
}
static inline unsigned long SimMagic1(unsigned long cmd, unsigned long arg0) {
  (void)cmd;
  (void)arg0;
  return 0;
}
static inline unsigned long SimMagic2(unsigned long cmd, unsigned long arg0,
                                      unsigned long arg1) {
  (void)cmd;
  (void)arg0;
  (void)arg1;
  return 0;
}

#define SimRoiStart()             ((void)sim_roi_start())
#define SimRoiEnd()                ((void)sim_roi_end())
#define SimGetProcId()             0
#define SimGetThreadId()            0
#define SimSetThreadName(name)     ((void)(name))
#define SimGetNumProcs()           1
#define SimGetNumThreads()         1
#define SimSetFreqMHz(proc, mhz)   ((void)(proc), (void)(mhz))
#define SimSetOwnFreqMHz(mhz)      ((void)(mhz))
#define SimGetFreqMHz(proc)        ((void)(proc), 0UL)
#define SimGetOwnFreqMHz()         0UL
#define SimMarker(arg0, arg1)      ((void)(arg0), (void)(arg1))
#define SimNamedMarker(arg0, str)  ((void)(arg0), (void)(str))
#define SimSetInstrumentMode(opt)  ((void)(opt))

/* PF_* command constants (Sniper sim_api.h / pf_interface.h compatibility) */
#define PF_SET_PARAM     8
#define PF_SET_ENABLE    9
#define PF_ENABLE       10
#define PF_DISABLE      11
#define PF_ENABLE_TRIG  15
#define PF_DISABLE_TRIG 16

/* Small POD struct matching the PF_* trigger argument layout */
struct PfTriggerArgs {
  uint64_t core_id;
  uint64_t trigger_id;
};

/* SimUser: route PF_* commands to QEMU runtime */
static inline unsigned long SimUser(unsigned long cmd, unsigned long arg) {
  switch (cmd) {
  case PF_SET_ENABLE:
    (void)sim_user_pf_set_enable();
    return 0;
  case PF_SET_PARAM:
    (void)sim_user_pf_set_param();
    return 0;
  case PF_ENABLE:
    (void)sim_user_pf_enable();
    return 0;
  case PF_DISABLE:
    (void)sim_user_pf_disable();
    return 0;
  case PF_ENABLE_TRIG:
    if (arg != 0) {
      const PfTriggerArgs *p =
          reinterpret_cast<const PfTriggerArgs *>(arg);
      (void)sim_user_pf_enable_trig(p->core_id, p->trigger_id);
    }
    return 0;
  case PF_DISABLE_TRIG:
    if (arg != 0) {
      const PfTriggerArgs *p =
          reinterpret_cast<const PfTriggerArgs *>(arg);
      (void)sim_user_pf_disable_trig(p->core_id, p->trigger_id);
    }
    return 0;
  default:
    return 0;
  }
}

/* QEMU: not in Sniper; return false so wait() never blocks */
#define SimInSimulator() 0

/* ------------------------------------------------------------------ */
/*  pf_interface.h equivalents                                        */
/* ------------------------------------------------------------------ */

typedef int64_t NodeId;

enum FuncId {
  TraversalHolder,
  BaseOffset_int32_t,
  BaseOffset_int64_t,
  PointerBounds_int32_t,
  PointerBounds_uint64_t,
  TriggerHolder,
  UpToOffset,
  StaticUpToOffset_8_16,
  StaticOffset_2,
  StaticOffset_4,
  StaticOffset_8,
  StaticOffset_16,
  StaticOffset_32,
  StaticOffset_64,
  StaticOffset_256,
  StaticOffset_512,
  StaticOffset_1024,
  StaticOffset_2_reverse,
  StaticOffset_4_reverse,
  StaticOffset_8_reverse,
  StaticOffset_16_reverse,
  SquashIfLarger,
  SquashIfSmaller,
  NeverSquash,
  InvalidFuncId
};

namespace std {
template <> struct hash<FuncId> {
  typedef FuncId argument_type;
  typedef size_t result_type;
  result_type operator()(const argument_type &x) const {
    using type = typename std::underlying_type<argument_type>::type;
    return std::hash<type>()(static_cast<type>(x));
  }
};
}

class pf_enable_t {
public:
  pf_enable_t()
      : enable(false)
#ifdef BLOCKING
        ,
        m(), cv()
#endif
  {
  }

  void init() { this->enable = false; }

  void wait() {
#ifdef BLOCKING
    std::unique_lock<std::mutex> lk(m);
    this->cv.wait(lk, [&]() { return enable; });
#else
    /* QEMU: call runtime wait (may be no-op when not in Sniper) */
    (void)sim_user_wait();
    while (!this->enable) {
      std::this_thread::yield();
    }
#endif
  }

  void signal() {
#ifdef BLOCKING
    std::lock_guard<std::mutex> lk(m);
    this->enable = true;
    cv.notify_all();
#else
    this->enable = true;
#endif
  }

  void disable() {
#ifdef BLOCKING
    std::lock_guard<std::mutex> g(m);
#endif
    this->enable = false;
  }

  inline bool is_enabled(void) const { return this->enable; }
  operator bool() const { return this->enable; }

private:
  std::atomic<bool> enable;
#ifdef BLOCKING
  std::mutex m;
  std::condition_variable cv;
#endif
};

class pf_params_t {
public:
  class node_int {
  public:
    node_int(uintptr_t _base, int64_t _size, int64_t _type_size, NodeId _id)
        : base(_base), size(_size), type_size(_type_size), id(_id) {}
    uintptr_t base;
    int64_t size;
    int64_t type_size;
    NodeId id;
  };

  class edge_int {
  public:
    edge_int(NodeId _from, NodeId _to, FuncId _f)
        : from(_from), to(_to), f(_f) {}
    NodeId from;
    NodeId to;
    FuncId f;
    void Print() const { printf("(%ld, %ld, %d)", (long)from, (long)to, f); }
  };

  class trigger_edge_int : public edge_int {
  public:
    trigger_edge_int(NodeId _from, NodeId _to, FuncId _f, FuncId _sq_f,
                     uint64_t _id)
        : edge_int(_from, _to, _f), sq_f(_sq_f), id(_id) {}
    FuncId sq_f;
    uint64_t id;
  };

  pf_params_t(uint64_t _numNodes, uint64_t _numEdges, uint64_t _numTriggers,
              uint64_t _numCores)
      : numNodes(_numNodes), numEdges(_numEdges), numTriggers(_numTriggers),
        core_id(_numCores), itEdge(0), itTrig(0),
        nodes((node_int *)malloc(sizeof(node_int) * numNodes)),
        edges((edge_int *)malloc(sizeof(edge_int) * numEdges)),
        triggers((trigger_edge_int *)malloc(sizeof(trigger_edge_int) *
                                            numTriggers)) {
    for (uint64_t i = 0; i < numNodes; ++i) {
      nodes[i] = node_int(0, 0, 0, -1);
    }
    create_params((int)numNodes, (int)numEdges, (int)numTriggers);
  }

  ~pf_params_t() {
    free(nodes);
    free(edges);
    free(triggers);
    delete_params();
  }

  void RegisterNodeWithSize(uintptr_t base, int64_t size, int64_t elem_size,
                            NodeId id) {
    assert(id >= 0);
    assert(id < (NodeId)numNodes);
    nodes[id] = node_int((uintptr_t)base, size, elem_size, id);
    register_node_with_size(base, size, elem_size, id);
  }

  template <typename T> void RegisterNode(T *base, int64_t size, NodeId id) {
    RegisterNodeWithSize((uintptr_t)base, size, sizeof(T), id);
  }

  void RegisterNodeWithSize(void *base, int64_t size, int64_t elem_size,
                            NodeId id) {
    assert(id >= 0);
    assert(id < (NodeId)numNodes);
    nodes[id] = node_int((uintptr_t)base, size, elem_size, id);
    register_node_with_size((uintptr_t)base, size, elem_size, id);
  }

  uint64_t GetNodeIdFromBaseAddr(uint64_t baseaddr_from) {
    for (uint64_t i = 0; i < numNodes; ++i) {
      if (nodes[i].base == baseaddr_from)
        return i;
    }
    assert(false && "if we miss here, this is bad");
    return 0;
  }

  void RegisterTravEdge(uintptr_t baseaddr_from, uintptr_t baseaddr_to,
                        FuncId f) {
    RegisterTravEdge(GetNodeIdFromBaseAddr(baseaddr_from),
                     GetNodeIdFromBaseAddr(baseaddr_to), f);
  }

  uint64_t RegisterTrigEdge(uintptr_t baseaddr_from, uintptr_t baseaddr_to,
                            FuncId f, FuncId sq_f) {
    NodeId id_from = GetNodeIdFromBaseAddr(baseaddr_from);
    NodeId id_to = GetNodeIdFromBaseAddr(baseaddr_to);
    return RegisterTrigEdge(id_from, id_to, f, sq_f);
  }

  void RegisterTravEdge(NodeId id_from, NodeId id_to, FuncId f) {
    if (itEdge == numEdges) {
      edge_int *new_array = (edge_int *)malloc(sizeof(edge_int) * numEdges * 2);
      numEdges = numEdges * 2;
      for (unsigned int i = 0; i < itEdge; ++i) {
        new_array[i] = edges[i];
      }
      free(edges);
      edges = new_array;
    }
    assert(itEdge < numEdges);
    edges[itEdge++] = edge_int(id_from, id_to, f);
    register_trav_edge2(id_from, id_to, (uint32_t)f);
  }

  uint64_t RegisterTrigEdge(NodeId id_from, NodeId id_to, FuncId f,
                            FuncId sq_f) {
    uint64_t id = (numNodes * id_from) + id_to;
    if (itTrig == numTriggers) {
      trigger_edge_int *new_array = (trigger_edge_int *)malloc(
          sizeof(trigger_edge_int) * numTriggers * 2);
      numTriggers = numTriggers * 2;
      for (unsigned int i = 0; i < itTrig; ++i) {
        new_array[i] = triggers[i];
      }
      free(triggers);
      triggers = new_array;
    }
    assert(itTrig < numTriggers);
    triggers[itTrig++] = trigger_edge_int(id_from, id_to, f, sq_f, id);
    register_trig_edge2(id_from, id_to, (uint32_t)f, (uint32_t)sq_f);
    return id;
  }

  void DeleteTrigEdge(uintptr_t baseaddr_from, uintptr_t baseaddr_to) {
    DeleteTrigEdge_helper(GetNodeIdFromBaseAddr(baseaddr_from),
                          GetNodeIdFromBaseAddr(baseaddr_to));
  }

  void DeleteTrigEdge_helper(NodeId id_from, NodeId id_to) {
    int t = -1;
    for (unsigned int i = 0; i < itTrig; ++i) {
      if (triggers[i].from == id_from && triggers[i].to == id_to) {
        t = i;
        break;
      }
    }
    assert(t != -1);
    for (unsigned int i = t; i < itTrig - 1; ++i) {
      triggers[i] = triggers[i + 1];
    }
    --itTrig;
  }

  void ClearTrigEdges() { itTrig = 0; }

  void DeleteTravEdge(uintptr_t baseaddr_from, uintptr_t baseaddr_to) {
    DeleteTravEdge_helper(GetNodeIdFromBaseAddr(baseaddr_from),
                          GetNodeIdFromBaseAddr(baseaddr_to));
  }

  void DeleteTravEdge_helper(NodeId id_from, NodeId id_to) {
    int t = -1;
    for (uint64_t i = 0; i < itEdge; ++i) {
      if (edges[i].from == id_from && edges[i].to == id_to) {
        t = (int)i;
        break;
      }
    }
    assert(t != -1);
    for (unsigned int i = (unsigned)t; i < itEdge - 1; ++i) {
      edges[i] = edges[i + 1];
    }
    --itEdge;
  }

  void ClearTravEdges() { itEdge = 0; }

  void Print() const {
    printf("Trav:\n");
    for (unsigned i = 0; i < itEdge; ++i) {
      printf("\t[%u] - ", i);
      edges[i].Print();
      printf("\n");
    }
    printf("Trig:\n");
    for (unsigned i = 0; i < itTrig; ++i) {
      printf("\t[%u] - ", i);
      triggers[i].Print();
      printf("\n");
    }
  }

  uint64_t numNodes;
  uint64_t numEdges;
  uint64_t numTriggers;
  uint64_t core_id;
  uint64_t itEdge;
  uint64_t itTrig;
  node_int *nodes;
  edge_int *edges;
  trigger_edge_int *triggers;
};

class TriggerChange {
public:
  TriggerChange(uint64_t _core_id, uint64_t _trigger_id)
      : core_id(_core_id), trigger_id(_trigger_id) {}

  uint64_t core_id;
  uint64_t trigger_id;

  static void DisableTrigger(uint64_t core_id, uint64_t trigger_id) {
    sim_user_pf_disable_trig(core_id, trigger_id);
  }

  static void EnableTrigger(uint64_t core_id, uint64_t trigger_id) {
    sim_user_pf_enable_trig(core_id, trigger_id);
  }
};

#endif /* QEMU_PF_SIM_API_H */
