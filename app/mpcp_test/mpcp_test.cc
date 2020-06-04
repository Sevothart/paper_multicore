#include <utility/ostream.h>
#include <time.h>
#include <process.h>
#include <real-time.h>
#include <semaphore.h>

using namespace EPOS;
OStream cout;
// Mutex entry;
typedef Semaphore_PCP Semaphore_Type;
int iterations = 5;

int recurso(Semaphore_Type * sem, int id)
{
    cout << "START: " << id << endl;

    for(int i = 0; i < iterations; i++) {
        sem->p();
        cout << "P_T: " << id << " it: " << i << endl;
        Alarm::delay(5000);
        cout << "V_T: " << id << " it: " << i << endl;
        sem->v();
        Periodic_Thread::wait_next();
    }
    
    return 0;
}

int main() {
    // entry.lock();

    cout << "Starting Semaphore_MPCP benchmark test..." << endl;
    Semaphore_Type * sem = new Semaphore_Type(1000000);

    cout << "Creating threads..." << endl;
    Periodic_Thread * t1 = new Periodic_Thread( Periodic_Thread::Configuration(1000000, 1000000, 5000, Periodic_Thread::NOW, iterations), &recurso, sem, 1);
    Periodic_Thread * t2 = new Periodic_Thread( Periodic_Thread::Configuration(2000000, 2000000, 5000, Periodic_Thread::NOW, iterations), &recurso, sem, 2);
    Periodic_Thread * t3 = new Periodic_Thread( Periodic_Thread::Configuration(3000000, 3000000, 5000, Periodic_Thread::NOW, iterations), &recurso, sem, 3);

    // entry.unlock();

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