// Copyright (c) 2015, The Regents of the University of California (Regents)
// See LICENSE.txt for license details

#include <cassert>
#include <cstdint>
#include <functional>
#include <iostream>
#include <vector>

#include "benchmark.h"
#include "bitmap.h"
#include "builder.h"
#include "command_line.h"
#include "graph.h"
#include "platform_atomics.h"
#include "pvector.h"
#include "sliding_queue.h"
#include "timer.h"
#include "util.h"

#include <omp.h>
#include <pf_interface.h>
#include <sim_api.h>

/*
GAP Benchmark Suite
Kernel: Betweenness Centrality (BC)
Author: Scott Beamer

Will return array of approx betweenness centrality scores for each vertex

This BC implementation makes use of the Brandes [1] algorithm with
implementation optimizations from Madduri et al. [2]. It is only an approximate
because it does not compute the paths from every start vertex, but only a small
subset of them. Additionally, the scores are normalized to the range [0,1].

As an optimization to save memory, this implementation uses a Bitmap to hold
succ (list of successors) found during the BFS phase that are used in the back-
propagation phase.

[1] Ulrik Brandes. "A faster algorithm for betweenness centrality." Journal of
    Mathematical Sociology, 25(2):163–177, 2001.

[2] Kamesh Madduri, David Ediger, Karl Jiang, David A Bader, and Daniel
    Chavarria-Miranda. "A faster parallel algorithm and efficient multithreaded
    implementations for evaluating betweenness centrality on massive datasets."
    International Symposium on Parallel & Distributed Processing (IPDPS), 2009.
*/


using namespace std;
typedef float ScoreT;

void PBFS(const Graph &g, NodeID source, pvector<NodeID> &path_counts,
    Bitmap &succ, vector<SlidingQueue<NodeID>::iterator> &depth_index,
    SlidingQueue<NodeID> &queue, pvector<NodeID>& depths) {
  assert( (uint64_t) depths.size() == (uint64_t) g.num_nodes() );
  depths.fill(-1);
  depths[source] = 0;
  path_counts[source] = 1;
  queue.push_back(source);
  depth_index.push_back(queue.begin());
  queue.slide_window();
  const NodeID* g_out_start = g.out_neigh(0).begin();
  #pragma omp parallel
  {
    NodeID depth = 0;
    QueueBuffer<NodeID> lqueue(queue);
    while (!queue.empty()) {
      #pragma omp single
      depth_index.push_back(queue.begin());
      depth++;
      #pragma omp for schedule(dynamic, 64)
      for (auto q_iter = queue.begin(); q_iter < queue.end(); q_iter++) {
        NodeID u = *q_iter;
        for (NodeID &v : g.out_neigh(u)) {
          if ((depths[v] == -1) &&
              (compare_and_swap(depths[v], static_cast<NodeID>(-1), depth))) {
            lqueue.push_back(v);
          }
          if (depths[v] == depth) {
            succ.set_bit_atomic(&v - g_out_start);
            fetch_and_add(path_counts[v], path_counts[u]);
          }
        }
      }
      lqueue.flush();
      #pragma omp barrier
      #pragma omp single
      queue.slide_window();
    }
  }
  depth_index.push_back(queue.begin());
}


