#include <time.h>
#include <process.h>
#include <synchronizer.h>
#include <semaphore.h>

using namespace EPOS;
OStream cout;

Mutex start;

int rotina(int n);
Thread * threads[5];

int main(){
    start.lock();

    typedef Scheduling_Criteria::CPU_Affinity Criterion;
    typedef Scheduling_Criteria::Priority Priority;

    cout << "Starting thread config structs." << endl;
    Thread::Configuration * t1_config = new Thread::Configuration( Thread::READY, Criterion(Thread::LOW, Priority::ANY) );
    Thread::Configuration * t2_config = new Thread::Configuration( Thread::READY, Criterion(Thread::LOW, Priority::ANY) );
    Thread::Configuration * t3_config = new Thread::Configuration( Thread::READY, Criterion(Thread::LOW, 3) );

    cout << "Creating threads." << endl;
    threads[0] = new Thread( *t1_config, &rotina, 0 );
    threads[1] = new Thread( *t1_config, &rotina, 1 );
    threads[2] = new Thread( *t2_config, &rotina, 2 );
    threads[3] = new Thread( *t2_config, &rotina, 3 );
    threads[4] = new Thread( *t3_config, &rotina, 4 );

    cout << "Threads will join now." << endl;
    for( int i=0; i<5; i++ )
        threads[i]->join();

    start.unlock();
    
    cout << "Deleting threads." << endl;    
    for(int i = 0; i < 5; i++)
        delete threads[i];

    cout << "The end!" << endl;
    return 0;
}

int rotina(int n) {
    for(int i = 100; i > 0; i--){
        cout << "Thread " << n << " running on core " << CPU::id() << endl;
        Alarm::delay(50000);
    }
    return 0;
}