#include <utility/ostream.h>
#include <time.h>
// #include <process.h>
#include <real-time.h>
#include <synchronizer.h>
// #include "inst-timer.h"

using namespace EPOS;
OStream cout;
typedef Semaphore_MrsP Semaphore_Type;
// typedef Semaphore_MPCP<true> Semaphore_Type;

int solo_resource(int id, int iterations)
{
    for (int i = 0; i < iterations; i++)
    {
        cout << "solo_resource::start_job | Thread " << id << " at " << CPU::id() << endl;
        Alarm::delay(60000*(i+1));
        cout << "solo_resource::finish_job | Thread " << id << " at " << CPU::id() << endl;
        Periodic_Thread::wait_next();
    }
    return 0;
}

int resource_preempted(Semaphore_Type *sem, int id, int iterations)
{
    Periodic_Thread::Criterion * c = new Periodic_Thread::Criterion( 99999, 99999, 1500*2, 1 );

    for (int i = 0; i < iterations; i++)
    {
        cout << "resource::try_access | Thread " << id << endl;
        sem->p();
        cout << "resource::get_access() | Thread " << id << " at " << CPU::id() << endl;

        // Migration test
        Alarm::delay(1000);
        Periodic_Thread::Configuration solo_config = Periodic_Thread::Configuration(60000, 60000, 1500*2, Periodic_Thread::NOW, iterations, 0, Periodic_Thread::READY, Periodic_Thread::Criterion(60000, 60000, 1500, 0));
        Periodic_Thread * solo_thread = new Periodic_Thread(solo_config, &solo_resource, 3, iterations);

        Thread * prev = Thread::self();
        // prev->lock();
        // if( Semaphore_MrsP::ownerMigrated( prev, solo_thread ) )
        // {
            kout << "@owner " << prev << " preempted by " << solo_thread << ".\n";
            kout << "Moving @owner from " << prev->criterion().queue() << " to " << 1 << endl;

            
            prev->priority(*c);
        // }
        // prev->unlock();
        // Alarm::delay(1500*(i+1)); // COM ESTE DELAY, TRAVA SEM IR ADIANTE A PARTIR DAQUI

        for(volatile int f = 0; f < 0xFFFFFF; f++ );

        cout << "resource::finished_computation() | Thread " << id << " at "    //<< sem->owner()->criterion().current_queue()
                                                                                << " " << CPU::id()
                                                                                << " " << prev->criterion().queue() << endl;
        sem->v();
        cout << "resource::exited() | Thread " << id << endl;
        
        Periodic_Thread::wait_next();
        solo_thread->join();
        delete solo_thread;
    }
    return 0;
}

int resource_shared(Semaphore_Type *sem, int id, int iterations)
{
    for (int i = 0; i < iterations; i++)
    {
        cout << "shared_resource::try_access | Thread " << id << endl;
        sem->p();

        cout << "shared_resource::get_access() | Thread " << id << " at " << CPU::id() << endl;
        Alarm::delay(1500*(i+1));
        cout << "shared_resource::finished_computation() | Thread " << id << " at " << CPU::id() << endl;

        sem->v();
        cout << "shared_resource::exited() | Thread " << id << endl;
        Periodic_Thread::wait_next();
    }
    return 0;
}

void thread_creator(int j)
{    
    int iterations = 1;
    int localCeiling[j];
    for(int k = 0; k < j; k++)
        localCeiling[k] = 100000*(k+1);

    Semaphore_Type * sem = new Semaphore_Type(localCeiling);//Semaphore_Type(1);//
    Periodic_Thread *threads[j];

    for (int i = 0; i < j; i++)
    {
        Periodic_Thread::Configuration config = Periodic_Thread::Configuration(100000*(i+1), 100000*(i+1), 1500*(i+1), Periodic_Thread::NOW, iterations, i, Periodic_Thread::READY, Periodic_Thread::Criterion(100000*(i+1), 100000*(i+1), 1500*(i+1), i));
        if(i == 0)
            threads[i] = new Periodic_Thread(config, &resource_preempted, sem, i, iterations);
        else
            threads[i] = new Periodic_Thread(config, &resource_shared, sem, i, iterations);
    }

    for (int i = 0; i < j; i++)
        threads[i]->join();

    for (int i = 0; i < j; i++)
        delete threads[i];

    delete sem;
}

int main()
{
    cout << "Starting Semaphore Wait benchmark test..." << endl;

    // int N_T = 2;

    // for (int i = 1; i <= N_T; i++)
    // {
        cout << "\n---- SEMAPHORE WITH " << 2 << " THREADS ----\n";
        thread_creator(2);
    // }

    cout << "simulation ended" << endl;
    /*while (1)
    {
        Alarm::delay(1000000);
        cout << "simulation ended" << endl;
    }*/
    return 0;
}