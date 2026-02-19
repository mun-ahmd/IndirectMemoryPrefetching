import sys
import json
import subprocess
import os

# --- Configuration ---
PREFETCHER_OPTIONS = [
    "tyche",
    "imp",
    #    "va_ampm_lite",
    # "ipcp_l1d",
    "no",
]

CONFIG_FILE = "champsim_config.json"
BINARY_PATH = "bin/champsim"

# Check for trace files
if len(sys.argv) < 2:
    print("Usage: python run_experiments.py <trace_file1> [trace_file2 ...]")
    sys.exit(1)

trace_files = sys.argv[1:]


def run_command(cmd, shell=False):
    """Helper to run shell commands and print them."""
    print(f"Running: {' '.join(cmd) if isinstance(cmd, list) else cmd}")
    try:
        subprocess.check_call(cmd, shell=shell)
    except subprocess.CalledProcessError as e:
        print(f"Error executing command. Exiting.")
        sys.exit(1)


# --- Main Loop ---
for prefetcher in PREFETCHER_OPTIONS:
    print(f"\n==================================================")
    print(f"   Building with L1D Prefetcher: {prefetcher}")
    print(f"==================================================")

    # 1. Load config, modify prefetcher, save config
    try:
        with open(CONFIG_FILE, "r") as f:
            config = json.load(f)

        # Update the L1D prefetcher
        config["L1D"]["prefetcher"] = prefetcher

        with open(CONFIG_FILE, "w") as f:
            json.dump(config, f, indent=4)

    except Exception as e:
        print(f"Error modifying {CONFIG_FILE}: {e}")
        sys.exit(1)

    # 2. Build ChampSim
    run_command(["make", "clean"])
    # We must run config.sh to apply the JSON changes before make
    run_command(["./config.sh", CONFIG_FILE])
    run_command(["make", f"-j{os.cpu_count()}"])

    # 3. Run Traces
    for trace in trace_files:
        trace_parent = os.path.basename(os.path.dirname(trace))
        trace_name = os.path.basename(trace)
        # Create a distinct log file for this run
        output_log = f"results_{prefetcher}_{trace_parent}_{trace_name}.txt"

        print(f"--> Running trace: {trace_name}")

        # Construct the simulation command
        sim_cmd = [
            BINARY_PATH,
            "--warmup_instructions",
            "20000000",
            "--simulation_instructions",
            "100000000",
            "-loongarch",
            trace,
        ]

        # Execute and save output to file
        with open(output_log, "w") as outfile:
            subprocess.run(sim_cmd, stdout=outfile, stderr=subprocess.STDOUT)

        print(f"    Finished. Output saved to {output_log}")

print("\nAll experiments completed.")
