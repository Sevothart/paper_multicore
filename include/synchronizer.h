// EPOS Synchronizer Components

#ifndef __synchronizer_h
#define __synchronizer_h

#include <architecture.h>
#include <utility/handler.h>
#include <process.h>

__BEGIN_SYS

class Synchronizer_Common
{
protected:
    typedef Thread::Queue Queue;

protected:
    Synchronizer_Common() {}
    ~Synchronizer_Common() { begin_atomic(); wakeup_all(); }

    // Atomic operations
    bool tsl(volatile bool & lock) { return CPU::tsl(lock); }
    int finc(volatile int & number) { return CPU::finc(number); }
    int fdec(volatile int & number) { return CPU::fdec(number); }

    // Thread operations
    void begin_atomic() { Thread::lock(); }
    void end_atomic() { Thread::unlock(); }

    void sleep() { Thread::sleep(&_queue); }
    void wakeup() { Thread::wakeup(&_queue); }
    void wakeup_all() { Thread::wakeup_all(&_queue); }

protected:
    Queue _queue;
};


class Mutex: protected Synchronizer_Common
{
public:
    Mutex();
    ~Mutex();

    void lock();
    void unlock();

private:
    volatile bool _locked;
};

// This is actually no Condition Variable
// check http://www.cs.duke.edu/courses/spring01/cps110/slides/sem/sld002.htm
class Condition: protected Synchronizer_Common
{
public:
    Condition();
    ~Condition();

    void wait();
    void signal();
    void broadcast();
};


// An event handler that triggers a mutex (see handler.h)
class Mutex_Handler: public Handler
{
public:
    Mutex_Handler(Mutex * h) : _handler(h) {}
    ~Mutex_Handler() {}

    void operator()() { _handler->unlock(); }

private:
    Mutex * _handler;
};

// An event handler that triggers a condition variable (see handler.h)
class Condition_Handler: public Handler
{
public:
    Condition_Handler(Condition * h) : _handler(h) {}
    ~Condition_Handler() {}

    void operator()() { _handler->signal(); }

private:
    Condition * _handler;
};

__END_SYS

#endif
