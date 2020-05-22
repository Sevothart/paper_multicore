// EPOS Semaphore Component Test Program

#include <machine/display.h>
#include <time.h>
#include <synchronizer.h>
#include <semaphore.h>
#include <process.h>

using namespace EPOS;
OStream cout;

const int iterations = 10;

Mutex table;

Thread * phil[5];
Semaphore * chopstick[5];

int philosopher(int n);

int main()
{
    table.lock();
    cout << "AAAAAAAAALLLLLLLLOOOOOOOOOO:" << endl;

    for(int i = 0; i < 5; i++)
        chopstick[i] = new Semaphore;

    phil[0] = new Thread(&philosopher, 0);
    phil[1] = new Thread(&philosopher, 1);
    phil[2] = new Thread(&philosopher, 2);
    phil[3] = new Thread(&philosopher, 3);
    phil[4] = new Thread(&philosopher, 4);

    cout << "Philosophers are alive and hungry!" << endl;

    cout << "The dinner is served ..." << endl;
    table.unlock();

    for(int i = 0; i < 5; i++) {
        int ret = phil[i]->join();
        table.lock();
        cout << "Philosopher " << i << " ate " << ret << " times " << endl;
        table.unlock();
    }

    for(int i = 0; i < 5; i++)
        delete chopstick[i];
    for(int i = 0; i < 5; i++)
        delete phil[i];

    cout << "The end!" << endl;
    return 0;
}

int philosopher(int n)
{
    int first = (n < 4)? n : 0;
    int second = (n < 4)? n + 1 : 4;

    for(int i = iterations; i > 0; i--) {

        table.lock();
        table.unlock();

        Delay thinking(1000000);

        table.lock();
        table.unlock();

        chopstick[first]->p();
        chopstick[second]->p();

        table.lock();
        table.unlock();

        Delay eating(500000);

        table.lock();
        table.unlock();

        chopstick[first]->v();
        chopstick[second]->v();
    }

    table.lock();
    table.unlock();

    return iterations;
}