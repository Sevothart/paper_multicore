#include <utility/ostream.h>
#include <time.h>
#include <real-time.h>
#include <synchronizer.h>

using namespace EPOS;
typedef Semaphore_MrsP Semaphore_Type;
OStream cout;
Chronometer chrono;

int resource_preempter(Semaphore_Type *sem, int iterations)
{
    Microsecond elapsed;

    for (int i = 0; i < iterations; i++)
    {
        sem->p();|
        elapsed = chrono.read() / 1000;
        for(Microsecond end = elapsed + 1000; end > elapsed; elapsed = chrono.read() / 1000);
        sem->v();
        Periodic_Thread::wait_next();
    }
    return 0;
}

/*
    Description: Works (thread is able to migrate from core a to b)
    when more than one thread are running in the same core in that instant.
    If a thread does not have another thread in the same core to switch context
    it seems to switch context with main thread and application finished (Why not idle?).
*/
int resource_priority(Semaphore_Type *sem, int iterations)
{
    Microsecond elapsed;
    Periodic_Thread::Criterion c = Periodic_Thread::Criterion( 99999, 99999, 1500*2, 1 );

    for (int i = 0; i < iterations; i++)
    {
        sem->p();
        Thread * self = Thread::self();

        cout << "resource::get_access() | Thread " << self << " at " << CPU::id()
            << " and queue " << self->criterion().queue() << endl;

        elapsed = chrono.read() / 1000;
        for(Microsecond end = elapsed + 1000; end > elapsed; elapsed = chrono.read() / 1000);

        
        kout << "Moving @owner from " << self->criterion().queue() << " to " << c.queue() << endl;
        self->priority(c);

        for(Microsecond end = elapsed + 1000; end > elapsed; elapsed = chrono.read() / 1000);

        cout << "resource::finished_computation() | Thread " << Thread::self() << " at id " << CPU::id()
            << " and queue " << self->criterion().queue() << endl;

        sem->v();
        Periodic_Thread::wait_next();
    }
    return 0;
}


void thread_creator(int iterations)
{
    // setup variables
    const int th_n{2};
    Periodic_Thread * threads[th_n];
    int prio[th_n] = {100000, 200000};//, 300000, 300000};
    Semaphore_Type * sem = new Semaphore_MrsP(prio);
    int core = 0;

    Periodic_Thread::Configuration config = Periodic_Thread::Configuration(100000, 100000, 1500,
        Periodic_Thread::NOW, iterations, core, Periodic_Thread::READY,
        Periodic_Thread::Criterion(100000, 100000, 1500, core)
    );
    threads[0] = new Periodic_Thread(config, &resource_priority, sem, iterations);

    config = Periodic_Thread::Configuration(200000, 200000, 1500,
        Periodic_Thread::NOW, iterations, core, Periodic_Thread::READY,
        Periodic_Thread::Criterion(200000, 200000, 1500, core)
    );
    threads[1] = new Periodic_Thread(config, &resource_priority, sem, iterations);

    for(int i = 0; i < th_n; ++i)
    {        
        kout << "Thread " << threads[i] <<  " created at queue " << threads[i]->criterion().queue() << endl;
    }

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
    const int iterations{1}; // number of threads
    thread_creator(iterations);

    cout << "simulation ended" << endl;
    return 0;
}