#include <time.h>
#include <process.h>
#include <synchronizer.h>
#include <semaphore.h>
#include <utility/ostream.h>
#include "instr_timer.h"

using namespace EPOS;
OStream cout;

static const int N_T = 3; //20
typedef Semaphore_PCP Semaphore_Type;
Semaphore_Type *res;
volatile int pseudobarrier = 0;

int thread_main()
{
    kout << "T_S\n";
    
    pseudobarrier++;
    res->p();
    
    // spin a little
    while(pseudobarrier < N_T)
        Thread::self()->yield();
    
    res->v();
    
    kout << "T_E\n";
    return 0;
}

int main()
{
    kout << "M_S\n";
    
    res = new Semaphore_Type(1);
    Thread* threads[N_T];
    
    // create threads
    for(int i = 0; i < N_T; i++)
        threads[i] = new Thread(&thread_main);
    
    // let them strut their stuff and then join
    for(int i = 0; i < N_T; i++)
        threads[i]->join();
    
    kout << "M_E\n";
    return 0;
}