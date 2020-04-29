#include <utility/ostream.h>
#include <time.h>

#include <machine/cortex/raspberry_pi3/raspberry_pi3_gpio.h>
#include <machine/cortex/engine/cortex_a53/bcm_arm_timer.h>

using namespace EPOS;

OStream cout;

long int z3 = 1000; 
long int z6 = 1000000;
long int z9 = 1000000000;

int main() {

    cout << "Starting ARM_Timer benchmark test..." << endl;
    ARM_Timer * timer = new ARM_Timer();
    timer->config(1, 1000000000);
    timer->enable();

    unsigned long int start = ( z9 * timer->count() ) / timer->clock(); 
    Alarm::delay( 1 );
    unsigned long int end = ( z9 * timer->count() ) / timer->clock();

    cout << "Start time [ns]: " << start << endl;
    cout << "End time [ns]: " << end << endl;

    while(1) { }

    return 0;
}