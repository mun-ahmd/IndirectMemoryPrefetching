./config.sh champsim_config.json
make
python3 run_traces.py ../../traces/champsim.trace_7916000000.champsim.trace.xz
make clean
