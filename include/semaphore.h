// EPOS Semaphore Abstraction Declarations

#ifndef __semaphore_h
#define __semaphore_h

#include <architecture.h>
#include <utility/handler.h>
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

    void operator()() { _handler->v(); }

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

class Semaphore_SRP: protected Semaphore_RT {
public:
    
    static const int MAX_TASKS = 8;
    static const int MAX_RESOURCES = 8;
    static const int DEFAULT_CEILING = -2147483647; // minimum 32-bit integer
    
    Semaphore_SRP(Thread_t ** tasks, int n_tasks, int value = 1):Semaphore_RT(value) 
    {   
        kout << "Semaphore_SRP constructor\n";
        // memcpy failed wtf
        for(int i = 0; i < n_tasks; i++)
        {
            _tasks[i] = tasks[i];
            _prptLevels[i] = 1; // TODO make work for non-binary semaphores
        }
        
        _nt = n_tasks;
        
        begin_atomic();
        // maybe test if full
        _resources[_nr] = this;
        _nr++;
        
        end_atomic();
    }
    
    void p();
    void v();
    
    void setPrptLevels(int * levels)
    {
        for(int i = 0; i < _nt; i++)
        {
            _prptLevels[i] = levels[i];
        }
        //memcpy(_prptLevels, levels, _nt * sizeof(int));
    }
    
    void updateCeiling()
    {
        int maxCeil = DEFAULT_CEILING; //as negative as it gets
    
        for(int i = 0; i < _nt; i++)
        {
            // check if task can can be blocked
            bool willBlock = (_value < _prptLevels[i]); 
            int level = _prptLevels[i];//getPreemptLevel(_tasks[i]);
            if(willBlock && level > maxCeil)
            {
                maxCeil = level;
            }
        }       
        
        _ceiling = maxCeil;
        
        
        updateSystemCeiling();
    }
    
    int ceiling() const { return _ceiling; }
    static int systemCeiling() {return _system_ceiling;}
    
    template <typename X>
    static int getPreemptLevel(X * task)
    {
        return - getDeadline(task->criterion());
    }

private:
    Thread_t * _tasks[MAX_TASKS];
    int _prptLevels[MAX_TASKS];
    int _nt;
    int _ceiling; 
    
    // CLASS MEMBERS
    static Semaphore_SRP* _resources[MAX_RESOURCES];
    static int _nr;
    
    static int _system_ceiling;
    
    static void updateSystemCeiling()
    {
        if(!_nr)
        {
            _system_ceiling = DEFAULT_CEILING;
            return;
        }
        
        _system_ceiling = _resources[0]->_ceiling;
        
        for(int i = 1; i < _nr; i++)
        {
            if(_resources[i]->_ceiling > _system_ceiling)
            {
                _system_ceiling = _resources[i]->_ceiling;
            }
        }
        
    }
    
    template <class C>
    static int getDeadline(const C & criterion) 
    { return 0; }
    
    static int getDeadline(const Scheduling_Criteria::RT_Common & criterion) 
    { return criterion._period; }
    
};

/*
MPCP Documentation: Rajkumar rules
1. Thread normal priority is used out of critical sections.
2. Local critical sections uses IPCP
3. Thread accessing Global critical sections has its priority assigned as: _highest_system_priority + 1 + _priority
4. Jobs on other Global critical section can only preempt if its priority is higher
5. When a job request Global critical section, can only access if there is no owner currently on the section
6. IF Global critical section is empty: Thread is inserted on synchronizer prioritized queue, using its normal_priority
7. On v() operations, if there's any thread on queue head, signal it: p()
*/

/* Used for resources not shared between cores*/
typedef Semaphore_IPCP Semaphore_MPCP_Local;

/* Used for shared resources between cores */
class Semaphore_MPCP_Global: public Semaphore_RT
{
public:
    Semaphore_MPCP_Global(Priority_t h_pri, int value = 1 ): Semaphore_RT( value ), _h_priority(h_pri + 1) {}

    Priority_t toGlobalCeiling() { return _h_priority + owner()->priority(); }
    void p();
    void v();

private:
    /* Highest priority in the whole system + 1, including every core */
    Priority_t _h_priority;
};

// Conditional Observer x Conditionally Observed with Data decoupled by a Semaphore
template<typename D, typename C = int>
class Semaphore_Observer;

template<typename D, typename C = int>
class Semaphore_Observed
{
    friend class Semaphore_Observer<D, C>;
private:
    typedef Semaphore_Observer<D, C> Observer;
    typedef typename Simple_Ordered_List<Semaphore_Observer<D, C>, C>::Element Element;

public:
    typedef C Observing_Condition;

public:
    Semaphore_Observed() {
        db<Observeds, Semaphore>(TRC) << "Semaphore_Observed() => " << this << endl;
    }

    ~Semaphore_Observed() {
        db<Observeds, Semaphore>(TRC) << "~Semaphore_Observed(this=" << this << ")" << endl;
    }

    void attach(Semaphore_Observer<D, C> * o, C c) {
        db<Observeds, Semaphore>(TRC) << "Semaphore_Observed::attach(obs=" << o << ",cond=" << c << ")" << endl;

        o->_link = Element(o, c);
        _observers.insert(&o->_link);
    }

    void detach(Semaphore_Observer<D, C> * o, C c) {
        db<Observeds, Semaphore>(TRC) << "Semaphore_Observed::detach(obs=" << o << ",cond=" << c << ")" << endl;

        _observers.remove(&o->_link);
    }

    bool notify(C c, D * d) {
        bool notified = false;

        db<Observeds, Semaphore>(TRC) << "Semaphore_Observed::notify(this=" << this << ",cond=" << c << ")" << endl;

        for(Element * e = _observers.head(); e; e = e->next()) {
            if(e->rank() == c) {
                db<Observeds, Semaphore>(INF) << "Observed::notify(this=" << this << ",obs=" << e->object() << ")" << endl;
                e->object()->update(this, c, d);
                notified = true;
            }
        }

        return notified;
    }

private:
    Simple_Ordered_List<Semaphore_Observer<D, C>, C> _observers;
};

template<typename D, typename C>
class Semaphore_Observer
{
    friend class Semaphore_Observed<D, C>;

public:
    typedef D Observed_Data;
    typedef C Observing_Condition;

public:
    Semaphore_Observer(): _sem(0), _link(this) {
        db<Observers>(TRC) << "Observer() => " << this << endl;
    }

    ~Semaphore_Observer() {
        db<Observers>(TRC) << "~Observer(this=" << this << ")" << endl;
    }

    void update(C c, D * d) {
        _list.insert(d->lext());
        _sem.v();
    }

    D * updated() {
        _sem.p();
        return _list.remove()->object();
    }

private:
    Semaphore _sem;
    Simple_List<D> _list;
    typename Semaphore_Observed<D, C>::Element _link;
};

__END_SYS

#endif
