/** @file
 * Isa Fake Device implementation
 */

#include "dev/hwlocks.hh"

#include "base/trace.hh"
// #include "debug/IsaFake.hh"
#include "mem/packet.hh"
#include "mem/packet_access.hh"
#include "sim/system.hh"

namespace gem5
{

HWLocks::HWLocks(const Params &p)
    : BasicPioDevice(p, p.ret_bad_addr ? 0 : p.pio_size)
{
    retData8 = p.ret_data8;
    retData16 = p.ret_data16;
    retData32 = p.ret_data32;
    retData64 = p.ret_data64;
}

Tick
HWLocks::read(PacketPtr pkt)
{
    pkt->makeAtomicResponse();
    panic("***Device being accessed by cache?\n");

    // if (params().warn_access != "")
    //      warn("Device %s accessed by read to address %#x size=%d\n",
    //              name(), pkt->getAddr(), pkt->getSize());
    // if (params().ret_bad_addr) {
    //     // DPRINTF(HWLocks, "read to bad address va=%#x size=%d\n",
    //     //        pkt->getAddr(), pkt->getSize());
    //     pkt->setBadAddress();
    // } else {
        assert(pkt->getAddr() >= pioAddr && pkt->getAddr() < pioAddr + pioSize);
        // DPRINTF(HWLocks, "read  va=%#x size=%d\n",
        //        pkt->getAddr(), pkt->getSize());
        switch (pkt->getSize()) {
          case sizeof(uint64_t):
             pkt->setLE(retData64);
             break;
          case sizeof(uint32_t):
             pkt->setLE(retData32);
             break;
          case sizeof(uint16_t):
             pkt->setLE(retData16);
             break;
          case sizeof(uint8_t):
             pkt->setLE(retData8);
             break;
          default:
             if (params().fake_mem)
                 std::memset(pkt->getPtr<uint8_t>(), 0, pkt->getSize());
             else
                 panic("invalid access size! Device being accessed by cache?\n");
        }
    // }
    return pioDelay;
}

Tick
HWLocks::write(PacketPtr pkt)
{
    pkt->makeAtomicResponse();
    if (params().warn_access != "") {
        uint64_t data;
        switch (pkt->getSize()) {
          case sizeof(uint64_t):
            data = pkt->getLE<uint64_t>();
            break;
          case sizeof(uint32_t):
            data = pkt->getLE<uint32_t>();
            break;
          case sizeof(uint16_t):
            data = pkt->getLE<uint16_t>();
            break;
          case sizeof(uint8_t):
            data = pkt->getLE<uint8_t>();
            break;
          default:
            panic("invalid access size: %u\n", pkt->getSize());
        }
        warn("Device %s accessed by write to address %#x size=%d data=%#x\n",
                name(), pkt->getAddr(), pkt->getSize(), data);
    }
    if (params().ret_bad_addr) {
        // DPRINTF(HWLocks, "write to bad address va=%#x size=%d \n",
        //        pkt->getAddr(), pkt->getSize());
        pkt->setBadAddress();
    } else {
        // DPRINTF(HWLocks, "write - va=%#x size=%d \n",
        //        pkt->getAddr(), pkt->getSize());

        if (params().update_data) {
            switch (pkt->getSize()) {
              case sizeof(uint64_t):
                retData64 = pkt->getLE<uint64_t>();
                break;
              case sizeof(uint32_t):
                retData32 = pkt->getLE<uint32_t>();
                break;
              case sizeof(uint16_t):
                retData16 = pkt->getLE<uint16_t>();
                break;
              case sizeof(uint8_t):
                retData8 = pkt->getLE<uint8_t>();
                break;
              default:
                panic("invalid access size!\n");
            }
        }
    }
    return pioDelay;
}

// HWLocksDriver*
// HWLocksDriverParams::create()
// {
//     return new HWLocksDriver(this);
// }

} // namespace gem5
