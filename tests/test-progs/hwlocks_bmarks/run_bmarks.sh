GEM5=/gem5/build/X86/gem5.opt       # gem5 to run
# any parameters to gem5
GEM5_PARAMS='--debug-flags=Exec --debug-file=/gem5/tests/test-progs/hwlocks_bmarks/scratch/trace.out'      
TRACE=/gem5/tests/test-progs/hwlocks_bmarks/scratch/trace.out
LOCKS=/gem5/tests/test-progs/hwlocks_bmarks/scratch/lock_addrs.out
CONFIG=/gem5/configs/example/se.py  # configuration file
# any parameters to the configuration file
CONFIG_PARAMS='-I 1000000000 --cpu-type=TimingSimpleCPU --caches --l2cache --mem-size 8192MiB'         
INPUT_FILE=/gem5/tests/test-progs/hwlocks_bmarks/inputs/32k_input  # input
INPUT_SIZE=32768                   # size of the input
MAX_VAL=255                         # maximum value of the input

cd /gem5/tests/test-progs/hwlocks_bmarks

# run the benchmarks for varying number of CPUs
for num_cpus in 1 2 4 8 16 32; do
    # run the benchmark
    $GEM5 $GEM5_PARAMS $CONFIG $CONFIG_PARAMS -n $(($num_cpus + 1)) -c histogram_a-x86 -o="$INPUT_FILE $INPUT_SIZE $MAX_VAL $num_cpus" > $LOCKS
    # analyze the trace 
    python3 analyze_trace.py a-$num_cpus-cpus-$INPUT_SIZE $(($num_cpus + 1)) > ./stats_output/a-$num_cpus-cpus-$INPUT_SIZE
    # add gem5 stats to the trace analysis
    cat m5out/stats.txt >> ./stats_output/a-$num_cpus-cpus-$INPUT_SIZE

    # b
    $GEM5 $GEM5_PARAMS $CONFIG $CONFIG_PARAMS -n $(($num_cpus + 1)) -c histogram_b-x86 -o="$INPUT_FILE $INPUT_SIZE $MAX_VAL $num_cpus" > $LOCKS
    python3 analyze_trace.py b-$num_cpus-cpus-$INPUT_SIZE $(($num_cpus + 1)) > ./stats_output/b-$num_cpus-cpus-$INPUT_SIZE
    cat m5out/stats.txt >> ./stats_output/b-$num_cpus-cpus-$INPUT_SIZE

    # d
    $GEM5 $GEM5_PARAMS $CONFIG $CONFIG_PARAMS -n $(($num_cpus + 1)) -c histogram_d-x86 -o="$INPUT_FILE $INPUT_SIZE $MAX_VAL $num_cpus" > $LOCKS
    python3 analyze_trace.py d-$num_cpus-cpus-$INPUT_SIZE $(($num_cpus + 1)) > ./stats_output/d-$num_cpus-cpus-$INPUT_SIZE
    cat m5out/stats.txt >> ./stats_output/d-$num_cpus-cpus-$INPUT_SIZE
done