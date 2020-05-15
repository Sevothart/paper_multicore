// EPOS Thread Test Program

#include <time.h>
#include <process.h>
#include <utility/ostream.h>
#include <synchronizer.h>
// #include <semaphore.h>

using namespace EPOS;

const int iterations = 100;

int func_a(void);
int func_b(void);

Thread * a;
Thread * b;
Thread * m;

OStream cout;

int main()
{   
    cout << "Thread test" << endl;

    m = Thread::self();

    cout << "I'm the first thread of the first task created in the system." << endl;
    cout << "I'll now create two threads and then wait for them to finish ..." << endl;

    Thread::Configuration *config1 = new Thread::Configuration(Thread::State::READY, Scheduling_Criteria::PRM(50, 50, 1, 1));
    Thread::Configuration *config2 = new Thread::Configuration(Thread::State::READY, Scheduling_Criteria::PRM(50, 50, 1, 2));

    a = new Thread(*config1 ,&func_a);
    b = new Thread(*config2, &func_b);

    int status_a = a->join();
    int status_b = b->join();

    cout << "Thread A exited with status " << status_a
         << " and thread B exited with status " << status_b << "" << endl;

    // WARNING: One of these deletes will break "assert(_state != RUNNING);" on ~Thread()
    delete a;
    delete b;
    delete m;

    cout << "This should not be shown on the display!" << endl;

    return 0;
}

int func_a(void)
{
    for(int i = iterations; i > 0; i--) {
        for(int i = 0; i < 79; i++)
            cout << "a";
        cout << "" << endl << CPU::id() << endl;
        Alarm::delay(500000);
    }

    return 'A';
}

int func_b(void)
{
    for(int i = iterations; i > 0; i--) {
        for(int i = 0; i < 79; i++)
            cout << "b";
        cout << "" << endl << CPU::id() << endl;
        Alarm::delay(500000);
    }

    return 'B';
}
