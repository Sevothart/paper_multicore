#include <utility/ostream.h>
#include <machine/cortex/raspberry_pi3/raspberry_pi3_gpio.h>

using namespace EPOS;
OStream cout;

long int z3 = 1000; 
long int z6 = 1000000;

int main() {  

    cout << "1" << endl;
    GPIO_Engine * led = new GPIO_Engine( GPIO_Common::B, 7, GPIO_Common::OUT, GPIO_Common::DOWN, GPIO_Common::NONE);
    cout << "2" << endl;
    
    /*
    long unsigned int start[10];
    long unsigned int end[10];
    int int_time[] = { 10, 20, 30, 40, 50, 60, 70, 80, 90, 100 };
    // int int_time[] = { 100, 200, 300, 400, 600, 800, 1000, 3000, 5000, 10000 };              // over 3000us, counts OK!
    // int int_time[] = { 1000, 2000, 3000, 4000, 6000, 8000, 10000, 30000, 50000, 100000 };    // This counts OK!
    Alarm::delay( 2000000 );

    cout << "Start." << endl;
    for(int i=0; i<10; i++){
        start[i] = (arm_timer->count() * ns) / arm_timer->clock();
        //start[i] = tsc_timer.time_stamp();
        Alarm::delay( int_time[i] );
        //end[i] = tsc_timer.time_stamp();
        end[i] = (arm_timer->count() * ns) / arm_timer->clock();

        Alarm::delay( 1000000 );
    }

    for( int i=0; i<10; i++){
        cout << "ARM" << i << ": " << end[i] - start[i] << "ns" << endl;
    }
    */
    return 0;
}