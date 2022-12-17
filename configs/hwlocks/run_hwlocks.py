""" This file creates a barebones system and executes 'hello', a simple Hello
World application. Adds a simple memobj between the CPU and the membus.

This config file assumes that the x86 ISA was built.
"""

# import the m5 (gem5) library created when gem5 is built
import m5

# add common scripts
m5.util.addToPath('../')

# import all of the SimObjects
from m5.objects import *
from caches import *

from common import SimpleOpts

# parse arguments
opts = SimpleOpts.parse_args()

# create the system we are going to simulate
system = System()

# Set the clock frequency of the system (and all of its children)
system.clk_domain = SrcClockDomain()
system.clk_domain.clock = '1GHz'
system.clk_domain.voltage_domain = VoltageDomain()

# Set up the system
system.mem_mode = 'timing'               # Use timing accesses
system.mem_ranges = [AddrRange('1024MiB')] # Create an address range

# Create a simple CPU
system.cpu = TimingSimpleCPU()
# Create the simple memory object
# system.memobj = SimpleMemobj()

system.membus = SystemXBar()

# create our hardware locks
system.hwlocks = HWLocks(pio_addr=0x100000, pio_size=8)
system.hwlocks_driver = HWLocksDriver(hardware=system.hwlocks, filename="hwlocks")
# create L1 caches
system.cpu.icache = L1ICache(opts)
system.cpu.dcache = L1DCache(opts)
# system.cpu.dcache.addr_ranges = system.mem_ranges
# constrain dcache to 10MB addr range just so it doesn't collide with pio device
system.cpu.dcache.addr_ranges = [AddrRange('1MiB')]

system.cpu.l1bus = L2XBar()
system.cpu.dcache_port = system.cpu.l1bus.slave

# Hook the CPU ports up to the cache
system.cpu.icache.connectCPU(system.cpu)
system.cpu.dcache.cpu_side = system.cpu.l1bus.master

# Create a memory bus, a coherent crossbar, in this case
system.l2bus = L2XBar()

# Hook the CPU ports up to the l2bus
system.cpu.icache.connectBus(system.l2bus)
system.cpu.dcache.connectBus(system.l2bus)

# create an L2 cache and connect it to l2bus
system.l2cache = L2Cache(opts)
system.l2cache.connectCPUSideBus(system.l2bus)

# connect L2 cache to membus
system.l2cache.connectMemSideBus(system.membus)

# connect the hardware locks
# system.hwlocks.pio = system.cpu.l1bus.master
system.hwlocks.pio = system.cpu.l1bus.master

# Connect the memobj
# system.memobj.mem_side = system.membus.cpu_side_ports

# create the interrupt controller for the CPU and connect to the membus
system.cpu.createInterruptController()
# TODO Commented this out to build, Class RiscvInterrupts doesn't have these fields
# I think this is for x86 only
# system.cpu.interrupts[0].pio = system.membus.mem_side_ports
# system.cpu.interrupts[0].int_requestor = system.membus.cpu_side_ports
# system.cpu.interrupts[0].int_responder = system.membus.mem_side_ports

# Connect the system up to the membus
system.system_port = system.membus.slave

# Create a DDR3 memory controller and connect it to the membus
system.mem_ctrl = MemCtrl()
system.mem_ctrl.dram = DDR3_1600_8x8()
system.mem_ctrl.dram.range = system.mem_ranges[0]
system.mem_ctrl.port = system.membus.master

# Create a process for a simple "Hello World" application
process = Process()
# Set the command
# grab the specific path to the binary
thispath = os.path.dirname(os.path.realpath(__file__))
# binpath = os.path.join(thispath, '../../tests/test-progs/threads/src/threads_lock_sw-riscv')
binpath = os.path.join(thispath, '../../tests/test-progs/devices/write_dev-riscv')
# cmd is a list which begins with the executable (like argv)
process.cmd = [binpath]
# Set the cpu to use the process as its workload and create thread contexts
system.cpu.workload = process
system.cpu.createThreads()

system.workload = SEWorkload.init_compatible(binpath)

# set up the root SimObject and start the simulation
root = Root(full_system = False, system = system)
# instantiate all of the objects we've created above
m5.instantiate()

print("Beginning simulation!")
exit_event = m5.simulate()
print('Exiting @ tick %i because %s' % (m5.curTick(), exit_event.getCause()))
