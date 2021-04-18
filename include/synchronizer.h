// EPOS Synchronizer Components

#ifndef __synchronizer_h
#define __synchronizer_h

//#include <architecture.h>
#include <utility/spin.h>
#include <utility/handler.h>
#include <utility/queue.h>
#include <process.h>

__BEGIN_SYS

class Synchronizer_Base
{
protected:
    Synchronizer_Base() {}
    ~Synchronizer_Base() { }

    // Atomic operations
    bool tsl(volatile bool & lock) { return CPU::tsl(lock); }
    int finc(volatile int & number) { return CPU::finc(number); }
    int fdec(volatile int & number) { return CPU::fdec(number); }

    // Thread operations
    void begin_atomic() { Thread::lock(); }
    void end_atomic() { Thread::unlock(); }
};
/*
class Synchronizer_Common_Test : public Synchronizer_Base
{
protected:
    typedef Thread::FIFO_Queue Queue;

protected:
    Synchronizer_Common_Test() {}
    ~Synchronizer_Common_Test() { begin_atomic(); wakeup_all(); }

    void sleep() { Thread::sleep(&_queue); }
    void wakeup() { Thread::wakeup(&_queue); }
    void wakeup_all() { Thread::wakeup_all(&_queue); }

protected:
    Queue _queue;
};
*/
template<bool>
class Synchronizer_Common : public Synchronizer_Base
{
protected:
    typedef Thread::Thread_Queue Queue;

protected:
    Synchronizer_Common() {}
    ~Synchronizer_Common() { begin_atomic(); wakeup_all(); }

    void sleep() { Thread::sleep(&_queue); }
    void wakeup() { Thread::wakeup(&_queue); }
    void wakeup_all() { Thread::wakeup_all(&_queue); }

protected:
    Queue _queue;
};

template<>
class Synchronizer_Common<false> : public Synchronizer_Base
{
protected:
    typedef Thread::FIFO_Queue Queue;

protected:
    Synchronizer_Common() {}
    ~Synchronizer_Common() { begin_atomic(); wakeup_all(); }

    void sleep() { Thread::sleep(&_queue); }
    void wakeup() { Thread::wakeup(&_queue); }
    void wakeup_all() { Thread::wakeup_all(&_queue); }

protected:
    Queue _queue;
};

class Mutex: protected Synchronizer_Common<true>
{
public:
    Mutex();
    ~Mutex();

    void lock();
    void unlock();

private:
    volatile bool _locked;
};


class Semaphore: protected Synchronizer_Common<true> {
public:
    Semaphore(int v = 1);
    ~Semaphore();

    void p(bool locked = false);
    void v(bool locked = false);

protected:
    volatile int _value;
};

template<bool T>
class Semaphore_Template: protected Synchronizer_Common<T> {
private:
    typedef Synchronizer_Common<T> Base;
public:
    Semaphore_Template(int v = 1) : _value(v) {}
    ~Semaphore_Template() {}

    void p(bool locked = false) {
        if(!locked)
    	    Base::begin_atomic();
	    // ITimer t;
        if(Base::fdec(_value) < 1) {
            // t.stop("sem_p", this);
            Base::sleep(); // implicit end_atomic()
        }
        else {
            // t.stop("sem_p", this);
            if(!locked)
                Base::end_atomic();
        }
    }
    void v(bool locked = false) {
        if(!locked)
    	    Base::begin_atomic();
        // ITimer t;
        if(Base::finc(_value) < 0) {
            // t.stop("sem_v", this);
            Base::wakeup();  // implicit end_atomic()
        }
        else {
            // t.stop("sem_v", this);
            if(!locked)
                Base::end_atomic();
        }
    }

protected:
    volatile int _value;
};

/*  
    T chooses between FIFO or ranked-priority queue for suspension based. FALSE: FIFO
    Q chooses between spin or semaphore_template. FALSE: Spin
*/
template<bool T, bool Q>
class BaseLock {};

template<>
class BaseLock<true, false>: public Simple_Spin {
public:
    BaseLock(): Simple_Spin() {}
    void p() { this->acquire(); }
    void v() { this->release(); }
};

template<>
class BaseLock<false, false>: public Simple_Spin {
public:
    BaseLock(): Simple_Spin() {}
    void p() { this->acquire(); }
    void v() { this->release(); }
};

