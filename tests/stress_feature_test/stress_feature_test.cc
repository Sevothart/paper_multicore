#include <utility/ostream.h>
#include <time.h>
#include <process.h>
#include <clerk.h>
#include <real-time.h>
#include "disparity.h"
//#include "bandwidth.h"
#include "cpu_hungry.h"
#include <utility/random.h>

using namespace EPOS;

OStream cout;

typedef TSC::Time_Stamp Time_Stamp;

// Configuration
const unsigned int  TEST_LENGTH            = 40; // in seconds
const bool          MEASURE_TIME           = false;
// To be measured
const float         MEMORY_IT_LENGHT       = 150;//150;    // 0.4 in microseconds
const float         MEMORY_IT_LENGHT_T8    = 3;
const float         MEMORY_IT_LENGHT_T9    = 4;
const float         MEMORY2_IT_LENGHT      = 1000;
const unsigned int  CPU_IT_LENGHT          = 900; //1000  // in microseconds
const unsigned int  MIDTERM_IT_LENGHT      = 100000;  // in microseconds

constexpr float TIMES[4] = { MEMORY_IT_LENGHT, CPU_IT_LENGHT, MIDTERM_IT_LENGHT, MEMORY2_IT_LENGHT};

const unsigned int THREADS             = 6;//12;//8;//11;

constexpr static struct Task_Set {
    const unsigned int p;
    const unsigned int d;
    const unsigned int c;
    const unsigned int cpu;
    const unsigned int f;
} set[THREADS] = {
    //PERIOD,DEADLINE,WCET,CPU,TASK
    // /* TS1
    {500000, 500000, 100000,2,0},   // 20 - band
    {500000, 500000, 100000,2,2},   // 20 - disp

    {500000, 500000, 100000,3,2},   // 20 - disp
    {500000, 500000, 100000,3,1},   // 20 - cpu

    {500000, 500000, 100000,4,1},   // 20 - cpu
    {500000, 500000, 100000,4,2},   // 20 - disp
    //*/

    /* TS4
    // Band on CPU 1
    {250000,250000,50000,2,0},      // 20 - band

    // Disparity on CPU 2 (Parallel to band on 1)
    {1000000,1000000,200000,3,1},   // 20 - cpu
    {1000000,1000000,200000,3,2},   // 20 - disp
    {500000, 500000, 100000,3,1},   // 20 - disp
    {500000, 500000,  50000,3,0},   // 10 - band

    {500000,500000,100000,4,0},  // 20 - band
    {1000000,1000000,200000,4,2}, // 20 - disp
    //*/
};


/* Think section
 * make thread [0] migrate every 10 executions to the other CPUS, print statistics at each migration
*/
unsigned int migrations_iters = 0;

inline constexpr int calc_iter_per_job(int i) {
    return set[i].c/TIMES[set[i].f];
}

inline constexpr int calc_jobs(int i) {
    return (TEST_LENGTH*1000000)/set[i].p;
}

template<int N>
struct ITER_PER_JOB {
    constexpr ITER_PER_JOB() : value() {
        for (int i = 0; i != N; ++i)
            value[i] = calc_iter_per_job(i); 
    }
    int value[N];
};

constexpr ITER_PER_JOB<THREADS> iter_per_job = ITER_PER_JOB<THREADS>();

template<int N>
struct JOBS {
    constexpr JOBS() : value() {
        for (int i = 0; i != N; ++i)
            value[i] = calc_jobs(i); 
    }
    int value[N];
};
constexpr JOBS<THREADS> jobs = JOBS<THREADS>();

unsigned int *g_mem_ptr[THREADS];           /* unsigned inter to allocated memory region */
const unsigned int CACHE_LINE_SIZE = 64;
const unsigned int DEFAULT_ALLOC_SIZE_KB = 524288;//16384*2;
const unsigned int G_MEM_SIZE = DEFAULT_ALLOC_SIZE_KB;

inline Time_Stamp get_time() {
    return TSC::time_stamp();
}

inline Microsecond us(Time_Stamp ts) {
    return Convert::count2us<Hertz,Time_Stamp, Time_Base>(TSC::frequency(), ts);
}

unsigned int time_job[THREADS][20];

Time_Stamp diff[THREADS];
Time_Stamp wcet[THREADS];

extern signed char img1[];
extern signed char img2[];

signed char* t_img1[THREADS];
signed char* t_img2[THREADS];


unsigned int bench_write(unsigned int id)
{
    register unsigned int i;
    for ( i = 0; i < G_MEM_SIZE/sizeof(unsigned int); i+=(CACHE_LINE_SIZE/sizeof(unsigned int)) ) {
        g_mem_ptr[id][i] += i;
    }
    return G_MEM_SIZE;
}

