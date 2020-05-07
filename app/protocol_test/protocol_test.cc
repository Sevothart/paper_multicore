#include <time.h>
#include <process.h>
#include <synchronizer.h>
#include <semaphore.h>
#include <utility/ostream.h>
#include "instr_timer.h"

using namespace EPOS;
OStream cout;

const unsigned int iterations = 10; //1024
const unsigned int sem_val = 10; //1000
unsigned long long p_total = 0, v_total = 0;

template< typename S >
void testSemaphore(S & sem, const char * name) {
    unsigned int p_t, v_t, p_worst = 0, v_worst = 0;
    
    for(unsigned long i = 0; i < iterations; i++)
    {
        ITimer tmr_p;
        for(int j = 0; j < sem_val; j++) 
            sem.p();
        p_t = tmr_p.stop( name, &sem );

        ITimer tmr_v;
        for(int j = 0; j < sem_val; j++) 
            sem.v();
        v_t = tmr_v.stop( name, &sem );
        
        p_total += p_t;
        v_total += v_t;
        
        if(p_t > p_worst) p_worst = p_t;
        if(v_t > v_worst) v_worst = v_t;
    }
    
    p_total = p_total >> 10;
    v_total = v_total >> 10;
    
    unsigned long p_avg = p_total;
    unsigned long v_avg = v_total;
    
    cout << "RESULT for semaphore of type " << name << ", PROCURE operation: " << endl;
    cout << "        Worst time: " << p_worst << endl;
    cout << "        Average time: " << p_avg << endl;
    cout << "        Timer ticks per op: " << p_avg / sem_val << endl << endl;
    
    cout << "RESULT for semaphore of type " << name << ", VACATE operation: " << endl;
    cout << "        Worst time: " << v_worst << endl;
    cout << "        Average time: " << v_avg << endl;
    cout << "        Timer ticks per op: " << v_avg / sem_val << endl << endl;
}

void connect_wait() {
    for(int i = 0; i < 15; i++) {
        Alarm::delay(100*1000);
    }
}

int main() {
    cout << "EPOS Semaphore Protocols Benchmark." << endl;
    cout << "Executing " << iterations << " iterations of "<< sem_val <<" OPs each." << endl;     
    //connect_wait();

    /*
    Semaphore s(sem_val);
    testSemaphore<Semaphore>(s, "Semaphore");
    */
    Semaphore_PCP s_pcp(0, sem_val);
    testSemaphore<Semaphore_PCP>(s_pcp, "Semaphore_PCP");
    /*
    Semaphore_IPCP s_ipcp(0, sem_val);
    testSemaphore(s_ipcp, "Semaphore_IPCP");
    */
    return 0;
}