/** @file
 * Declaration of a fake device.
 */

#ifndef __HW_LOCKS_HH__
#define __HW_LOCKS_HH__

#include <string>

#include "dev/io_device.hh"
#include "mem/packet.hh"
#include "params/HWLocks.hh"
// #include "params/HWLocksDriver.hh"

namespace gem5
{

/**
 * IsaFake is a device that returns, BadAddr, 1 or 0 on all reads and
 *  rites. It is meant to be placed at an address range
 * so that an mcheck doesn't occur when an os probes a piece of hw
 * that doesn't exist (e.g. UARTs1-3), or catch requests in the memory system
 * that have no responders..
 */
class HWLocks : public BasicPioDevice
{
  protected:
    uint8_t retData8;
    uint16_t retData16;
    uint32_t retData32;
    uint64_t retData64;

  public:
    PARAMS(HWLocks);

    /**
      * The constructor for Isa Fake just registers itself with the MMU.
      * @param p params structure
      */
    HWLocks(const Params &p);

    /**
     * This read always returns -1.
     * @param pkt The memory request.
     * @param data Where to put the data.
     */
    virtual Tick read(PacketPtr pkt);

    /**
     * All writes are simply ignored.
     * @param pkt The memory request.
     * @param data the data to not write.
     */
    virtual Tick write(PacketPtr pkt);
};

// class HWLocksDriver : public EmulatedDriver
// {
//   private:
//     HWLocks *hardware;

  // public:
  //   // PARAMS(HWLocksDriver);
  //   // typedef HWLocksDriverParams Params;
  //   HWLocksDriver(Params *p) : EmulatedDriver(p), hardware(p->hardware) {}

  //   int open(LiveProcess *p, ThreadContext *tc, int mode, int flags) {
  //       // hardware->open(tc);
  //       // return 1;
  //       panic("HWLocks driver doesn't implement any ioctls");
  //       M5_DUMMY_RETURN
  //   }

  //   int ioctl(LiveProcess *p, ThreadContext *tc, unsigned req) {
  //       panic("HWLocks driver doesn't implement any ioctls");
  //       M5_DUMMY_RETURN
  //   }
// };

} // namespace gem5

#endif // __HW_LOCKS_HH__