int disparity(int id)
{
    int rows = 32;
    int cols = 32;
    I2D *imleft, *imright, *retDisparity;

    int WIN_SZ=8, SHIFT=64;

    imleft  = (I2D *) t_img1[id];
    imright = (I2D *) t_img2[id];

    rows = imleft->height;
    cols = imleft->width;

    (void)SHIFT;
    (void)WIN_SZ;

    retDisparity = getDisparity(imleft, imright, WIN_SZ, SHIFT);
    int height = retDisparity->height;
    delete retDisparity;

    return height;
}

template<unsigned int ID>
void run_func() {

    Time_Stamp init = 0;
    Time_Stamp end = 0;

    unsigned int my_iter_per_job = iter_per_job.value[ID];
    unsigned int iterations;
    unsigned int ret;
    unsigned int cpu;

    cpu = CPU::id();
    init = get_time();

    for(iterations = 0; iterations < my_iter_per_job; iterations++) {
        switch(set[ID].f) {
            case 0:
                ret += bench_write(ID);
                break;
            case 1:
                ret += cpu_hungry();
                break;
            default:
                ret += disparity(ID);
                break;
        }
    }
    end = get_time();
    diff[ID] += end - init;
    if ((end - init)/my_iter_per_job > wcet[ID]) {
        wcet[ID] = (end - init)/my_iter_per_job;
    }
}

Thread * threads[THREADS];
/*
template<int ID>
inline void init_threads(Microsecond activation, TSC::Time_Stamp tsc0) {
    cout << ID << ",i=" << jobs.value[ID] << ",ij=" << iter_per_job.value[ID] << endl;
    threads[ID] = new RT_Thread(&run_func<ID>, set[ID].d, set[ID].p, set[ID].c, activation, jobs.value[ID], set[ID].cpu-1);
    init_threads<ID + 1>(activation, tsc0);
};

template<>
inline void init_threads<THREADS>(Microsecond activation, TSC::Time_Stamp tsc0) {}
*/
int freq_control() {
    Hertz clock_base = 1200000000;
    int iters = (TEST_LENGTH*1000000)/1000000; // each 2 hyp
    int count_dvfs = 1;
    for (int i = 0; i < iters; ++i)
    {
        if (!(i % 5) && i > 0) {
            cout << "Iter" << i << " - Clock = " << Machine::clock(1200000000 - (count_dvfs % 7)*100000000) << endl; // " - Keep Alive" << endl;//
            count_dvfs++;
        } else {
            cout << "Iter" << i << " - Keep Alive" << endl;
        }
        Delay(1000000);
    }
    return iters;
}

