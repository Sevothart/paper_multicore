#include <utility/ostream.h>
#include <time.h>
#include <process.h>
// #include <machine/cortex/raspberry_pi3/raspberry_pi3_gpio.h>
#include "instr_timer.h"

/*
    ARM_Timer MAX resolution: 4ns
    ARM_Timer MAX time that can be counted: 4.295s
*/

using namespace EPOS;
OStream cout;

int main() {

    cout << "Starting ARM_Timer benchmark test..." << endl;
    
    ITimer t;
    Alarm::delay( 4295000 );
    t.stop("measure_t", Thread::self() );

    cout << "Ending ARM_Timer benchmark test..." << endl;
    return 0;
}