template<>
class BaseLock<true, true>: public Semaphore_Template<true> {
private:
    typedef Semaphore_Template<true> Base;
    /*
protected:
    using Base::begin_atomic;
    using Base::end_atomic;
    */
public:
    BaseLock(int v): Semaphore_Template<true>(v) {}
    void p( bool locked = false ) { Base::p(locked); }
    void v( bool locked = false ) { Base::v(locked); }
};

template<>
class BaseLock<false, true>: public Semaphore_Template<false> {
private:
    typedef Semaphore_Template<false> Base;
public:
    BaseLock(int v): Semaphore_Template<false>(v) {}
    void p( bool locked = false ) { Base::p(locked); }
    void v( bool locked = false ) { Base::v(locked); }
};

template<bool T, bool Q>
class Semaphore_RT: public BaseLock<T, Q> {
private:
    typedef BaseLock<T, Q> Base;
public: 
    Semaphore_RT(int v = 1): Base(v), _owner(0), _priority(0){}
    
    typedef Thread Thread_t;
    typedef int Priority_t;
    
    
    Thread_t* owner() { return _owner; }
    void owner(Thread_t* own) { _owner = own; }
    
    Priority_t priority() { return _priority; }
    void priority(Priority_t priority) { _priority = priority; }
    
protected:
    static Thread_t* currentThread() {
        return( reinterpret_cast<Thread_t *>(Thread::self()) );
    }
    
    Thread_t* nextThread() {
        if(Base::_queue.empty())
            return 0;
        else
            return reinterpret_cast< Thread_t *>( Base::_queue.head()->object() );
    }
    
private:
    Thread_t *_owner;
    Priority_t _priority;
};

//Semaphore for Priority Inheritance Protocol 
class Semaphore_PIP: protected Semaphore_RT<true, true> {
private:
    typedef Semaphore_RT<true, true> Base;
public:
    Semaphore_PIP(int value = 1): Base(value) {} 
    void p();
    void v();
};

template<bool T, bool Q>
class Semaphore_Ceiling: public Semaphore_RT<T, Q> {
private:
    typedef Semaphore_RT<T, Q> Base;
protected:
    typedef int Priority_t;
    using Base::begin_atomic;
    using Base::end_atomic;
public:
    Semaphore_Ceiling(){}
    Semaphore_Ceiling(Priority_t ceiling, int value = 1) : Semaphore_RT<T, Q>(value), _cpu(1) { _ceiling[0] = ceiling; }
    Semaphore_Ceiling(int cpu, Priority_t * ceiling, int value = 1) : Semaphore_RT<T, Q>(value), _cpu(cpu), _ceiling(ceiling) {}
    
    Priority_t ceiling( int cpu = 0 ) { return _ceiling[cpu]; }
    void ceiling(Priority_t ceiling, int cpu) { _ceiling[cpu] = ceiling; }
    void ceiling(Priority_t * ceiling) { _ceiling = ceiling; }

private:
    int _cpu;
    Priority_t * _ceiling;
};

template<bool T, bool Q>
class Static_Ceiling: protected Semaphore_Ceiling<T, Q>{
private:
    typedef Semaphore_Ceiling<T, Q> Base;
protected:
    typedef int Priority_t;
public:
    Static_Ceiling(Priority_t ceiling, int value = 1): Semaphore_Ceiling<T, Q>(ceiling, value) {}
    Static_Ceiling(int cpu, Priority_t * ceiling, int value = 1) : Semaphore_Ceiling<T, Q>(cpu, ceiling, value) {}

    virtual void toCeiling() = 0;
};

template<bool T, bool Q>
class Dynamic_Ceiling: public Semaphore_Ceiling<T, Q>{
private:
    typedef Semaphore_RT<T,Q> Base;
protected:
    typedef int Priority_t;
    using Base::begin_atomic;
    using Base::end_atomic;
public:
    Dynamic_Ceiling(Priority_t ceiling, int value = 1): Semaphore_Ceiling<T, Q>(ceiling, value) {}
    Dynamic_Ceiling(int cpu, Priority_t * ceiling, int value = 1): Semaphore_Ceiling<T, Q>(cpu, ceiling, value) {}

    virtual void updateCeiling() = 0;
};

//Semaphore for Immediate Priority Ceiling Protocol 
class Semaphore_IPCP: protected Static_Ceiling<true, true> {
public:
    Semaphore_IPCP(Priority_t ceiling, int value = 1): Static_Ceiling(ceiling, value) {}

    void toCeiling() {
        priority( owner()->priority() );
  	  	owner()->setPriority( ceiling() );
    }
    void p();
    void v();
};

