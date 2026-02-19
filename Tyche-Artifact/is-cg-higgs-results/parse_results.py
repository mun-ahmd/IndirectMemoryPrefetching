#!/usr/bin/env python3
"""
Parse ChampSim result files and create a CSV with Task, Prefetcher, IPC,
L1D_Prefetch_Requests, L1D_Prefetch_Hits, L1D_Prefetch_Misses.
"""

import re
import csv
from pathlib import Path

RESULTS_DIR = Path(__file__).resolve().parent


def parse_prefetcher_from_filename(name: str) -> str:
    """Map filename prefix to prefetcher label."""
    if name.startswith("results_no"):
        return "no_prefetcher"
    if name.startswith("results_tyche"):
        return "tyche"
    if name.startswith("results_imp"):
        return "imp"
    if name.startswith("results_ipcp_l1d"):
        return "ipcp_l1d"
    return "unknown"


def parse_task_from_filename(name: str) -> str:
    """Extract task name (e.g. dfs_higgs, sssp_higgs) from filename."""
    # results_<pref>_crono_<task>_trace_<n>.champsim.trace.xz.txt
    if "dfs_higgs" in name:
        return "dfs_higgs"
    if "sssp_higgs" in name:
        return "sssp_higgs"
    if "NAS" in name and "cg" in name:
        return "NAS_cg"
    if "NAS" in name and "is" in name:
        return "NAS_is"
    return "unknown"


def parse_result_file(path: Path) -> dict | None:
    """Parse a single ChampSim result file. Returns dict or None on failure."""
    text = path.read_text()
    prefetcher = parse_prefetcher_from_filename(path.name)
    task = parse_task_from_filename(path.name)

    # CPU 0 cumulative IPC: 2.17352
    ipc_match = re.search(r"CPU 0 cumulative IPC:\s*([\d.]+)", text)
    if not ipc_match:
        return None
    ipc = float(ipc_match.group(1))

    # cpu0_L1D PREFETCH  ACCESS: ... HIT: ... MISS: ...
    access_match = re.search(
        r"cpu0_L1D PREFETCH\s+ACCESS:\s*(\d+)\s+HIT:\s*(\d+)\s+MISS:\s*(\d+)", text
    )
    if not access_match:
        return None
    prefetch_hits = int(access_match.group(2))
    prefetch_misses = int(access_match.group(3))

    # cpu0_L1D PREFETCH  REQUESTED: ...
    requested_match = re.search(r"cpu0_L1D PREFETCH\s+REQUESTED:\s*(\d+)", text)
    if not requested_match:
        return None
    prefetch_requests = int(requested_match.group(1))

    return {
        "Task": task,
        "Prefetcher": prefetcher,
        "IPC": ipc,
        "L1D_Prefetch_Requests": prefetch_requests,
        "L1D_Prefetch_Hits": prefetch_hits,
        "L1D_Prefetch_Misses": prefetch_misses,
    }


def main():
    rows = []
    for path in sorted(RESULTS_DIR.glob("results_*.txt")):
        row = parse_result_file(path)
        if row:
            rows.append(row)

    out_path = RESULTS_DIR / "results.csv"
    fieldnames = [
        "Task",
        "Prefetcher",
        "IPC",
        "L1D_Prefetch_Requests",
        "L1D_Prefetch_Hits",
        "L1D_Prefetch_Misses",
    ]
    with open(out_path, "w", newline="") as f:
        w = csv.DictWriter(f, fieldnames=fieldnames)
        w.writeheader()
        w.writerows(rows)

    print(f"Wrote {len(rows)} rows to {out_path}")


if __name__ == "__main__":
    main()
