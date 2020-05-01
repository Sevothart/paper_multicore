#include <utility/ostream.h>
#include <time.h>

#include <machine/cortex/raspberry_pi3/raspberry_pi3_gpio.h>
#include <machine/cortex/engine/cortex_a53/bcm_arm_timer.h>

/*
    ARM_Timer MAX resolution: 4ns
    ARM_Timer MAX time that can be counted: 4.295s
*/

using namespace EPOS;
OStream cout;

int main() {

    cout << "Starting ARM_Timer benchmark test..." << endl;
    ARM_Timer * timer = new ARM_Timer();
    timer->enable();

    timer->start();
    Alarm::delay( 4295000 );
    timer->stop();

    while(1) { }

    return 0;
}