#include <utility/ostream.h>
#include <time.h>
#include <real-time.h>
#include <synchronizer.h>

using namespace EPOS;
typedef Semaphore_MrsP Semaphore_Type;
OStream cout;
Chronometer chrono;

int resource_preempter()
{
    Microsecond elapsed = chrono.read() / 1000;
    for(Microsecond end = elapsed + 1000; end > elapsed; elapsed = chrono.read() / 1000);
}

/*
    Description: nothing happens, threads that run this routine never change cores
    Calling reschedule (IC::IPI) just by itself doesnt work
*/
int resource_reschedule(Semaphore_Type *sem, int id, int iterations)
{
    Periodic_Thread::Criterion c = Periodic_Thread::Criterion( 99999, 99999, 1500*2, 1 );

    for (int i = 0; i < iterations; i++)
    {
        sem->p();
        cout << "resource::get_access() | Thread " << Thread::self() << " at " << CPU::id() << endl;
        Microsecond elapsed = chrono.read() / 1000;
        for(Microsecond end = elapsed + 500; end > elapsed; elapsed = chrono.read() / 1000);

        Thread * prev = Thread::self();
        kout << "Moving @owner from " << prev->criterion().queue() << " to " << c.queue() << endl;
        prev->lock();
        prev->reschedule(1);

        for(Microsecond end = elapsed + 1000; end > elapsed; elapsed = chrono.read() / 1000);

        cout << "resource::finished_computation() | Thread " << Thread::self() << " at " << " " << CPU::id() << prev->criterion().queue() << endl;
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
int resource_priority(Semaphore_Type *sem, int id, int iterations)
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

void thread_creator(const int th_n, const int iterations, const bool same_core)
{    
    int prio[2] = {100000, 200000};
    Semaphore_Type * sem = new Semaphore_MrsP(prio);

    Periodic_Thread * threads[th_n];
    Periodic_Thread::Configuration config = Periodic_Thread::Configuration(0, 0, 0,
        Periodic_Thread::NOW, 0, 0, Periodic_Thread::READY,
        Periodic_Thread::Criterion(0, 0, 0, 0)
    );

    for (int i = 0; i < th_n; i++)
    {
        // Creates periodic_thread configs
        if(same_core) { // threads created at core 0 if same_core flag is set
            config = Periodic_Thread::Configuration(100000*(i+1), 100000*(i+1), 1500*(i+1),
                Periodic_Thread::NOW, iterations, 0, Periodic_Thread::READY,
                Periodic_Thread::Criterion(100000*(i+1), 100000*(i+1), 1500*(i+1), 0)
            );
            threads[i] = new Periodic_Thread(config, &resource_priority, sem, 0, iterations);
        } else {
            config = Periodic_Thread::Configuration(100000*(i+1), 100000*(i+1), 1500*(i+1),
                Periodic_Thread::NOW, iterations, i, Periodic_Thread::READY,
                Periodic_Thread::Criterion(100000*(i+1), 100000*(i+1), 1500*(i+1), i)
            );
            threads[i] = new Periodic_Thread(config, &resource_priority, sem, i, iterations);
        }

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
    const int n_threads{2};
    const int iterations{1};

    cout << "\n---- SEMAPHORE WITH " << n_threads << " THREADS ----\n";
    thread_creator(n_threads, iterations, true);

    cout << "simulation ended" << endl;
    return 0;
}