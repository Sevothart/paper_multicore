#include <utility/ostream.h>
#include <time.h>
#include <process.h>
#include <real-time.h>
#include <synchronizer.h>
#include "inst-timer.h"

using namespace EPOS;
OStream cout;

Mutex entry;
typedef Semaphore_MSRP<true, false> Semaphore_Type;
Semaphore_Type * sem;

int recurso(int id, int iterations)
{
    for(int i = 0; i < iterations; i++) {
        sem->p();

        // cout << "P_T | Thread_id " << id << " at core " << CPU::id() << " in iteration " << i << endl;
        Alarm::delay(1000);
        // cout << "V_T | Thread_id " << id << " at core " << CPU::id() << " in iteration " << i << endl;
        
        sem->v();
        Periodic_Thread::wait_next();
    }
    return 0;
}

void thread_creator(int j)
{
    ITimer t;
    int iterations = 1000;

    int levels[j];
    for(int i = 0; i < j; i++)
        levels[i] = 1;

    Thread * threads[j];
    Periodic_Thread::Configuration config = Periodic_Thread::Configuration(50000, 50000, 1000, Periodic_Thread::NOW, iterations, 0, Periodic_Thread::READY, Periodic_Thread::Criterion( 50000, 50000, 1000, 0 ));

    for(int i = 0; i < j; i++)
        threads[i] = new Periodic_Thread( config, &recurso, i, iterations );

    sem = new Semaphore_Type( threads, levels, j, 1 );

    for(int i = 0; i < j; i++)
        threads[i]->join();

    for(int i = 0; i < j; i++)
        delete threads[i];
    
    t.printData();
    t.cleanData();
    delete sem;
}

int main() {
    cout << "Starting Semaphore Wait benchmark test..." << endl;

    int N_T = 1;

    for(int i = 1; i <= N_T; i++)
    {
        cout << "\n---- SEMAPHORE WITH " << i << " THREADS ----\n";
        thread_creator(i);
    }

    cout << "simulation ended" << endl;
    while (1)
    {
        Alarm::delay(1000000);
        cout << "simulation ended" << endl;
    }
    return 0;
}