// EPOS ARMv7 CPU Mediator Initialization

#include <architecture.h>
#include <machine/cortex/engine/cortex_a53/bcm_arm_timer.h>

extern "C" { void __epos_library_app_entry(void); }

__BEGIN_SYS

void CPU::init()
{
    db<Init, CPU>(TRC) << "CPU::init()" << endl;

    if(CPU::id() == 0) {
        if(Traits<MMU>::enabled)
            MMU::init();
        else
            db<Init, MMU>(WRN) << "MMU is disabled!" << endl;
    }

#ifdef __PMU_H
    if(Traits<PMU>::enabled)
        PMU::init();
#endif

#ifdef __TSC_H
    if(Traits<TSC>::enabled){
        TSC::init();
        ARM_Timer::init();
    }
#endif
}

__END_SYS
