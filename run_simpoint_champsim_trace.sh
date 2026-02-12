#!/bin/bash

set -e

output=$1
# cmd="~/coremark/coremark-aarch64-linux-gnu.exe 0 0 0 10000 > /dev/null"
cmd="${@:2}"
bb_interval=100000000
reserved_mem="4G" # You may need to adjust this based on the workload
stack_size="100M" # currently not applied apply using -s ${stack_size}
libs_path="./loongarch-tools/target/usr/" # now we do not need to compile the code with -static flags
guest_env="LD_LIBRARY_PATH=/usr/lib64:/lib64 -E OMP_NUM_THREADS=1" # Currently limited to a single thread, due to simpoint analysis and possibly the plugin not being capable of multiple threads

CMD_GETBBV="./qemu/build/qemu-loongarch64 -R ${reserved_mem}  -L ${libs_path} -E ${guest_env} -D log.txt -d plugin -plugin ./qemu_plugins/build/libbbv_ibar.so,name=${output},size=${bb_interval} -- $cmd"

echo ${CMD_GETBBV}
eval ${CMD_GETBBV}

CMD_GETSIMPOINT="./SimPoint.3.2/bin/simpoint -maxK 5 -loadFVFile ${output}/bbv -saveSimpoints ${output}/simpoints -saveSimpointWeights ${output}/weights >/dev/null"
echo ${CMD_GETSIMPOINT}
eval ${CMD_GETSIMPOINT}

CMD_GETTRACE="env TRACE_FILENAME=${output}/champsim.trace SIMPOINT_FILE=${output}/simpoints BB_INTERVAL=${bb_interval} ./qemu/build/qemu-loongarch64 -R ${reserved_mem}  -L ${libs_path} -E ${guest_env} -D log.txt -d plugin -plugin ./qemu_plugins/build/libchampsim_la_with_reg_simpoint.so -- ${cmd}"
echo ${CMD_GETTRACE}
eval ${CMD_GETTRACE}

TARGET_TRACE=$(find "${output}" -maxdepth 1 -name "*.champsim.trace" -print -quit)
CMD_XZTRACE="xz -v -T0 ${TARGET_TRACE}"
echo ${CMD_XZTRACE}
eval ${CMD_XZTRACE}

# CMD_TYCHE="~/Tyche-Artifact/bin/champsim --warmup_instructions 20000000 --simulation_instructions 100000000 -loongarch ${TARGET_TRACE}.xz"
# echo ${CMD_TYCHE}
# eval ${CMD_TYCHE}
