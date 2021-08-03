// EPOS Spin Lock Utility Declarations

#ifndef __simplespin_h
#define __simplespin_h

// #include <architecture.h>
#include <process.h>

__BEGIN_UTIL

// Forwarder to the running thread id
// class This_Thread
// {
// public:
//     static unsigned int id();
//     static void not_booting() { _not_booting = true; }

// private:
//     static bool _not_booting;
// };

// // Recursive Spin Lock
// class Spin
// {
// public:
//     Spin(): _level(0), _owner(0) {}

//     void acquire() {
//         int me = This_Thread::id();

//         while(CPU::cas(_owner, 0, me) != me);

//         db<Spin>(TRC) << "Spin::acquire[this=" << this << ",id=" << hex << me << "]() => {owner=" << _owner << dec << ",level=" << _level << "}" << endl;

//         _level++;
//     }

//     void release() {
//         db<Spin>(TRC) << "Spin::release[this=" << this << "]() => {owner=" << hex << _owner << dec << ",level=" << _level << "}" << endl;

//         if(--_level <= 0) {
//     	    _level = 0;
//             _owner = 0;
//     	}
//     }

//     volatile bool taken() const { return (_owner != 0); }

// private:
//     volatile int _level;
//     volatile int _owner;
// };

// Flat Spin Lock
class Simple_Spin
{
public:
    Simple_Spin(): _locked(false) {}

    void acquire() { 
        bool sync = false;

        while( CPU::tsl(_locked) )
        {
            if(!sync)
            {
                sync = true;
                Thread::unlock();
            }
        }
        if( sync )
            Thread::lock();
        
        kout << "Spin::acquire[SPIN=" << this << "]()" << endl;
    }

    void release() {
//        if(_locked)
            _locked = 0;

        kout << "Spin::release[SPIN=" << this << "]()}" << endl;
    }

private:
    volatile bool _locked;
};

__END_UTIL

#endif
