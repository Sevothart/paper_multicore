#include <utility/ostream.h>
#include <synchronizer.h>
#include <process.h>
#include <clerk.h>
#include <time.h>
#include <architecture/cpu.h>
#include <machine.h>

using namespace EPOS;

OStream cout;

const bool SILENT = false;
const unsigned int THREADS = 4;
const unsigned int ITERATIONS = 10; // 5 sec

Semaphore print;

Thread * thread[THREADS];

int test() {
    int cpu = CPU::id();
    if (!SILENT) {
        print.p();
        cout << "CPU[" << cpu << "] Start! Clock = " << Machine::clock() << endl;
        print.v();
    }
    TSC::Time_Stamp t0 = 0;
    TSC::Time_Stamp t1 = 0;

    for (unsigned int i = 0; i < ITERATIONS; ++i) {
        if (!cpu) {
            print.p();
            cout << "Clock = " <<Machine::clock(1200000000 - (i % 7)*100000000) << endl;
            print.v();
        }
        t0 = TSC::time_stamp();
        Delay(500000);
        //for (int i = 0; i < cpu+100000000; ++i) ASM("nop"); // to evaluate clock change
        t1 = TSC::time_stamp();
        if (!SILENT) {
            print.p();
            cout << "CPU[" << cpu << "] Iter["<< i <<"]" << ",t=" << t1-t0 << endl;
            print.v();
        }
    }

    print.p();
    cout << "CPU[" << cpu << "] End!" << endl;
    print.v();

    return 0;
}

int main()
{
    cout << "Simple SMP Tester!" << endl;
    if (!SILENT) {
        cout << THREADS << " Threads will be created following CPU Affinity Scheduling!" << endl;
    }
    
    print = Semaphore(1);
    print.p();

    cout << "clock["<< CPU::id() <<"]" << Machine::clock() << endl;
    Hertz new_clock = 1200000000;
    cout << "    clock change to" << new_clock << "Hz" << endl;
    cout << "    clock now is = " << Machine::clock(new_clock) << endl;

    for (unsigned int i = 0; i < THREADS; ++i) {
        thread[i] = new Thread(&test);
    }

    if (!SILENT) {
        cout << "All Threads have been created! \n Now they will sleep for 0.5s each and print a message for " << ITERATIONS << " Times \n Releasing print lock!" << endl;
    }
    Monitor::enable_captures(TSC::time_stamp());
    print.v();

    for (unsigned int i = 0; i < THREADS; ++i) {
        thread[i]->join();
    }

    if (!SILENT) {
        cout << "All threads have ended!" << endl;
        cout << "Simple SMP Tester will now delete them!" << endl;
    }

    for (unsigned int i = 0; i < THREADS; ++i) {
        delete thread[i];
    }

    cout << "Goodbey world!" << endl;
    Monitor::disable_captures();

    return 0;
}
