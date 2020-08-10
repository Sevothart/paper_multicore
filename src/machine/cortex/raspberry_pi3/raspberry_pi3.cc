// EPOS ARM Cortex-A53 Mediator Implementation

#include <machine/machine.h>
#include <machine/display.h>

__BEGIN_SYS

volatile unsigned int Raspberry_Pi3::_cores;

void Raspberry_Pi3::reboot()
{
    db<Machine>(WRN) << "Machine::reboot()" << endl;
    ASM("wfi");
//TODO: reboot!
}

__END_SYS
