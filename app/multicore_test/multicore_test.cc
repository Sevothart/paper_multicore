#include <time.h>
#include <process.h>
#include <semaphore.h>
#include <synchronizer.h>

using namespace EPOS;
OStream cout;

Mutex start;

Semaphore_MPCP<false> s_mpcp(0, 10);

int rotina(int n);
Thread * threads[5];

int main(){
    start.lock();
    Thread::Configuration *config1 = new Thread::Configuration(Thread::State::READY, Scheduling_Criteria::PRM(50, 50, 1, 3));
    Thread::Configuration *config2 = new Thread::Configuration(Thread::State::READY, Scheduling_Criteria::PRM(50, 50, 1, 2));

    cout << "Creating threads." << endl;
    for( int i=0; i<3; i++ )
        threads[i] = new Thread(*config1, &rotina, i);
    for( int i=3; i<5; i++ )
        threads[i] = new Thread(*config2, &rotina, i);

    start.unlock();

    cout << "Threads will join now." << endl;
    for( int i=0; i<5; i++ )
        threads[i]->join();
    
    cout << "Deleting threads." << endl;    
    for(int i = 0; i < 5; i++)
        delete threads[i];

    cout << "The end!" << endl;
    return 0;
}

int rotina(int n) {
    s_mpcp.p();

    for(int i = 100; i > 0; i--){
        cout << "Thread " << n << " running on core " << CPU::id() << endl;
        Alarm::delay(50000);
    }
    cout << "TERMINEI! " << n << endl;
    s_mpcp.v();
    return 0;
}