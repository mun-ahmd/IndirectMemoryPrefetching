#!/usr/bin/env python3
import os
import subprocess
import argparse
from collections import defaultdict

# The exact extern "C" function names
TARGET_FUNCTIONS = [
    "create_params",
    "create_enable",
    "delete_params",
    "delete_enable",
    "register_node_with_size",
    "register_trav_edge1",
    "register_trav_edge2",
    "register_trig_edge1",
    "register_trig_edge2",
    "register_identify_edge",
    "register_identify_edge_source",
    "register_identify_edge_target",
    "sim_user_pf_set_param",
    "sim_user_pf_set_enable",
    "sim_user_pf_enable",
    "sim_user_pf_disable",
    "sim_user_wait",
    "sim_roi_start",
    "sim_roi_end",
]


def is_elf_binary(filepath):
    """Check if a file is an ELF binary."""
    try:
        if not os.path.isfile(filepath) or os.path.islink(filepath):
            return False
        with open(filepath, "rb") as f:
            return f.read(4) == b"\x7fELF"
    except Exception:
        return False


def analyze_binaries(tool_path, directory):
    # Dictionary to store: { binary_name: { function_name: count } }
    results = {}

    binaries = [
        os.path.join(directory, f)
        for f in os.listdir(directory)
        if is_elf_binary(os.path.join(directory, f)) and not f.endswith(".o")
    ]

    if not binaries:
        print(f"No ELF binaries found in '{directory}'.")
        return

    print(f"Analyzing {len(binaries)} binaries...\n")

    for binary_path in binaries:
        binary_name = os.path.basename(binary_path)
        results[binary_name] = defaultdict(int)

        try:
            # -d disassembles to find all calls and the definition
            cmd = [tool_path, "-d", binary_path]
            result = subprocess.run(
                cmd, capture_output=True, text=True, errors="ignore", check=True
            )
            output = result.stdout

            for func in TARGET_FUNCTIONS:
                # Searching for <name> ensures we don't catch substrings
                # (e.g., register_identify_edge vs register_identify_edge_source)
                count = output.count(f"<{func}")
                if count > 0:
                    results[binary_name][func] = count

        except subprocess.CalledProcessError:
            results[binary_name]["ERROR"] = "Failed to process"
        except Exception as e:
            results[binary_name]["ERROR"] = f"Unexpected error: {e}"

    # --- PRINTING THE RESULTS ---
    for binary, counts in results.items():
        print(f"Binary: {binary}")
        print("-" * (len(binary) + 8))

        if not counts:
            print("  (No prefetcher functions found)")
        elif "ERROR" in counts:
            print(f"  [!] {counts['ERROR']}")
        else:
            # Sort by count descending
            sorted_funcs = sorted(counts.items(), key=lambda x: x[1], reverse=True)
            for func, count in sorted_funcs:
                print(f"  {func:<35} : {count}")
        print()  # Newline between binaries


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Count Prefetcher API occurrences per binary."
    )
    parser.add_argument(
        "-t", "--tool", required=True, help="Path to loongarch64-objdump"
    )
    parser.add_argument("-d", "--dir", default=".", help="Directory to scan")

    args = parser.parse_args()
    analyze_binaries(args.tool, args.dir)
