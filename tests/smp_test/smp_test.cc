#include <utility/ostream.h>
#include <synchronizer.h>
#include <process.h>
#include <time.h>
#include <architecture/cpu.h>

using namespace EPOS;

#define SILENT 1

OStream cout;

const unsigned int t_num = 4;

const int iterations = 10; // 5 sec

Semaphore print;

Thread * threads[t_num];

int test() {
    int cpu = CPU::id(); // m1
    #if !SILENT
        print.p();
        cout << "CPU[" << cpu << "] Start!" << endl;
        print.v();
    #endif
    for (int i = 0; i < iterations; ++i)
    {
        Delay(500000);
        #if !SILENT
            print.p();
            cout << "CPU[" << cpu << "] Iter["<< i <<"]" << ",&I=" << &i << endl;
            print.v();
        #endif
    }

    print.p();
    cout << "CPU[" << cpu << "] End!" << endl;
    print.v();

    return 0;
}

int main()
{
    cout << "Simple SMP Tester!" << endl;
    #if !SILENT
        cout << t_num << " Threads will be created following CPU Affinity Scheduling!" << endl;
        cout << "Will create them now!" << endl;
    #endif
    
    print = Semaphore(1);
    print.p();

    for (int i = 0; i < t_num; ++i)
    {
        threads[i] = new Thread(&test);
    }
    #if !SILENT
        cout << "All Threads have been created! \n Now they will sleep for 0.5s each and print a message for " << iterations << " Times \n Releasing print lock!" << endl;
    #endif
    print.v();

    for (int i = 0; i < t_num; ++i)
    {
        threads[i]->join();
    }
    #if !SILENT
        cout << "All threads have ended!" << endl;
        cout << "Simple SMP Tester will now delete them!" << endl;
    #endif
    for (int i = 0; i < t_num; ++i)
    {
        delete threads[i];
    }
    #if !SILENT
        cout << "Sleeping for 2 second, then the system will be rebooted!" << endl;
    #endif
    cout << "Goodbey world!" << endl;

    return 0;
}
