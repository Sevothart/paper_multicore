#include <utility/ostream.h>
#include <time.h>
#include <process.h>
#include <real-time.h>
#include <synchronizer.h>
#include "inst-timer.h"

using namespace EPOS;
OStream cout;
typedef Semaphore_MSRP Semaphore_Type;
Semaphore_Type * sem;

int resource(int id, int iterations)
{
    for (int i = 0; i < iterations; i++)
    {
        cout << "resource::try_access | Thread " << id << endl;
        sem->p();

        cout << "resource::get_access() | Thread " << id << " at core " << CPU::id() << " in iteration " << i << endl;
        Alarm::delay(1000);
        cout << "resource::finished_computation() | Thread " << id << " at core " << CPU::id() << " in iteration " << i << endl;

        sem->v();
        cout << "resource::exited() | Thread " << id << endl;
        Periodic_Thread::wait_next();
    }
    return 0;
}
void thread_creator(int j)
{
    ITimer t;
    int iterations = 2;

    int levels[j];
    for(int i = 0; i < j; i++)
        levels[i] = 1+i;

    Thread * threads[j];

    for(int i = 0; i < j; i++)
    {
        Periodic_Thread::Configuration config = Periodic_Thread::Configuration(50000*(i+1), 50000*(i+1), 1000, Periodic_Thread::NOW, iterations, 0, Periodic_Thread::READY, Periodic_Thread::Criterion( 50000*(i+1), 50000*(i+1), 1000, 0 ));
        threads[i] = new Periodic_Thread( config, &resource, i, iterations );
    }

    sem = new Semaphore_Type( threads, levels, j, 1 );

    for(int i = 0; i < j; i++)
        threads[i]->join();

    for(int i = 0; i < j; i++)
        delete threads[i];

    TSC::frequency();
    t.timer.frequency();
    t.stop("thread_creator", Thread::self() );
    t.printData();
    t.cleanData();
    delete sem;
}

int main() {
    cout << "Starting Semaphore Wait benchmark test..." << endl;

    int N_T = 3;

    for(int i = 1; i <= N_T; i++)
    {
        cout << "\n---- SEMAPHORE WITH " << i << " THREADS ----\n";
        thread_creator(i);
    }

    /*
    cout << "simulation ended" << endl;
    while (1)
    {
        Alarm::delay(1000000);
        cout << "simulation ended" << endl;
    }
    */
    return 0;
}