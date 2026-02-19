#!/usr/bin/env python3
"""
Plot ChampSim results normalized to the no-prefetcher baseline.
Compares Tyche and ipcp_l1d vs no prefetcher (baseline = 1.0 for IPC).
"""

import csv
from pathlib import Path

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np

RESULTS_DIR = Path(__file__).resolve().parent
CSV_PATH = RESULTS_DIR / "results.csv"


def load_results():
    """Load results.csv into a list of dicts."""
    rows = []
    with open(CSV_PATH, newline="") as f:
        for r in csv.DictReader(f):
            row = {k: v for k, v in r.items()}
            row["IPC"] = float(row["IPC"])
            row["L1D_Prefetch_Requests"] = int(row["L1D_Prefetch_Requests"])
            row["L1D_Prefetch_Hits"] = int(row["L1D_Prefetch_Hits"])
            row["L1D_Prefetch_Misses"] = int(row["L1D_Prefetch_Misses"])
            rows.append(row)
    return rows


def main():
    rows = load_results()
    tasks = sorted({r["Task"] for r in rows})
    prefetchers = ["no_prefetcher", "tyche", "ipcp_l1d"]

    # Build baseline (no_prefetcher) IPC per task
    baseline_ipc = {}
    for r in rows:
        if r["Prefetcher"] == "no_prefetcher":
            baseline_ipc[r["Task"]] = r["IPC"]

    print(baseline_ipc)

    # Normalized IPC: prefetcher IPC / no_prefetcher IPC (baseline = 1.0)
    data = {p: [] for p in prefetchers}
    for task in tasks:
        base = baseline_ipc[task]
        for p in prefetchers:
            row = next((r for r in rows if r["Task"] == task and r["Prefetcher"] == p), None)
            if row:
                data[p].append(row["IPC"] / base if base else 0)
            else:
                data[p].append(0)

    x = np.arange(len(tasks))
    width = 0.25

    # Prefetch hit rate = Hits / (Hits + Misses); 0 when no prefetches
    def hit_rate(r):
        h, m = r["L1D_Prefetch_Hits"], r["L1D_Prefetch_Misses"]
        return (h / (h + m) * 100) if (h + m) > 0 else 0.0

    fig, axes = plt.subplots(2, 3, figsize=(12, 8))

    # --- IPC normalized (baseline = 1.0) ---
    ax1 = axes[0, 0]
    bars0 = ax1.bar(x - width, data["no_prefetcher"], width, label="no prefetcher", color="gray")
    bars1 = ax1.bar(x, data["tyche"], width, label="Tyche", color="C0")
    bars2 = ax1.bar(x + width, data["ipcp_l1d"], width, label="ipcp_l1d", color="C1")
    ax1.set_ylabel("IPC (normalized)")
    ax1.set_title("IPC normalized to no prefetcher")
    ax1.set_xticks(x)
    ax1.set_xticklabels(tasks)
    ax1.legend()
    ax1.axhline(y=1.0, color="k", linestyle="--", alpha=0.5)
    ax1.set_ylim(bottom=0)

    # --- L1D Prefetch Requests (absolute; baseline = 0) ---
    ax2 = axes[0, 1]
    req = {p: [next((r["L1D_Prefetch_Requests"] for r in rows if r["Task"] == t and r["Prefetcher"] == p), 0) for t in tasks] for p in prefetchers}
    ax2.bar(x - width, req["no_prefetcher"], width, label="no prefetcher", color="gray")
    ax2.bar(x, req["tyche"], width, label="Tyche", color="C0")
    ax2.bar(x + width, req["ipcp_l1d"], width, label="ipcp_l1d", color="C1")
    ax2.set_ylabel("L1D Prefetch Requests")
    ax2.set_title("L1D Prefetch Requests (absolute)")
    ax2.set_xticks(x)
    ax2.set_xticklabels(tasks)
    ax2.legend()
    ax2.set_ylim(bottom=0)

    # --- L1D Prefetch Hit Rate (%) ---
    ax_hr = axes[0, 2]
    _def = {"L1D_Prefetch_Hits": 0, "L1D_Prefetch_Misses": 0}
    hr = {
        p: [hit_rate(next((r for r in rows if r["Task"] == t and r["Prefetcher"] == p), _def)) for t in tasks]
        for p in prefetchers
    }
    ax_hr.bar(x - width, hr["no_prefetcher"], width, label="no prefetcher", color="gray")
    ax_hr.bar(x, hr["tyche"], width, label="Tyche", color="C0")
    ax_hr.bar(x + width, hr["ipcp_l1d"], width, label="ipcp_l1d", color="C1")
    ax_hr.set_ylabel("Hit rate (%)")
    ax_hr.set_title("L1D Prefetch Hit Rate")
    ax_hr.set_xticks(x)
    ax_hr.set_xticklabels(tasks)
    ax_hr.legend()
    ax_hr.set_ylim(0, 100)

    # --- L1D Prefetch Hits (absolute) ---
    ax3 = axes[1, 0]
    hits = {p: [next((r["L1D_Prefetch_Hits"] for r in rows if r["Task"] == t and r["Prefetcher"] == p), 0) for t in tasks] for p in prefetchers}
    ax3.bar(x - width, hits["no_prefetcher"], width, label="no prefetcher", color="gray")
    ax3.bar(x, hits["tyche"], width, label="Tyche", color="C0")
    ax3.bar(x + width, hits["ipcp_l1d"], width, label="ipcp_l1d", color="C1")
    ax3.set_ylabel("L1D Prefetch Hits")
    ax3.set_title("L1D Prefetch Hits (absolute)")
    ax3.set_xticks(x)
    ax3.set_xticklabels(tasks)
    ax3.legend()
    ax3.set_ylim(bottom=0)

    # --- L1D Prefetch Misses (absolute) ---
    ax4 = axes[1, 1]
    miss = {p: [next((r["L1D_Prefetch_Misses"] for r in rows if r["Task"] == t and r["Prefetcher"] == p), 0) for t in tasks] for p in prefetchers}
    ax4.bar(x - width, miss["no_prefetcher"], width, label="no prefetcher", color="gray")
    ax4.bar(x, miss["tyche"], width, label="Tyche", color="C0")
    ax4.bar(x + width, miss["ipcp_l1d"], width, label="ipcp_l1d", color="C1")
    ax4.set_ylabel("L1D Prefetch Misses")
    ax4.set_title("L1D Prefetch Misses (absolute)")
    ax4.set_xticks(x)
    ax4.set_xticklabels(tasks)
    ax4.legend()
    ax4.set_ylim(bottom=0)

    axes[1, 2].set_visible(False)

    plt.tight_layout()
    out = RESULTS_DIR / "results_normalized.png"
    plt.savefig(out, dpi=150)
    print(f"Saved {out}")


if __name__ == "__main__":
    main()