//Semaphore for Priority Ceiling Protocol (classic)
class Semaphore_PCP: protected Semaphore_Ceiling<true, true> {
public:
    Semaphore_PCP(Priority_t ceiling, int value = 1): Semaphore_Ceiling(ceiling, value) {}

    void toCeiling() {
  	  	if( ceiling() < owner()->priority() )
			owner()->setPriority( ceiling() );
    }   
    void p();
    void v();
};

//Semaphore for Stack Resource Police
template<bool T = true>
class Semaphore_SRP: protected Dynamic_Ceiling<T, true> {
private:
    typedef Dynamic_Ceiling<T, true> Base;
protected:
    typedef int Priority_t;
    using Base::begin_atomic;
    using Base::end_atomic;
public:
    
    static const int MAX_TASKS = 20;
    static const int MAX_RESOURCES = 8;
    static const int DEFAULT_CEILING = -2147483647; // minimum 32-bit integer

    Semaphore_SRP(int cpu, Priority_t* ceiling, int value = 1):
        Dynamic_Ceiling<T, true>(cpu, ceiling, value) {}

    Semaphore_SRP(Thread ** tasks, int * levels ,int n_tasks, int value = 1):
        Dynamic_Ceiling<T, true>(0, value) {   
        
        for(int i = 0; i < n_tasks; i++) {
            _tasks[i] = tasks[i];
            _tasks[i]->preemptLevel( levels[i] );
        }

        _nt = n_tasks;

        begin_atomic();
        if(_resources) {
            _resources[_nr] = this;
            _nr++;
        }
        end_atomic();
    }
    
    void p();
    void v();
    
    static int systemCeiling() {return _system_ceiling;}
protected:
    typename Base::Thread_t * _tasks[MAX_TASKS];
    int _nt;
    static Semaphore_SRP* _resources[MAX_RESOURCES];
    static int _nr; 
    static int _system_ceiling;

    void updateCeiling() {
        int maxCeil = DEFAULT_CEILING;

        for(int i = 0; i < _nt; i++) {
            /* If not enough resource and task!=RUNNING, this thread willBlock. */
            if(Base::_value < 1 && _tasks[i]->state() != Thread::State::RUNNING && _tasks[i]->preemptLevel() > maxCeil)
                maxCeil = _tasks[i]->preemptLevel();
        }
        Base::ceiling(maxCeil, 0);
        updateSystemCeiling();
    }
    
    static void updateSystemCeiling() {
        if(!_nr) {
            _system_ceiling = DEFAULT_CEILING;
            return;
        }
        
        _system_ceiling = _resources[0]->ceiling();
        
        for(int i = 1; i < _nr; i++) {
            if(_resources[i]->ceiling() > _system_ceiling)
                _system_ceiling = _resources[i]->ceiling();
        }
    }
public:
    virtual ~Semaphore_SRP() { _resources[--_nr] = 0; }
};

//Semaphore for Multicore Priority Ceiling Protocol
template<bool T>
class Semaphore_MPCP: public Semaphore_IPCP {
public:
    Semaphore_MPCP(Priority_t ceiling = 0, int value = 1): Semaphore_IPCP( ceiling, value ) {}

    void toCeiling() {
        priority( owner()->priority() );
        int p = toGlobalCeiling();
        kout << "Semaphore_MPCP<" << T << ">::toCeiling(): " << priority() << "->" << p << endl;
  	    owner()->setPriority( p );
    }

    int divider(int numero)
    {
        int count = 0, ans = 1, k = 0;
        for(int num=numero; num!=0; num=num/10) {
            if(num%10 != 0)
                k+=1;
            count++;
        }
        count = count - 3 + k;

        for(int n=1; n <= count; n++)
            ans = ans*10;
        return ans;
    }

    Priority_t toGlobalCeiling() { return _pg - ( priority()/divider(priority()) ); }

    void p();
    void v();

private:
    /* _pg = highestPriority in the system that uses global shared resources + 1 */
    static const unsigned int _pg = Traits<Semaphore_MPCP>::highest_priority - 1;
};

template<>
class Semaphore_MPCP<false>: public Semaphore_IPCP {
public:
    Semaphore_MPCP(Priority_t ceiling = 0, int value = 1): Semaphore_IPCP(ceiling, value) {}

    void toCeiling() { Semaphore_IPCP::toCeiling(); } 

    void p() { Semaphore_IPCP::p(); }
    void v() { Semaphore_IPCP::v(); }
};