pvector<ScoreT> Brandes(const Graph &g, SourcePicker<Graph> &sp,
                        NodeID num_iters) {
  Timer t;
  t.Start();
  pvector<ScoreT> scores(g.num_nodes(), 0);
  pvector<NodeID> path_counts(g.num_nodes());
  pvector<NodeID> depths(g.num_nodes());
  pvector<ScoreT> deltas(g.num_nodes(), 0);
  Bitmap succ(g.num_edges_directed());
  vector<SlidingQueue<NodeID>::iterator> depth_index;
  SlidingQueue<NodeID> queue(g.num_nodes());
  t.Stop();
  PrintStep("a", t.Seconds());
  const NodeID* g_out_start = g.out_neigh(0).begin();

// ============================================================================
  const int numNodes_pf = 5 + 6;
  const int numEdges_pf = 5 + 7;
  const int numTriggers_pf = 1 + 1;
  const uint64_t num_cores = omp_get_max_threads();
  pf_params_t* params = new pf_params_t(numNodes_pf, numEdges_pf,
    numTriggers_pf, num_cores);
  pf_enable_t* enable = new pf_enable_t();

  // Phase 1
  params->RegisterNode(&(*queue.begin()), g.num_nodes(), (NodeId)0);
  params->RegisterNode(g.out_index_, g.num_nodes() + 1, (NodeId)1);
  params->RegisterNode(g.out_neighbors_, g.num_edges(), (NodeId)2);
  params->RegisterNode(&(*depths.begin()), g.num_nodes(), (NodeId)3);
  params->RegisterNode(&(*path_counts.begin()), g.num_nodes(), (NodeId)4);
  params->RegisterTravEdge((NodeId)0, (NodeId)1, BaseOffset_int32_t);
  params->RegisterTravEdge((NodeId)0, (NodeId)4, BaseOffset_int32_t);
  params->RegisterTravEdge((NodeId)1, (NodeId)2, PointerBounds_int32_t);
  params->RegisterTravEdge((NodeId)2, (NodeId)3, BaseOffset_int32_t);
  params->RegisterTravEdge((NodeId)2, (NodeId)4, BaseOffset_int32_t);
  auto p1 = params->RegisterTrigEdge((NodeId)0, (NodeId)0, UpToOffset, SquashIfLarger);

  // Phase 2
  params->RegisterNode(&(*queue.begin()), g.num_nodes(), (NodeId) 5);
  params->RegisterNode(g.out_index_, g.num_nodes() + 1, (NodeId) 6);
  params->RegisterNode(g.out_neighbors_, g.num_edges(), (NodeId) 7);
  params->RegisterNode(&(*scores.begin()), g.num_nodes(), (NodeId) 8);
  params->RegisterNode(&(*path_counts.begin()), g.num_nodes(), (NodeId) 9);
  params->RegisterNode(&(*deltas.begin()), g.num_nodes(), (NodeId) 10);
  auto p2 = params->RegisterTrigEdge((NodeId) 5, (NodeId) 5, UpToOffset, SquashIfLarger);
  params->RegisterTravEdge((NodeId)5, (NodeId)10, BaseOffset_int32_t);
  params->RegisterTravEdge((NodeId)5, (NodeId)9, BaseOffset_int32_t);
  params->RegisterTravEdge((NodeId)5, (NodeId)8, BaseOffset_int32_t);
  params->RegisterTravEdge((NodeId)5, (NodeId)6, BaseOffset_int32_t);
  params->RegisterTravEdge((NodeId)6, (NodeId)7, PointerBounds_int32_t);
  params->RegisterTravEdge((NodeId)7, (NodeId)10, BaseOffset_int32_t);
  params->RegisterTravEdge((NodeId)7, (NodeId)9, BaseOffset_int32_t);

  printf("THIS STUFF IS JANKY\n");
  SimUser(PF_SET_PARAM, (long unsigned int) params);
  printf("THIS STUFF IS JANKY pt2\n");
  SimUser(PF_SET_ENABLE, (long unsigned int) enable);
  printf("THIS STUFF IS JANKY pt3\n");

  SimRoiStart();
  SimUser(PF_ENABLE, 0);

  if (SimInSimulator() and !enable->is_enabled()) {
      enable->wait();
  }

  // ======================================================
  // Kernel
  for (NodeID iter=0; iter < num_iters; iter++) {
    NodeID source = sp.PickNext();
    cout << "source: " << source << endl;
    t.Start();
    path_counts.fill(0);
    depth_index.resize(0);
    queue.reset();
    succ.reset();

    // ============
    // Phase 1
    TriggerChange::EnableTrigger(num_cores, p1);
    TriggerChange::DisableTrigger(num_cores, p2);

    PBFS(g, source, path_counts, succ, depth_index, queue, depths);

    // ============
    // Phase 2
    TriggerChange::EnableTrigger(num_cores, p2);
    TriggerChange::DisableTrigger(num_cores, p1);

    t.Stop();
    PrintStep("b", t.Seconds());
    deltas.fill(0);
    t.Start();
    for (int d=depth_index.size()-2; d >= 0; d--) {
      #pragma omp parallel for schedule(dynamic, 64)
      for (auto it = depth_index[d]; it < depth_index[d+1]; it++) {
        NodeID u = *it;
        ScoreT delta_u = 0;
        for (NodeID &v : g.out_neigh(u)) {
          if (succ.get_bit(&v - g_out_start)) {
            delta_u += static_cast<ScoreT>(path_counts[u]) /
                       static_cast<ScoreT>(path_counts[v]) * (1 + deltas[v]);
          }
        }
        deltas[u] = delta_u;
        scores[u] += delta_u;
      }
    }
    t.Stop();
    PrintStep("p", t.Seconds());
  }
  // normalize scores
  ScoreT biggest_score = 0;
  #pragma omp parallel for reduction(max : biggest_score)
  for (NodeID n=0; n < g.num_nodes(); n++)
    biggest_score = max(biggest_score, scores[n]);
  #pragma omp parallel for
  for (NodeID n=0; n < g.num_nodes(); n++)
    scores[n] = scores[n] / biggest_score;
  // ======================================================

  SimRoiEnd();
  SimUser(PF_DISABLE, 0);
// ============================================================================

  delete params;
  delete enable;
  return scores;
}


