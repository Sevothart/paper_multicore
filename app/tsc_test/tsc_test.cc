#include <utility/ostream.h>
#include <time.h>

#include <machine/cortex/raspberry_pi3/raspberry_pi3_gpio.h>
#include <machine/cortex/engine/cortex_a53/bcm_arm_timer.h>

using namespace EPOS;
OStream cout;

int main() {

    cout << "Starting ARM_Timer benchmark test..." << endl;
    ARM_Timer * timer = new ARM_Timer();
    timer->enable();

    timer->start();
    Alarm::delay( 1000000 );
    timer->stop();

    timer->disable();

    while(1) { }

    return 0;
}