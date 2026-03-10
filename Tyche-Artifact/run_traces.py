import sys
import json
import subprocess
import os

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


for trace in trace_files:
    trace_parent = os.path.basename(os.path.dirname(trace))
    trace_name = os.path.basename(trace)
        # Create a distinct log file for this run
    output_log = f"test_Final{trace_parent}_{trace_name}.txt"

    print(f"--> Running trace: {trace_name}")

        # Construct the simulation command
    sim_cmd = [
            "bin/champsim",
            "--warmup_instructions",
            "2000000",
            "--simulation_instructions",
            "10000000",
            "-loongarch",
            trace,
        ]

        # Execute and save output to file
    with open(output_log, "w") as outfile:
            subprocess.run(sim_cmd, stdout=outfile, stderr=subprocess.STDOUT)

    print(f"    Finished. Output saved to {output_log}")

print("\nAll experiments completed.")