class Semaphore_MSRP: protected Semaphore_SRP<false> {
private:
    typedef Dynamic_Ceiling<false, true> Base;
public:
    Semaphore_MSRP(Thread ** tasks, int * levels, int n_tasks, int value = 1):
    Semaphore_SRP<false>(Traits<Build>::CPUS, levels, value)
    {
        for(int i = 0; i < n_tasks; i++) {
            _tasks[i] = tasks[i];
            _tasks[i]->preemptLevel( levels[i] );
        }

        _nt = n_tasks;
        begin_atomic();

        if(_globalResources) {
            _globalResources[_nGR] = this;
            _nGR++;
        }
        end_atomic();
    }

    ~Semaphore_MSRP() { _globalResources[--_nGR] = 0; }
    void p();
    void v();
    static int systemCeiling(int cpu){ return _systemCeiling[cpu]; }

protected:
    static const int _cpu = Traits<Build>::CPUS;
    static int _systemCeiling[_cpu];
    static Semaphore_MSRP * _globalResources[MAX_RESOURCES];
    static int _nGR; /* Number of current global resources actives */

    void updateCeiling(){
        int maxCeil;

        for(int k = 0; k < _cpu; k++) {
            maxCeil = DEFAULT_CEILING;

            for(int i = 0; i < _nt; i++) {
                if( (int)_tasks[i]->criterion().queue() == k) { /* Compare only tasks from the same core */
                    if( (_value < 1 && _tasks[i]->state() != Thread::State::RUNNING) && _tasks[i]->preemptLevel() > maxCeil) /* Block condition */
                        maxCeil = _tasks[i]->preemptLevel();
                }
            }
            Base::ceiling(maxCeil, k);
            kout << "Semaphore_MSRP::updateCeiling(): new ceiling->" << maxCeil << " , at core " << k << endl;
        }
        updateSystemCeiling();
    }
    /* This was simplified because updateCeiling already take cares of each core ceiling iteration, this wasnt true for SRP */
    static void updateSystemCeiling(){

        for(int k = 0; k < _cpu; k++) { /* SystemCeiling for each core */

            if(!_nGR) {
                _systemCeiling[k] = DEFAULT_CEILING;
                return;
            }
            
            _systemCeiling[k] = _globalResources[0]->Base::ceiling(k);
            
            for(int i = 1; i < _nGR; i++) { /* Get the highest preemptLevel between all resources for everycore */
                if(_globalResources[i]->Base::ceiling(k) > _systemCeiling[k])
                    _systemCeiling[k] = _globalResources[i]->Base::ceiling(k);
            }
            kout << "Semaphore_MSRP::updateCeiling(): new systemCeiling->" << _systemCeiling[k] << " , at core " << k << endl;
        }
    }
};

// This is actually no Condition Variable
// check http://www.cs.duke.edu/courses/spring01/cps110/slides/sem/sld002.htm
class Condition: protected Synchronizer_Common<true>
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

// An event handler that triggers a semaphore (see handler.h)
class Semaphore_Handler: public Handler
{
public:
    Semaphore_Handler(Semaphore * h) : _handler(h) {}
    ~Semaphore_Handler() {}

    void operator()() { _handler->v(); }

private:
    Semaphore * _handler;
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
        db<Observers, Semaphore>(TRC) << "Concurrent_Observed() => " << this << endl;
    }

    ~Concurrent_Observed() {
        db<Observers, Semaphore>(TRC) << "~Concurrent_Observed(this=" << this << ")" << endl;
    }

    void attach(Concurrent_Observer<D, C> * o, const C & c) {
        db<Observers, Semaphore>(TRC) << "Concurrent_Observed::attach(obs=" << o << ",cond=" << c << ")" << endl;

        o->_link = Element(o, c);
        _observers.insert(&o->_link);
    }

    void detach(Concurrent_Observer<D, C> * o, const C & c) {
        db<Observers, Semaphore>(TRC) << "Concurrent_Observed::detach(obs=" << o << ",cond=" << c << ")" << endl;

        _observers.remove(&o->_link);
    }

    bool notify(const C & c, D * d) {
        bool notified = false;

        db<Observers, Semaphore>(TRC) << "Concurrent_Observed::notify(this=" << this << ",cond=" << c << ")" << endl;

        for(Element * e = _observers.head(); e; e = e->next()) {
            if(e->rank() == c) {
                db<Observers, Semaphore>(INF) << "Observed::notify(this=" << this << ",obs=" << e->object() << ")" << endl;
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
