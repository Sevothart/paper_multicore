#include <utility/ostream.h>
#include <process.h>
#include <clerk.h>
#include <real-time.h>
#include <time.h>
#include <utility/random.h>

using namespace EPOS;

OStream cout;

void run_func() {
    for (int i = 0; i < 250; ++i)
    {
        Random * rand;
        rand->seed(0);
        int fib = 1;
        int temp = 1;
        int prev = 1;
        fib = 1+rand->random()%3;
        //fib = 1;
        prev = 1;
        for (int j = 1; j < 350000; j++) {
            temp = prev+fib;
            prev = fib;
            fib = temp;
        }
    }
}

inline TSC::Time_Stamp get_time() {
    return TSC::time_stamp();
}

Thread * threads[6];

int freq_control() {
    Hertz clock_base = 1200000000;
    int iters = (15*1000000)/1000000; // each 2 hyp
    int count_dvfs = 1;
    for (int i = 0; i < iters; ++i)
    {
        if (!(i % 2) && i > 0) {
            cout << "Iter" << i << " - Clock = " << Machine::clock(1200000000 - (count_dvfs % 7)*100000000) << endl; // " - Keep Alive" << endl;//
            count_dvfs++;
        } else {
            cout << "Iter" << i << " - Keep Alive" << endl;
        }
        Delay(1000000);
    }
    return iters;
}

int main()
{
    cout << "Hello world!" << endl;
    cout << "clock["<< CPU::id() <<"]" << Machine::clock() << endl;
    Hertz new_clock = 1200000000;
    cout << "    clock change to" << new_clock << "Hz" << endl;
    cout << "    clock now is = " << Machine::clock(new_clock) << endl;
    TSC::Time_Stamp tsc0 = get_time()+Convert::us2count<TSC::Time_Stamp, Time_Base>(TSC::frequency(),10000);
    Monitor::enable_captures(tsc0);
    threads[0] = new RT_Thread(&run_func, 500000, 500000, 100000, 10000, 30, 1);
    threads[1] = new RT_Thread(&run_func, 500000, 500000, 100000, 10000, 30, 1);
    threads[2] = new RT_Thread(&run_func, 500000, 500000, 100000, 10000, 30, 2);
    threads[3] = new RT_Thread(&run_func, 500000, 500000, 100000, 10000, 30, 2);
    threads[4] = new RT_Thread(&run_func, 500000, 500000, 100000, 10000, 30, 3);
    threads[5] = new RT_Thread(&run_func, 500000, 500000, 100000, 10000, 30, 3);

    Thread * freq = new Thread(Thread::Configuration(Thread::READY, Thread::Criterion(1000000, 1000000, 10000, 0)), &freq_control);

    for (int i = 0; i < 6; ++i)
    {
        threads[i]->join();
        cout << "T["<< i << "]" << endl;
        ///*
        for (unsigned int j = 0; j < 30; ++j)
        {
            cout << "i" << j << "=" << threads[i]->_statistics.thread_monitoring[0][j];
            for (unsigned int k = 1; k < COUNTOF(Traits<Monitor>::PMU_EVENTS)+COUNTOF(Traits<Monitor>::SYSTEM_EVENTS); ++k)
            {
                cout << "," << threads[i]->_statistics.thread_monitoring[k][j];
            }
            cout << "\n";
        }
        //*/
    }

    freq->join();

    return 0;
}
