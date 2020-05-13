// Page coloring test

#include <utility/random.h>
#include <utility/math.h>
#include <machine/display.h>
#include <machine/rtc.h>
#include <time.h>
#include <synchronizer.h>
#include <real-time.h>

using namespace EPOS;

const unsigned int ITERATIONS = 200; // number of times each thread will be executed ~40seg
const unsigned int TEST_REPETITIONS = 1;
const unsigned int THREADS = 17; // number of periodic threads

const unsigned int ARRAY_SIZE = 8 * 1024;
const unsigned int MEMORY_ACCESS = 16384;
const unsigned int WRITE_RATIO = 4;
const unsigned int POLLUTE_BUFFER_SIZE = 16 * 1024;

template<int ID>
void pollute_cache(); // unsigned int repetitions, int id

int run(int test);
void collect_wcet(int test);
void print_stats(void);

template<int ID>
void job(); // unsigned int, int // function passed to each periodic thread

typedef unsigned int us;

// 17 threads, total utilization of 5.972483, 8 processors
constexpr static struct Task_Set {
    void (*f)(); //unsigned int, int
    us p;
    us d;
    us c;
    us affinity;
} set[THREADS] = {
                  // period, deadline, execution time, cpu (partitioned)
                  {&job<0>,  50000,  50000,  32329, 0},
                  {&job<1>,  50000,  50000,   5260, 1},
                  {&job<2>,  50000,  50000,  12295, 2},
                  {&job<3>, 200000, 200000,  62727, 3},
                  {&job<4>, 100000, 100000,  49286, 4},
                  {&job<5>, 200000, 200000,  48083, 5},
                  {&job<6>, 200000, 200000,  22563, 6},
                  {&job<7>, 100000, 100000,  17871, 7},
                  {&job<8>,  25000,  25000,  15211, 1},
                  {&job<9>, 200000, 200000, 129422, 6},
                  {&job<10>, 200000, 200000,  52910, 7},
                  {&job<11>, 100000, 100000,  14359, 5},
                  {&job<12>,  25000,  25000,  14812, 2},
                  {&job<13>,  50000,  50000,  33790, 3},
                  {&job<14>,  25000,  25000,   7064, 5},
                  {&job<15>, 100000, 100000,  20795, 7},
                  {&pollute_cache<16>, 200000, 200000, 42753, 4} //lowest_priority_task*/
};

const unsigned int lowest_priority_task = 16;

typedef struct {
    /*us mean[TEST_REPETITIONS];
    us var[TEST_REPETITIONS];
    us wcet[TEST_REPETITIONS];*/
    us * mean;
    us * var;
    us * wcet;
} wcet_stats;

Display d;
OStream cout;
Clock clock;

volatile wcet_stats * stats;
us exec_time;
static const bool same_color = false;
volatile us * wcet[THREADS];
Thread * threads[THREADS]; // periodic threads that will be created
bool first_run[THREADS];

int main()
{
    d.clear();
    cout << "Page coloring test!" << endl;
    exec_time = 0;

    stats = new wcet_stats[THREADS];

    for(int i = 0; i < THREADS; i++) {
        wcet[i] = new us[ITERATIONS];
        stats[i].mean = new us[TEST_REPETITIONS];
        stats[i].var = new us[TEST_REPETITIONS];
        stats[i].wcet = new us[TEST_REPETITIONS];
    }

    for(int i = 0; i < TEST_REPETITIONS; i++) {
        cout << "Starting test " << i << endl;
        run(i);
    }

    cout << "SDN WFD done!" << endl;
    cout << "Worst-case exec time = " << exec_time / 1000000 << " seconds!" << endl;

    print_stats();

    for(int i = 0; i < THREADS; i++)
        delete wcet[i];
}

int run(int test)
{
    TSC_Chronometer chrono;

    for(int i = 0; i <  THREADS; i++)
        for(int j = 0; j < ITERATIONS; j++)
            wcet[i][j] = 0;

    for(int i = 0; i < THREADS; i++) {
        first_run[i] = true;
        cout << "T[" << i << "] p=" << set[i].p << " d=" << set[i].d << " c=" << set[i].c << " a=" << set[i].affinity << endl;
        if(i == lowest_priority_task)
            // p,d,c,act,t,cpu
            //threads[i] = new Periodic_Thread(RTConf(us(set[i].p), us(set[i].d), us(set[i].c), 0, ITERATIONS, set[i].affinity, Thread::READY, Thread::Criterion(us(set[i].p), us(set[i].d), us(set[i].c), set[i].affinity), Color(i + 1)),
            //                                 set[i].f, (unsigned int)((set[i].c / 1730) * 10), i);
            threads[i] = new RT_Thread(set[i].f, us(set[i].d), us(set[i].p), us(set[i].c), 0, ITERATIONS, set[i].affinity, Color(i + 1));
        else
            //threads[i] = new Periodic_Thread(RTConf(us(set[i].p), us(set[i].d), us(set[i].c), 0, ITERATIONS, set[i].affinity, Thread::READY, Thread::Criterion(us(set[i].p), us(set[i].d), us(set[i].c), set[i].affinity), Color(i + 1)),
            //                                 set[i].f, (i == 12) ? (unsigned int)(set[i].c / 540) : (unsigned int)((set[i].c / 540) * 3), i);
            threads[i] = new RT_Thread(set[i].f, us(set[i].d), us(set[i].p), us(set[i].c), 0, ITERATIONS, set[i].affinity, Color(i + 1));
    }

    chrono.start();

    for(int i = 0; i <  THREADS; i++)
        threads[i]->join();

    chrono.stop();

    collect_wcet(test);

    for(int i = 0; i <  THREADS; i++)
        delete threads[i];

    cout << "Page coloring test " << test << " done in " << chrono.read() / 1000000 << " seconds!\n";
}

