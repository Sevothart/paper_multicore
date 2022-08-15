#include <utility/ostream.h>
#include <time.h>
#include <real-time.h>
#include <synchronizer.h>
#include <architecture.h>

using namespace EPOS;
typedef Semaphore_MrsP Semaphore_Type;
OStream cout;
Chronometer chrono;

bool helper_ready;

int resource_helper(Semaphore_Type *sem, int iterations)
{
    // wait for other threads to take control of the resource
    Microsecond elapsed = chrono.read() / 1000;
    for(Microsecond end = elapsed + 1000; end > elapsed; elapsed = chrono.read() / 1000);
    helper_ready = true;
    // kout << "\tHELPER_READY_SET\n";

    for (int i = 0; i < iterations; i++)
    {
        sem->p();
        Thread * self = Thread::self();

        cout << "resource_helper::get_access() | Thread " << self << " at " << CPU::id()
            << " and queue " << self->criterion().queue() << endl;

        // computation time
        elapsed = chrono.read() / 1000;
        for(Microsecond end = elapsed + 1000; end > elapsed; elapsed = chrono.read() / 1000);

        cout << "resource_helper::finished_computation() | Thread " << Thread::self() << " at id " << CPU::id()
            << " and queue " << self->criterion().queue() << endl;

        sem->v();
        Periodic_Thread::wait_next();
    }
    return 0;
}



int resource_priority(Semaphore_Type *sem, int iterations)
{
    Microsecond elapsed;

    for (int i = 0; i < iterations; i++)
    {
        sem->p();
        Thread * self = Thread::self();

        cout << "resource_priority::get_access() | Thread " << self << " at " << CPU::id()
            << " and queue " << self->criterion().queue() << "\n";

        // computation time
        elapsed = chrono.read() / 1000;
        for(Microsecond end = elapsed + 2000; end > elapsed; elapsed = chrono.read() / 1000);

        cout << "resource_priority::finished_computation() | Thread " << Thread::self() << " at id " << CPU::id()
            << " and queue " << self->criterion().queue() << endl;

        sem->v();
        Periodic_Thread::wait_next();
    }
    return 0;
}

// int resource_preempter(int iterations)
// {
//     // wait for other threads to take control of the resource
    
//     Microsecond elapsed = chrono.read() / 1000;
//     for(Microsecond end = elapsed + 1000; end > elapsed; elapsed = chrono.read() / 1000);
//     // if(!helper_ready) kout << "\tHELPER_NOT_READY\n";
//     // else kout << "\tHELPER_IS_READY\n";

//     for (int i = 0; i < iterations; i++)
//     {
//         Thread * self = Thread::self();

        // bool preempted{false};
        // if( Traits<Semaphore_MrsP>::mrsp_enabled )
        // {
        //     preempted = Semaphore_MrsP::ownerMigrated(Semaphore_MrsP::mrspOwner(), self);
        // }

        // if(preempted)
        // {
        //     kout << "\tOWNER_PREEMPTED\n";
        //     Thread * own = Semaphore_MrsP::mrspOwner();
        //     Periodic_Thread::Criterion c = Periodic_Thread::Criterion( 99999, 99999, 1500*2, Semaphore_MrsP::mrspHelperCPU() );
        //     kout << "Moving @owner from " << own->criterion().queue() << " to " << c.queue() << "\n";
        //     own->priority(c);
        // }
        
//         // computation time
//         elapsed = chrono.read() / 1000;
//         for(Microsecond end = elapsed + 1000; end > elapsed; elapsed = chrono.read() / 1000);

//         cout << "resource_preempter::finished_computation() | Thread " << Thread::self() << " at id " << CPU::id()
//             << " and queue " << self->criterion().queue() << endl;

//         Periodic_Thread::wait_next();
//     }
//     return 0;
// }


void thread_creator(int iterations)
{
    // setup variables
    const int th_n{2};
    Periodic_Thread * threads[th_n];
    int prio[8] = {300000, 200000, 0, 0, 0, 0, 0, 0};
    Semaphore_Type * sem = new Semaphore_MrsP(prio);

    int core = 0; // helper thread
    Periodic_Thread::Configuration config = Periodic_Thread::Configuration(300000, 300000, 2000,
        Periodic_Thread::NOW, iterations, core, Periodic_Thread::READY,
        Periodic_Thread::Criterion(300000, 300000, 2000, core)
    );
    threads[0] = new Periodic_Thread(config, &resource_helper, sem, iterations);

    core = 1; // resource owner thread
    config = Periodic_Thread::Configuration(200000, 200000, 2000,
        Periodic_Thread::NOW, iterations, core, Periodic_Thread::READY,
        Periodic_Thread::Criterion(200000, 200000, 2000, core)
    );
    threads[1] = new Periodic_Thread(config, &resource_priority, sem, iterations);


    // preempter thread
    // config = Periodic_Thread::Configuration(100000, 100000, 2000,
    //     Periodic_Thread::NOW, iterations, core, Periodic_Thread::READY,
    //     Periodic_Thread::Criterion(100000, 100000, 2000, core)
    // );
    // threads[2] = new Periodic_Thread(config, &resource_preempter, iterations);


    // for(int i = 0; i < th_n; ++i)
    // {        
    //     kout << "Thread " << threads[i] <<  " created at queue " << threads[i]->criterion().queue() << endl;
    // }

    chrono.start();
    for (int i = 0; i < th_n; i++)
        threads[i]->join();
    chrono.stop();

    for (int i = 0; i < th_n; i++)
        delete threads[i];
    delete sem;
}

int main()
{
    cout << "STARTING SIMULATION..." << endl;
    const int iterations{1}; // number of threads
    thread_creator(iterations);

    cout << "simulation ended" << endl;
    while (1)
    {
        Alarm::delay(1000000);
        cout << "simulation ended" << endl;
    }
    return 0;
}