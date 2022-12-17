from m5.params import *
from m5.objects.Device import BasicPioDevice
from m5.objects.Process import EmulatedDriver

class HWLocks(BasicPioDevice):
    type = 'HWLocks'
    cxx_header = "dev/hwlocks.hh"
    cxx_class = 'gem5::HWLocks'

    devicename = Param.String("hwlocks", "Name of device to error on")

    # parameters copied from Device.py for IsaFake
    pio_size = Param.Addr(0x8, "Size of address range")
    ret_data8 = Param.UInt8(0xFF, "Default data to return")
    ret_data16 = Param.UInt16(0xFFFF, "Default data to return")
    ret_data32 = Param.UInt32(0xFFFFFFFF, "Default data to return")
    ret_data64 = Param.UInt64(0xFFFFFFFFFFFFFFFF, "Default data to return")
    ret_bad_addr = Param.Bool(False, "Return pkt status bad address on access")
    update_data = Param.Bool(False, "Update the data that is returned on writes")
    warn_access = Param.String("", "String to print when device is accessed")
    fake_mem = Param.Bool(False,
      "Is this device acting like a memory and thus may get a cache line sized req")

class HWLocksDriver(EmulatedDriver):
    type = 'HWLocksDriver'
    cxx_header = "dev/hwlocks.hh"
    filename = "hwlocks"

    hardware = Param.HWLocks("The hwlocks hardware")