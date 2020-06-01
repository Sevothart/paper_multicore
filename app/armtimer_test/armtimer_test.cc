#include <utility/ostream.h>
#include <time.h>
#include <process.h>
#include "instr_timer.h"

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