void collect_wcet(int test)
{
    for(int i = 0; i < THREADS; i++) {
        us wc, m, var;
        wc = Math::largest(wcet[i], ITERATIONS);
        m = Math::mean(wcet[i], ITERATIONS);
        var = Math::variance(const_cast<us *&>(wcet[i]), ITERATIONS, m);
        stats[i].mean[test] = m;
        stats[i].wcet[test] = wc;
        stats[i].var[test] = var;
    }

    cout << "m = " << stats[0].mean[test] << " wc = " << stats[0].wcet[test] << " var = " << stats[0].var[test] << "\n";
}

void print_stats(void)
{
    for(int i = 0; i < THREADS; i++) {
        us wc, m, var, wc_m, wc_var;
        if(TEST_REPETITIONS > 1) {
            wc = Math::largest(stats[i].wcet, TEST_REPETITIONS);
            wc_m = Math::mean(stats[i].wcet, TEST_REPETITIONS);
            wc_var = Math::variance(stats[i].wcet, TEST_REPETITIONS, wc_m);
            m = Math::mean(stats[i].mean, TEST_REPETITIONS);
            var = Math::mean(stats[i].var, TEST_REPETITIONS);
        } else {
            wc = stats[i].wcet[0];
            wc_m = stats[i].wcet[0];
            wc_var = 0;
            m = stats[i].mean[0];
            var = 0;
        }

        cout << "Thread " << i << " wc = " << wc << " m = " << m << " var = " << var << " wc m = " << wc_m << " wc var = " << wc_var << "\n";
    }
}

template<int ID>
void pollute_cache() //unsigned int repetitions, int id
{
    static Random * rand[THREADS];
    static int *pollute_buffer[THREADS];
    static unsigned int iter[THREADS];

    int id = ID;

    unsigned int repetitions = 0;

    if(id == lowest_priority_task)
        repetitions = (unsigned int)((set[id].c / 1730) * 10);
    else
        repetitions = (id == 12) ? (unsigned int)(set[id].c / 540) : (unsigned int)((set[id].c / 540) * 3);

    TSC_Chronometer c;
    int sum = 0;

    if (first_run[id]) {

        if(same_color)
            rand[id] = new (COLOR_2) Random();
        else
            rand[id] = new Random();

        rand[id]->seed(clock.now() + id);

        if(same_color)
            pollute_buffer[id] = new (COLOR_2) int[POLLUTE_BUFFER_SIZE];
        else
            pollute_buffer[id] = new int[POLLUTE_BUFFER_SIZE];

        iter[id] = 0;

        first_run[id] = false;
    }

    //for(int i = 0; i <  ITERATIONS; i++) {
    //    Periodic_Thread::wait_next();

    c.start();

    for(int j = 0; j < repetitions; j++) {
        for(int k = (rand[id]->random() % (POLLUTE_BUFFER_SIZE - 1) ) % 1000; k < POLLUTE_BUFFER_SIZE; k += 64) {
            pollute_buffer[id][k] = j % 64;
            sum += pollute_buffer[id][k];
        }
    }

    c.stop();

    wcet[id][iter[id]] = c.read();

    //if(wcet[id] < c.read())
    //  wcet[id] = c.read();
    //c.reset();

    iter[id]++;

    //}

    if(iter[id] == ITERATIONS){
        delete rand[id];
        delete pollute_buffer[id];

        cout << "Thread " << id << " done\n";
    }
}

template<int ID>
void job() // unsigned int repetitions, int id
{
    static Random * rand[THREADS];
    static int *array[THREADS];
    static unsigned int iter[THREADS];

    int id = ID;

    unsigned int repetitions =  0;

    if(id == lowest_priority_task)
        repetitions = (unsigned int)((set[id].c / 1730) * 10);
    else
        repetitions = (id == 12) ? (unsigned int)(set[id].c / 540) : (unsigned int)((set[id].c / 540) * 3);

    int sum = 0;
    TSC_Chronometer c;

    if (first_run[id]) {
        if(same_color)
            rand[id] = new (COLOR_2) Random();
        else
            rand[id] = new Random();

        rand[id]->seed(clock.now() + id);

        if(same_color)
            array[id] = new (COLOR_2) int[ARRAY_SIZE];
        else
            array[id] = new int[ARRAY_SIZE];

        iter[id] = 0;

        first_run[id] = false;
    }


    //for(int i = 0; i <  ITERATIONS; i++) {
        //Periodic_Thread::wait_next();
        c.reset();

        //if(id == 0)
        //    cout << "Thread " << id << " ite = " << i << " start\n";

        c.start();

        for(int j = 0; j < repetitions; j++) {
            for(int k = 0; k < MEMORY_ACCESS; k++) {
                int pos = rand[id]->random() % (ARRAY_SIZE - 1);
                sum += array[id][pos];
                if((k % WRITE_RATIO) == 0)
                    array[id][pos] = k + j;
            }
        }

        c.stop();

        wcet[id][iter[id]] = c.read();

        if(id == 0)
            cout << "Thread " << id << " ite = " << iter[id] << " finished in " << c.read() << " wcet[][]=" << wcet[id][iter[id]] << "\n";

        iter[id]++;
        //if(wcet[id] < c.read())
        //  wcet[id] = c.read();
        //c.reset();

    //}

    if (iter[id] == ITERATIONS){
        delete rand[id];
        delete array[id];
    }

    //return sum;
}