int main_t(int thread_init, int thread_end, int exec)
{
    cout << "Begin Main, img1=" << sizeof(img1)*sizeof(signed char) << ",g_mem_ptr=" << G_MEM_SIZE << endl;
    cout << "clock["<< CPU::id() <<"]" << Machine::clock() << endl;
    Hertz new_clock = 1200000000;
    cout << "    clock change to" << new_clock << "Hz" << endl;
    cout << "    clock now is = " << Machine::clock(new_clock) << endl;

    Delay(500000);
    cout << "SETUP" << endl;
    unsigned int cpu = 1;
    for (unsigned int i = thread_init; i < thread_end; ++i)
    {
        if (set[i].f == 2) {
            t_img1[i] = new signed char[sizeof(img1)];
            t_img2[i] = new signed char[sizeof(img2)];
            for (int j = 0; j < sizeof(img1); ++j)
            {
                t_img1[i][j] = img1[j];
            }
            for (int j = 0; j < sizeof(img2); ++j)
            {
                t_img2[i][j] = img2[j];
            }
        } else if (set[i].f == 0) {
            g_mem_ptr[i] = new unsigned int[G_MEM_SIZE/sizeof(unsigned int)];
            cout << "g" << i << "=" << g_mem_ptr[i] << endl;
            for (unsigned int j = 0; j < G_MEM_SIZE / sizeof(unsigned int); j++) {
                g_mem_ptr[i][j] = j + j*i;
            }
        }
    }
    cout << "ALLOCATION=0,1;2,3;4,5" << endl;

    Time_Stamp tsc0 = get_time()+Convert::us2count<Time_Stamp, Time_Base>(TSC::frequency(),10000);
    //switch(exec) {
    //    case 0:
            cout << 0 << ",i=" << jobs.value[0] << ",ij=" << iter_per_job.value[0] << endl;
            threads[0] = new RT_Thread(&run_func<0>, set[0].d, set[0].p, set[0].c, 10000, jobs.value[0], set[0].cpu-1);
            cout << 1 << ",i=" << jobs.value[1] << ",ij=" << iter_per_job.value[1] << endl;
            threads[1] = new RT_Thread(&run_func<1>, set[1].d, set[1].p, set[1].c, 10000, jobs.value[1], set[1].cpu-1);
    //        break;
    //    case 1:
            cout << 2 << ",i=" << jobs.value[2] << ",ij=" << iter_per_job.value[2] << endl;
            threads[2] = new RT_Thread(&run_func<2>, set[2].d, set[2].p, set[2].c, 10000, jobs.value[2], set[2].cpu-1);
            cout << 3 << ",i=" << jobs.value[3] << ",ij=" << iter_per_job.value[3] << endl;
            threads[3] = new RT_Thread(&run_func<3>, set[3].d, set[3].p, set[3].c, 10000, jobs.value[3], set[3].cpu-1);
            cout << 4 << ",i=" << jobs.value[4] << ",ij=" << iter_per_job.value[4] << endl;
            threads[4] = new RT_Thread(&run_func<4>, set[4].d, set[4].p, set[4].c, 10000, jobs.value[4], set[4].cpu-1);
    //        break;
    //    default:
            cout << 5 << ",i=" << jobs.value[5] << ",ij=" << iter_per_job.value[5] << endl;
            threads[5] = new RT_Thread(&run_func<5>, set[5].d, set[5].p, set[5].c, 10000, jobs.value[5], set[5].cpu-1);
            //cout << 6 << ",i=" << jobs.value[6] << ",ij=" << iter_per_job.value[6] << endl;
            //threads[6] = new RT_Thread(&run_func<6>, set[6].d, set[6].p, set[6].c, 10000, jobs.value[6], set[6].cpu-1);
            //cout << 6 << ",i=" << jobs.value[6] << ",ij=" << iter_per_job.value[6] << endl;
            //threads[6] = new RT_Thread(&run_func<6>, set[6].d, set[6].p, set[6].c, 10000, jobs.value[6], set[6].cpu-1);
    //        cout << 7 << ",i=" << jobs.value[7] << ",ij=" << iter_per_job.value[7] << endl;
    //        threads[7] = new RT_Thread(&run_func<7>, set[7].d, set[7].p, set[7].c, 10000, jobs.value[7], set[7].cpu-1);
    //        break;
    //}

    Monitor::enable_captures(tsc0);
    cout << "All TASKs created" << ",Time=" << us(tsc0) << endl;
    Thread * freq = new Thread(Thread::Configuration(Thread::READY, Thread::Criterion(1000000, 1000000, 10000, 0)), &freq_control);

    for (int i = thread_init; i < thread_end; ++i)
    {
        threads[i]->join();
        cout << "T["<< i << "]" << endl;
        ///*
        for (unsigned int j = 0; j < TEST_LENGTH*2; ++j)
        {
            cout << "i" << j << "=" << threads[i]->_statistics.thread_monitoring[0][j];
            for (unsigned int k = 1; k < COUNTOF(Traits<Monitor>::PMU_EVENTS)+COUNTOF(Traits<Monitor>::SYSTEM_EVENTS); ++k)
            {
                cout << "," << threads[i]->_statistics.thread_monitoring[k][j];
            }
            cout << "\n";
        }
    }
    freq->join();

    Time_Stamp times = get_time() - tsc0;

    //printing thread ids
    cout << "Returned to application main!" << endl;
    Monitor::disable_captures();
    cout << "Elapsed = " << us(times) << endl;
    cout << "Threads=" << THREADS << endl;

    cout << "-----------------------------------------------------" << endl;
    cout << "...............Threads Timing Behavior..............." << endl;
    cout << "-----------------------------------------------------" << endl;
    for (int i = thread_init; i < thread_end; ++i)
    {
        cout<<"t"<<i<<": "<<reinterpret_cast<unsigned int>(threads[i])<<endl;
        cout << "average execution time [" << i << "]: " << us(diff[i]/(jobs.value[i]*iter_per_job.value[i])) << ", diff: " << us(diff[i]) << endl;
        cout << "iter_per_job=" << iter_per_job.value[i] << "; ";
        cout << "WCET [" << i << "]: " << us(wcet[i]) << endl;
        if(MEASURE_TIME) {
            cout << "TIME_JOB:[";
            for (int j = 0; j < 20; ++j)
            {
                cout << "[" << j << "]" << time_job[i][j] << ",";
            }
            cout << "]" << endl;
        }
    }

    cout << "Threads_end" << endl;
    cout << "ID:" << reinterpret_cast<volatile unsigned int>(Thread::self());
    for (int i = thread_init; i < thread_end; ++i)
    {
        cout << "," << reinterpret_cast<volatile unsigned int>(threads[i]);
        if (set[i].f == 2) {
            delete t_img1[i];
            delete t_img2[i];
        } else if (set[i].f == 0){
            delete g_mem_ptr[i];
        }

        delete threads[i];
    }
    cout << "\n";

    return 0;
}

