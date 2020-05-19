#include <utility/ostream.h>
#include <time.h>
#include <process.h>
#include <synchronizer.h>
#include <semaphore.h>

using namespace EPOS;
OStream cout;
Mutex entry;

int recurso(Semaphore_MPCP<false> * sem)
{
    cout << "P_T" << endl;
    sem->p();

    Alarm::delay(2000000);

    cout << "V_T" << endl;
    sem->v();
    return 0;
}

int main() {
    entry.lock();

    cout << "Starting Semaphore_MPCP benchmark test..." << endl;
    Semaphore_MPCP<false> * sem = new Semaphore_MPCP<false>(2);

    cout << "Creating threads..." << endl;
    Thread * t1 = new Thread( Thread::Configuration(Thread::State::READY,  2), &recurso, sem);
    Thread * t2 = new Thread( Thread::Configuration(Thread::State::READY,  5), &recurso, sem);
    Thread * t3 = new Thread( Thread::Configuration(Thread::State::READY, 10), &recurso, sem);

    entry.unlock();

    cout << "Joining threads..." << endl;
    t1->join();
    t2->join();
    t3->join();

    cout << "Deleting threads..." << endl;
    delete t1;
    delete t2;
    delete t3;

    cout << "Ending Semaphore_MPCP benchmark test..." << endl;
    return 0;
}