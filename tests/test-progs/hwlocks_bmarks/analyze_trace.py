import sys

# first argument: analysis name
# second argument: number of cpus
num_cpus = int(sys.argv[2])

# parse lock addresses in lock_addrs.out
hwlocks = []
with open('./scratch/lock_addrs.out', 'r') as la:
    stdout = la.readlines()
for l in stdout:
    if "lock address:" in l:
        hwlocks.append((l.split()[2]).replace('0x',''))

# cpus is a list of cpu ids
cpus = []
for x in range(num_cpus):
    cpus.append("cpu" + str(x))

# open trace
with open('./scratch/trace.out', 'r') as tr:
    insts = tr.readlines()

thread_ticks = [0] * num_cpus
final_tick = 0
lock_reqs = []
unlock_reqs = []
misses = []
for cpu in cpus:
    for inst in insts:
        # bring lock() requests for the same cpu together
        if cpu in inst and "pthread_mutex_lock" in inst:
            lock_reqs.append(inst)
        # bring unlock() requests for the same cpu together
        if cpu in inst and "pthread_mutex_unlock" in inst:
            unlock_reqs.append(inst)
        # bring read misses together
        # if "LockedRMWReadReq" in inst and "miss" in inst:
        #     misses.append(inst)
        # get total tick count
        if "syscall" in inst and "exit" in inst:
            final_tick = int((inst.split()[0]).replace(':','')) 

        # if thread start
        cpu_id = int(cpu.replace('cpu',''))
        if cpu in inst and "clone" in inst:
            if thread_ticks[cpu_id] == 0: 
                thread_ticks[cpu_id] = int((inst.split()[0]).replace(':','')) 
        # if thread exit
        if cpu in inst and "start_thread" in inst and "syscall" in inst:
            thread_ticks[cpu_id] = int((inst.split()[0]).replace(':','')) - thread_ticks[cpu_id]


print("------ Analysis for " + sys.argv[1] + " ------")

for hwlock_addr in hwlocks:
    acquire_tick_sums = [0] * num_cpus
    num_acquires = [0] * num_cpus
    release_tick_sums = [0] * num_cpus
    num_releases = [0] * num_cpus

    is_hwlock = False
    start_tick = 0
    for inst in lock_reqs:
        # if this lock is for a hwlock, set the flag
        if hwlock_addr in inst:
            is_hwlock = True
        # if this is the first instruction in an acquire, init start_tick
        if "pthread_mutex_lock " in inst:
            # print("start = " + inst)
            start_tick = int((inst.split()[0]).replace(':',''))
        # if this is the last instruction in an acquire, compute the tick cycles that have elapsed
        elif "RET_NEAR : wripi" in inst:
            # print("end = " + inst)
            ticks_elapsed = int((inst.split()[0]).replace(':','')) - start_tick
            if is_hwlock:
                cpu_id = int((inst.split()[1]).replace('system.cpu','').replace(':',''))
                acquire_tick_sums[cpu_id] += ticks_elapsed
                num_acquires[cpu_id] += 1
                # print("tick_sum = " + str(tick_sum))
            is_hwlock = False 

    print("lock with addr = 0x" + hwlock_addr)
    print("    avg ticks elapsed (acquire) = " + str(sum(acquire_tick_sums[1:]) / len(acquire_tick_sums[1:])))
    print("    number of acquires          = " + str(num_acquires))

    is_hwlock = False
    start_tick = 0
    for inst in unlock_reqs:
        # if this lock is for a hwlock, set the flag
        if hwlock_addr in inst:
            is_hwlock = True
        # if this is the first instruction in a release, init start_tick
        if "pthread_mutex_unlock " in inst:
            # print("start = " + str(int((inst.split()[0]).replace(':',''))))
            start_tick = int((inst.split()[0]).replace(':',''))
        # if this is the last instruction in a release, compute the tick cycles that have elapsed
        elif "RET_NEAR : wripi" in inst:
            # print("end = " + str(int((inst.split()[0]).replace(':',''))))
            ticks_elapsed = int((inst.split()[0]).replace(':','')) - start_tick
            if is_hwlock:
                cpu_id = int((inst.split()[1]).replace('system.cpu','').replace(':',''))
                release_tick_sums[cpu_id] += ticks_elapsed
                num_releases[cpu_id] += 1
                # print("tick_sum = " + str(tick_sum))
            is_hwlock = False 

    # num_misses = 0    
    # for miss in misses:
    #     if hwlock_addr in miss:
    #         num_misses += 1

    print("    avg ticks elapsed (release) = " + str(sum(release_tick_sums[1:]) / len(release_tick_sums[1:])))
    print("    number of releases          = " + str(num_releases))
    print("    avg ticks per core          = " + str(sum(thread_ticks[1:]) / len(thread_ticks[1:])))
    print("    total ticks                 = " + str(final_tick))
    # print("    number of misses        = " + str(num_misses))
    print("---------------------------------------")