int main() {
    cout << "BATCH MAIN, help me JESUS!" << endl;
    cout << "Lets call first main! From t=" << 0 << " to t=" << 2 << "!" << endl;
    //main_t(0, 2, 0);
    //Monitor::reset_accounting();
    //cout << "Jesus is among us! Second main now! From t=" << 2 << " to t=" << 5 << "!" << endl;
    //main_t(2, 5, 1);
    //Monitor::reset_accounting();
    //cout << "Jesus is among us! Third main now! From t=" << 5 << " to t=" << THREADS << "!" << endl;
    //main_t(5, THREADS, 2);
    main_t(0,THREADS,0);
    return 0;
}

// Trash can
/*
void calc_iter_per_job() {
    for (int i = 0; i < THREADS; i++)
    {
        if (MEASURE_TIME) {
            jobs[i] = 20;
            if (TS_CONF[i][4] == 2)
                jobs[i] = 5;
            iter_per_job[i] = 1;
        } else {
            //if (i == 19) {
            //    iter_per_job[i] = TS_CONF[i][2]/35000;
            //} else {
                switch(TS_CONF[i][4]) { // [4] -> TASK
                                        // [2] ->  WCET
                    case 0:
                        cout << "0 => " << TS_CONF[i][2] << "*" << MEMORY_IT_LENGHT << " = " << TS_CONF[i][2] * MEMORY_IT_LENGHT <<endl; 
                        iter_per_job[i] = TS_CONF[i][2] * MEMORY_IT_LENGHT; // inverse relation
                        break;
                    case 1:
                        iter_per_job[i] = TS_CONF[i][2]/CPU_IT_LENGHT;
                        break;
                    default:
                        iter_per_job[i] = TS_CONF[i][2]/MIDTERM_IT_LENGHT;
                        break;
            //    }
            }
            if(iter_per_job[i] < 1) {
                iter_per_job[i] = 1;
            }
            jobs[i] = TEST_LENGTH*1000000/TS_CONF[i][0]; // [0] -> Period == Deadline
        }
        cout <<"jobs[" << i << "]: " << jobs[i] << ", Iters :" << iter_per_job[i] << endl;
    }
}

    //                                           thread 9 now has 60% usage
    //{300000, 300000, 120000, 2, 2}, // 36   - 88540 ok (29.51)  + 20 = 49 | 1*89892 (29.7) = 49
    //{300000, 300000, 120000, 3, 1}, // 66   - 2*64189  (42.83)  + 40 = 83 | 2*69231 (45.7) = 85 --> 42 + 20
    //{300000, 300000, 120000, 3, 2}, // 84.5 - 7*36549  (85.281) + 0  = 85 | 7*35068 (81)        --> 
    //{300000, 300000, 240000, 4, 2}, // 72   - 63050    (42.03)  + 10 = 52 | 2*49606 (65.5) --> 98943 -- 33 + 10 TODO TEST  
    //{300000, 300000, 108000, 5, 2}, // 36   - 94841    (31.61)  + 0  = 31 | 1*95081 (31.4)
    //{300000, 300000, 253500, 6, 2}, // 84.5   7*36104  (84.243) + 0  = 84 | 7*34678 (80.1)
    //{300000, 300000, 198000, 7, 2}, // 72     60794    (40.5)   + 20 = 60 | 2*61464 (43) = 60 --> 55 TODO TEST
    //{300000, 300000, 198000, 8, 2}, // 66     2*64591  (43)     + 0  = 40 | 2*67463 (44.5)
    // Bandwidth
    //{300000 ,300000, 150000, 4,0}, // 60 // 50 // 20
    //{300000 ,300000 ,150000, 0,1},  // 20
    //{100000 ,100000 ,50000 ,2,0},
    //{100000 ,100000 ,50000 ,3,0}, // 10
    //{100000 ,100000 ,50000 ,4,0},
    //{100000 ,100000 ,10000 ,6,0},
    //{300000 ,300000 , 45000 ,7,0}, // 15 // 20
    // CPU Affinity
    //{300000, 300000, 100000, 4, 1},
    //{300000, 300000, 100000, 3, 1}
    //{100000 ,100000 ,40000 ,8,0},
    // {100000 ,100000 ,20000 ,1,1},
    // {200000 ,200000 ,70000 ,2,1},
    // {100000 ,100000 ,10000 ,3,1},
    // {200000 ,200000 ,10000 ,4,1},
    // {200000 ,200000 ,30000 ,5,1},
    // {100000 ,100000 ,30000 ,6,1},
    // {100000 ,100000 ,30000 ,7,1},
    // {200000 ,200000 ,50000 ,8,1},
    // {1000000,1000000,140000,1,2},
    // {1000000,1000000,280000,2,2},
    // {1500000,1500000,210000,3,2},
    // {2000000,2000000,350000,4,2},
    // {1000000,1000000,350000,5,2},
    // {1000000,1000000,240000,6,2},
    // {1500000,1500000,350000,7,2},
    // {2000000,2000000,350000,8,2}


int main()
{
    cout << "Begin Main, img1=" << sizeof(img1)*sizeof(signed char) << ",g_mem_ptr=" << G_MEM_SIZE << endl;
    cout << "clock["<< CPU::id() <<"]" << Machine::clock() << endl;
    Hertz new_clock = 1200000000;
    cout << "    clock change to" << new_clock << "Hz" << endl;
    cout << "    clock now is = " << Machine::clock(new_clock) << endl;

    Delay(500000);
    cout << "SETUP" << endl;

    for (unsigned int i = 0; i < THREADS; ++i)
    {
        if (set[i].f == 2) {
            t_img1[i] = new signed char[sizeof(img1)];
            t_img2[i] = new signed char[sizeof(img2)];
            for (int j = 0; j < sizeof(img1); ++j)
            {
                t_img1[i][j] = img1[j];
            }
            for (int j = 0; j < sizeof(img2); ++j)
            {
                t_img2[i][j] = img2[j];
            }
        } else if (set[i].f == 0) {
            g_mem_ptr[i] = new unsigned int[G_MEM_SIZE/sizeof(unsigned int)];
            cout << "g" << i << "=" << g_mem_ptr[i] << endl;
            for (unsigned int j = 0; j < G_MEM_SIZE / sizeof(unsigned int); j++) {
                g_mem_ptr[i][j] = j + j*i;
            }
        }
    }

    Time_Stamp tsc0 = get_time()+Convert::us2count<Time_Stamp, Time_Base>(TSC::frequency(),10000);

    init_threads<0>(10000, us(tsc0));

    Monitor::enable_captures(tsc0);
    //Monitor::disable_captures();
    cout << "All TASKs created" << ",Time=" << us(tsc0) << endl;
    Thread * freq = new Thread(Thread::Configuration(Thread::READY, Thread::Criterion(400000, 400000, 10000, 0)), &freq_control);

    for (int i = 0; i < THREADS; ++i)
    {
        threads[i]->join();
        cout << "AVG-Monitor:" << Convert::count2us<Hertz, TSC::Time_Stamp, Time_Base>(TSC::frequency(), (threads[i]->_statistics.average_execution_time/threads[i]->_statistics.jobs)) << endl;
    }
    freq->join();

    Time_Stamp times = get_time() - tsc0;

    //printing thread ids
    cout << "Returned to application main!" << endl;
    Monitor::disable_captures();
    cout << "Elapsed = " << us(times) << endl;
    cout << "Threads=" << THREADS << endl;

    cout << "-----------------------------------------------------" << endl;
    cout << "...............Threads Timing Behavior..............." << endl;
    cout << "-----------------------------------------------------" << endl;
    for (int i = 0; i < THREADS; ++i)
    {
        cout<<"t"<<i<<": "<<reinterpret_cast<unsigned int>(threads[i])<<endl;
        cout << "average execution time [" << i << "]: " << us(diff[i]/(jobs.value[i]*iter_per_job.value[i])) << ", diff: " << us(diff[i]) << endl;
        cout << "iter_per_job=" << iter_per_job.value[i] << "; ";
        cout << "WCET [" << i << "]: " << us(wcet[i]) << endl;
        if(MEASURE_TIME) {
            cout << "TIME_JOB:[";
            for (int j = 0; j < 20; ++j)
            {
                cout << "[" << j << "]" << time_job[i][j] << ",";
            }
            cout << "]" << endl;
        }
    }

    cout << "Threads_end" << endl;
    cout << "ID:" << reinterpret_cast<volatile unsigned int>(Thread::self());
    for (int i = 0; i < THREADS; ++i)
    {
        cout << "," << reinterpret_cast<volatile unsigned int>(threads[i]);
    }
    cout << "\n";

    return 0;
}


*/