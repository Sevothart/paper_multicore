#include <utility/ostream.h>
#include <time.h>
#include <process.h>
#include <real-time.h>
#include <synchronizer.h>
#include "inst-timer.h"

using namespace EPOS;
OStream cout;
Mutex entry;

static const int iterations = 1000;
static const int N_T = 1;
volatile int pseudobarrier = 0;

typedef Semaphore_PCP Semaphore_Type;
Semaphore_Type * sem;

int thread_main(int id)
{   
    while(pseudobarrier < N_T)
    {
        pseudobarrier++;
        for(int i = 0; i < iterations; i++) {
            sem->p();
            Thread::yield();
            sem->v();
        }
    }
    return 0;
}

int main()
{   
    cout << "Starting Semaphore Spin benchmark test..." << endl;
    
    sem = new Semaphore_Type( Scheduling_Criteria::PRM::NORMAL ,1);
    Thread * threads[N_T];
    ITimer t;

    for(int i = 0; i < N_T; i++)
        threads[i] = new Thread( thread_main, i );
    
    for(int i = 0; i < N_T; i++)
        threads[i]->join();
    
    for(int i = 0; i < N_T; i++)
        delete threads[i];
    
    t.dataPrint();
    cout << "simulation ended" << endl;
    while (1)
    {
        Alarm::delay(1000000);
        cout << "simulation ended" << endl;
    }
    return 0;
}