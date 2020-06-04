// EPOS Semaphore Abstraction Declarations

#ifndef __semaphore_h
#define __semaphore_h

#include <synchronizer.h>

__BEGIN_SYS

class Semaphore: protected Synchronizer_Common
{
public:
    
    static const bool traced = false;
    
    Semaphore(int v = 1);
    ~Semaphore();

    void p();
    void v();

protected:
    volatile int _value;
};

// An event handler that triggers a semaphore (see handler.h)
class Semaphore_Handler: public Handler
{
public:
    Semaphore_Handler(Semaphore * h) : _handler(h) {}
    ~Semaphore_Handler() {}

    void operator()() { kout << "BEFORE SEM_HANDLER\n"; _handler->v(); }

private:
    Semaphore * _handler;
};

class Semaphore_RT: public Semaphore
{
public: 
    Semaphore_RT(int v = 1): Semaphore(v), _owner(0), _priority(0){}
    
    typedef Thread Thread_t;
    typedef int Priority_t;
    
    
    Thread_t* owner() { return _owner; }
    void owner(Thread_t* own) { _owner = own; }
    
    Priority_t priority() { return _priority; }
    void priority(Priority_t priority) { _priority = priority; }
    
protected:
    Thread_t* currentThread()
    {
        return( reinterpret_cast<Thread_t *>(Thread::self()) );
    }
    
    Thread_t* nextThread()
    {
        if(_queue.empty()) return 0;
        return reinterpret_cast< Thread_t *>(_queue.head()->object());
    }
    
private:
    Thread_t *_owner;
    Priority_t _priority;
};

class Semaphore_Ceiling: protected Semaphore_RT
{
public:
    Semaphore_Ceiling(Priority_t ceiling, int value = 1) : Semaphore_RT(value), _ceiling(ceiling) { }
    
    Priority_t ceiling() { return _ceiling; }
    void ceiling(Priority_t ceiling) {_ceiling = ceiling;}

private:
    Priority_t _ceiling;
};

//Semaphore for Immediate Priority Ceiling Protocol 
class Semaphore_IPCP: protected Semaphore_Ceiling
{
public:
    Semaphore_IPCP(Priority_t ceiling, int value = 1):Semaphore_Ceiling(ceiling, value) {
    }
    
    void p();
    void v();

};

//Semaphore for Priority Ceiling Protocol (classic)
class Semaphore_PCP: protected Semaphore_Ceiling
{
public:
    Semaphore_PCP(Priority_t ceiling, int value = 1): Semaphore_Ceiling(ceiling, value) {
    }
    
    void p();
    void v();

};

template<bool T>
class Semaphore_MPCP: public Semaphore_IPCP
{
public:
    Semaphore_MPCP(Priority_t ceiling = 0, int value = 1): Semaphore_IPCP(ceiling, value) {}

    Priority_t toGlobalCeiling() { return _highestPriority + owner()->priority(); }

    void p();
    void v();

private:
    /* Highest priority in the whole system + 1, including every core */
    static const Priority_t _highestPriority = Traits<Semaphore_MPCP<true>>::highest_priority;
};

// Conditional Observer x Conditionally Observed with Data decoupled by a Semaphore
template<typename D, typename C = void>
class Concurrent_Observer;

template<typename D, typename C = void>
class Concurrent_Observed
{
    friend class Concurrent_Observer<D, C>;

private:
    typedef typename Simple_Ordered_List<Concurrent_Observer<D, C>, C>::Element Element;

public:
    typedef D Observed_Data;
    typedef C Observing_Condition;

public:
    Concurrent_Observed() {
        db<Observeds, Semaphore>(TRC) << "Concurrent_Observed() => " << this << endl;
    }

    ~Concurrent_Observed() {
        db<Observeds, Semaphore>(TRC) << "~Concurrent_Observed(this=" << this << ")" << endl;
    }

    void attach(Concurrent_Observer<D, C> * o, const C & c) {
        db<Observeds, Semaphore>(TRC) << "Concurrent_Observed::attach(obs=" << o << ",cond=" << c << ")" << endl;

        o->_link = Element(o, c);
        _observers.insert(&o->_link);
    }

    void detach(Concurrent_Observer<D, C> * o, const C & c) {
        db<Observeds, Semaphore>(TRC) << "Concurrent_Observed::detach(obs=" << o << ",cond=" << c << ")" << endl;

        _observers.remove(&o->_link);
    }

    bool notify(const C & c, D * d) {
        bool notified = false;

        db<Observeds, Semaphore>(TRC) << "Concurrent_Observed::notify(this=" << this << ",cond=" << c << ")" << endl;

        for(Element * e = _observers.head(); e; e = e->next()) {
            if(e->rank() == c) {
                db<Observeds, Semaphore>(INF) << "Observed::notify(this=" << this << ",obs=" << e->object() << ")" << endl;
                e->object()->update(c, d);
                notified = true;
            }
        }

        return notified;
    }

private:
    Simple_Ordered_List<Concurrent_Observer<D, C>, C> _observers;
};

template<typename D, typename C>
class Concurrent_Observer
{
    friend class Concurrent_Observed<D, C>;

public:
    typedef D Observed_Data;
    typedef C Observing_Condition;

public:
    Concurrent_Observer(): _semaphore(0), _link(this) {
        db<Observers>(TRC) << "Observer() => " << this << endl;
    }

    ~Concurrent_Observer() {
        db<Observers>(TRC) << "~Observer(this=" << this << ")" << endl;
    }

    void update(const C & c, D * d) {
        _list.insert(d->lext());
        _semaphore.v();
    }

    D * updated() {
        _semaphore.p();
        return _list.remove()->object();
    }

private:
    Semaphore _semaphore;
    Simple_List<D> _list;
    typename Concurrent_Observed<D, C>::Element _link;
};
__END_SYS

#endif