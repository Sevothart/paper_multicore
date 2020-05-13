// EPOS Raspberry Pi3 (Cortex-A53) Mediator Declarations

#ifndef __raspberry_pi3_machine_h
#define __raspberry_pi3_machine_h

#include <machine/machine.h>
#include <system/memory_map.h>
#include <system.h>
#include "raspberry_pi3_ioctrl.h"

__BEGIN_SYS

class Raspberry_Pi3: public Machine_Common
{
    friend Machine; // for pre_init() and init()

protected:
    typedef CPU::Reg32 Reg32;
    typedef CPU::Log_Addr Log_Addr;


public:
    Raspberry_Pi3() {}

    static void delay(const Microsecond & time);

    static void reboot();
    static void poweroff() { reboot(); }

    static const UUID & uuid() { return System::info()->bm.uuid; }

public:
    static void smp_barrier_init(unsigned int n_cpus) {
        _cores = n_cpus;
    }

    static Hertz clock() {
        return static_cast<Hertz>(ioc()->arm_clock());
    }

    static unsigned int frequency() { 
        // ARM MBOX read consumes a lot of time, its efficient to maintain the frequency in a internal variable
        return _frequency;
    }

    static Hertz max_clock() {
        return static_cast<Hertz>(ioc()->arm_max_clock());
    }

    static Hertz min_clock() {
        return static_cast<Hertz>(ioc()->arm_min_clock());
    }

    static Hertz clock(const Hertz & frequency) {
        _frequency = frequency;
        return static_cast<Hertz>(ioc()->arm_clock(frequency));
    }

private:
    static void pre_init();
    static void init() {}

private:
    static volatile unsigned int _cores;
    static Hertz _frequency;

private:
    static IOCtrl * ioc() { return reinterpret_cast<IOCtrl *>(Memory_Map::MBOX_BASE); }
};

typedef Raspberry_Pi3 Machine_Model;

__END_SYS

#endif