void PrintTopScores(const Graph &g, const pvector<ScoreT> &scores) {
  vector<pair<NodeID, ScoreT>> score_pairs(g.num_nodes());
  for (NodeID n : g.vertices())
    score_pairs[n] = make_pair(n, scores[n]);
  int k = 5;
  vector<pair<ScoreT, NodeID>> top_k = TopK(score_pairs, k);
  for (auto kvp : top_k)
    cout << kvp.second << ":" << kvp.first << endl;
}


// Still uses Brandes algorithm, but has the following differences:
// - serial (no need for atomics or dynamic scheduling)
// - uses vector for BFS queue
// - regenerates farthest to closest traversal order from depths
// - regenerates successors from depths
bool BCVerifier(const Graph &g, SourcePicker<Graph> &sp, NodeID num_iters,
                const pvector<ScoreT> &scores_to_test) {
  pvector<ScoreT> scores(g.num_nodes(), 0);
  for (int iter=0; iter < num_iters; iter++) {
    NodeID source = sp.PickNext();
    // BFS phase, only records depth & path_counts
    pvector<int> depths(g.num_nodes(), -1);
    depths[source] = 0;
    vector<NodeID> path_counts(g.num_nodes(), 0);
    path_counts[source] = 1;
    vector<NodeID> to_visit;
    to_visit.reserve(g.num_nodes());
    to_visit.push_back(source);
    for (auto it = to_visit.begin(); it != to_visit.end(); it++) {
      NodeID u = *it;
      for (NodeID v : g.out_neigh(u)) {
        if (depths[v] == -1) {
          depths[v] = depths[u] + 1;
          to_visit.push_back(v);
        }
        if (depths[v] == depths[u] + 1)
          path_counts[v] += path_counts[u];
      }
    }
    // Get lists of vertices at each depth
    vector<vector<NodeID>> verts_at_depth;
    for (NodeID n : g.vertices()) {
      if (depths[n] != -1) {
        if (depths[n] >= static_cast<int>(verts_at_depth.size()))
          verts_at_depth.resize(depths[n] + 1);
        verts_at_depth[depths[n]].push_back(n);
      }
    }
    // Going from farthest to clostest, compute "depencies" (deltas)
    pvector<ScoreT> deltas(g.num_nodes(), 0);
    for (int depth=verts_at_depth.size()-1; depth >= 0; depth--) {
      for (NodeID u : verts_at_depth[depth]) {
        for (NodeID v : g.out_neigh(u)) {
          if (depths[v] == depths[u] + 1) {
            deltas[u] += static_cast<ScoreT>(path_counts[u]) /
                         static_cast<ScoreT>(path_counts[v]) * (1 + deltas[v]);
          }
        }
        scores[u] += deltas[u];
      }
    }
  }
  // Normalize scores
  ScoreT biggest_score = *max_element(scores.begin(), scores.end());
  for (NodeID n : g.vertices())
    scores[n] = scores[n] / biggest_score;
  // Compare scores
  bool all_ok = true;
  for (NodeID n : g.vertices()) {
    if (scores[n] != scores_to_test[n]) {
      cout << n << ": " << scores[n] << " != " << scores_to_test[n] << endl;
      all_ok = false;
    }
  }
  return all_ok;
}


int main(int argc, char* argv[]) {
  omp_set_num_threads(4);
  std::cout << "#threads = " << omp_get_max_threads() << std::endl;
  CLIterApp cli(argc, argv, "betweenness-centrality", 1);
  if (!cli.ParseArgs())
    return -1;
  if (cli.num_iters() > 1 && cli.start_vertex() != -1)
    cout << "Warning: iterating from same source (-r & -i)" << endl;
  Builder b(cli);
  Graph g = b.MakeGraph();
  SourcePicker<Graph> sp(g, cli.start_vertex());
  auto BCBound =
    [&sp, &cli] (const Graph &g) { return Brandes(g, sp, cli.num_iters()); };
  SourcePicker<Graph> vsp(g, cli.start_vertex());
  auto VerifierBound = [&vsp, &cli] (const Graph &g,
                                     const pvector<ScoreT> &scores) {
    return BCVerifier(g, vsp, cli.num_iters(), scores);
  };
  BenchmarkKernel(cli, g, BCBound, PrintTopScores, VerifierBound);
  return 0;
}
