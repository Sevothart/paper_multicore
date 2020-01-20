 
// EPOS Synchronizer Abstraction Test Program

#include <utility/ostream.h>
#include <thread.h>
#include <semaphore.h>
#include <alarm.h>
#include "machine/cortex_m/gpio.h"
#include <chronometer.h>
#include <synchronizer.h>
//#include "../../../hydro/firmware/include/synchronizer.h"

using namespace EPOS;

const unsigned int iterations = 1024;
const unsigned int sem_val = 1000;

unsigned long long p_total = 0, v_total = 0;

OStream cout;

template <typename S>
void testSemaphore(S& sem, const char * name, GPIO & led)
{
    
    unsigned int p_t, v_t, p_worst = 0, v_worst = 0;
    
    const int tmrval = 0x7fffffff;
    
    Cortex_M_GPTM tmr( 2, 0, false, false, false);
    tmr.load(tmrval);
    
    
    led.set(1);
    for(unsigned long i = 0; i < iterations; i++)
    {
        tmr.enable();
        for(int j = 0; j < sem_val; j++) 
            sem.p();
        p_t = tmrval - tmr.read();
        tmr.disable();
        tmr.load(tmrval);
        
        tmr.enable();
        for(int j = 0; j < sem_val; j++) 
            sem.v();
        v_t = tmrval - tmr.read();
        tmr.disable();
        tmr.load(tmrval);
        
        p_total += p_t;
        v_total += v_t;
        
        if(p_t > p_worst) p_worst = p_t;
        if(v_t > v_worst) v_worst = v_t;        
    }
    
    p_total = p_total >> 10;
    v_total = v_total >> 10;
    
    unsigned long p_avg = p_total;
    unsigned long v_avg = v_total;
    
    led.set(0);
    
    cout << "RESULT for semaphore of type " << name << ", PROCURE operation:\n";
    cout << "        Worst time: " << p_worst << "\n";
    cout << "        Average time: " << p_avg << "\n";
    cout << "        Timer ticks per op: " << p_avg / sem_val << "\n\n";
    
    cout << "RESULT for semaphore of type " << name << ", VACATE operation:\n";
    cout << "        Worst time: " << v_worst << "\n";
    cout << "        Average time: " << v_avg << "\n";
    cout << "        Timer ticks per op: " << v_avg / sem_val << "\n\n";
    
};




void connect_wait(GPIO & led)
{
    for(int i = 0; i < 15; i++)
    {
        led.set(1);
        Alarm::delay(100*1000);
        led.set(0);
        Alarm::delay(100*1000);
    }
}

#define TEST(s, x) testSemaphore<x>(s, #x, led)
int main_old()
{
    
    GPIO led('C', 3, GPIO::OUTPUT);    
    connect_wait(led);
    
    cout << "EPOS Semaphore Protocols Benchmark\n Executing " << iterations << " iterations of "<< sem_val <<" OPs each \n\n";    
    //cout << sizeof(unsigned long) << " " << sizeof(unsigned long long) << " " << endl; 
    
    
#ifndef SRP
    Semaphore s(sem_val);
    TEST(s, Semaphore);
    
    Semaphore_PIP s_pip(sem_val);    
    TEST(s_pip, Semaphore_PIP);
    
    Semaphore_PCP s_pcp(0 /*ceiling*/, sem_val);    
    TEST(s_pcp, Semaphore_PCP);
    
    Semaphore_IPCP s_ipcp(0 /*ceiling*/, sem_val);  
    TEST(s_ipcp, Semaphore_IPCP);
#else
    TEST(Semaphore_SRP);
#endif
    
    return 0;
}

static const int N_T = 20;
typedef Semaphore_SRP Semaphore_Type;
Semaphore_Type *res;
volatile int pseudobarrier = 0;
volatile int go = 0;

int thread_main()
{
    kout << "T_S\n";
    
    while(!go);
    
    pseudobarrier++;
    res->p();
    
    // spin a little
    while(pseudobarrier < N_T) Thread::yield();
    
    res->v();
    
    kout << "T_E\n";
    return 0;
}

int main()
{
    GPIO led('C', 3, GPIO::OUTPUT);    
    connect_wait(led);

    kout << "M_S\n";
    
    Thread* threads[N_T];
    threads[0] = new Thread(thread_main);
    Thread* threadList[] = {threads[0],threads[0],threads[0],threads[0],threads[0],threads[0],threads[0],threads[0]};
    
    res = new Semaphore_Type(0, 8, 1);
    
    
    
    // create threads
    led.set(1);
    for(int i = 1; i < N_T; i++)
    {
        threads[i] = new Thread(thread_main);
    }
    led.set(0);
    
    // let them strut their stuff
    go = 1;
    
    //then join
    for(int i = 0; i < N_T; i++)
    {
        threads[i]->join();
    }
    
    kout << "M_E\n";
    
    // test over
    
    return 0;
}