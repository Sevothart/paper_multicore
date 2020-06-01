// EPOS Real-time Declarations

#ifndef __real_time_h
#define __real_time_h

#include <utility/handler.h>
#include <utility/math.h>
#include <utility/convert.h>
#include <time.h>
#include <process.h>
#include <semaphore.h>

__BEGIN_SYS

// Aperiodic Thread
typedef Thread Aperiodic_Thread;

// Periodic threads are achieved by programming an alarm handler to invoke
// p() on a control semaphore after each job (i.e. task activation). Base
// threads are created in BEGINNING state, so the scheduler won't dispatch
// them before the associate alarm and semaphore are created. The first job
// is dispatched by resume() (thus the _state = SUSPENDED statement)

// Periodic Thread
class Periodic_Thread: public Thread
{
public:
    enum {
        SAME        = Scheduling_Criteria::RT_Common::SAME,
        NOW         = Scheduling_Criteria::RT_Common::NOW,
        UNKNOWN     = Scheduling_Criteria::RT_Common::UNKNOWN,
        ANY         = Scheduling_Criteria::RT_Common::ANY
    };

protected:
    // Alarm Handler for periodic threads under static scheduling policies
    class Static_Handler: public Semaphore_Handler
    {
    public:
        Static_Handler(Semaphore * s, Periodic_Thread * t): Semaphore_Handler(s) {}
        ~Static_Handler() {}
    };

    // Alarm Handler for periodic threads under dynamic scheduling policies
    class Dynamic_Handler: public Semaphore_Handler
    {
    public:
        Dynamic_Handler(Semaphore * s, Periodic_Thread * t): Semaphore_Handler(s), _thread(t) {}
        ~Dynamic_Handler() {}

        void operator()() {
            _thread->criterion().update();

            Semaphore_Handler::operator()();
        }

    private:
        Periodic_Thread * _thread;
    };

    typedef IF<Criterion::dynamic, Dynamic_Handler, Static_Handler>::Result Handler;

public:
    struct Configuration: public Thread::Configuration {
        Configuration(const Microsecond & p, const Microsecond & d = SAME, const Microsecond & cap = UNKNOWN, const Microsecond & act = NOW, const unsigned int n = INFINITE, int cpu_id = ANY, const State & s = READY, const Criterion & c = NORMAL, const Color & a = WHITE, Task * t = 0, unsigned int ss = STACK_SIZE)
        : Thread::Configuration(s, c, a, t, ss), period(p), deadline(d == SAME ? p : d), capacity(cap), activation(act), times(n), cpu(cpu_id) {}

        Microsecond period;
        Microsecond deadline;
        Microsecond capacity;
        Microsecond activation;
        unsigned int times;
        int cpu;
    };

public:
    template<typename ... Tn>
    Periodic_Thread(const Microsecond & p, int (* entry)(Tn ...), Tn ... an)
    : Thread(Thread::Configuration(SUSPENDED, Criterion(p)), entry, an ...),
      _semaphore(0), _handler(&_semaphore, this), _alarm(p, &_handler, INFINITE) { resume(); }

    template<typename ... Tn>
    Periodic_Thread(const Configuration & conf, int (* entry)(Tn ...), Tn ... an)
    : Thread(Thread::Configuration(SUSPENDED, (conf.criterion != NORMAL) ? conf.criterion : Criterion(conf.period), conf.color, conf.task, conf.stack_size), entry, an ...),
      _semaphore(0), _handler(&_semaphore, this), _alarm(conf.period, &_handler, conf.times) {
        if (monitored) {
            if(INARRAY(Traits<Monitor>::SYSTEM_EVENTS, Traits<Monitor>::DEADLINE_MISSES)) {
                _statistics.times_p_count = conf.times;
                _statistics.alarm_times = &_alarm; // will be reconfigured at entry, however the address is still the same
            }
        }
        if((conf.state == READY) || (conf.state == RUNNING)) {
            _state = SUSPENDED;
            resume();
        } else
            _state = conf.state;
    }

    const Microsecond & period() const { return _alarm.period(); }
    void period(const Microsecond & p) { _alarm.period(p); }

    static volatile bool wait_next() {
        Periodic_Thread * t = reinterpret_cast<Periodic_Thread *>(running());
        if(monitored) {
            TSC::Time_Stamp ts = TSC::time_stamp();

            if(INARRAY(Traits<Monitor>::SYSTEM_EVENTS, Traits<Monitor>::THREAD_EXECUTION_TIME)) {
                t->_statistics.execution_time += ts - t->_statistics.last_execution;
                t->_statistics.last_execution = ts; // for deadline misses to account correctly (as they not necessarily inccur in a dispatch)
                t->_statistics.average_execution_time += t->_statistics.execution_time;
                t->_statistics.jobs++;
                t->_statistics.execution_time = 0;
            }

            if(INARRAY(Traits<Monitor>::SYSTEM_EVENTS, Traits<Monitor>::DEADLINE_MISSES))
                t->_statistics.missed_deadlines = t->_statistics.times_p_count - (t->_statistics.alarm_times->_times);
        }

        db<Thread>(TRC) << "Thread::wait_next(this=" << t << ",times=" << t->_alarm._times << ")" << endl;

        if(t->_alarm._times) {
            kout << "BEFORE P WAIT NEXT" << endl;
            t->_semaphore.p();
            kout << "AFTER P WAIT NEXT" << endl;
            if(INARRAY(Traits<Monitor>::SYSTEM_EVENTS, Traits<Monitor>::DEADLINE_MISSES))
                t->_statistics.times_p_count--;
        }

        return t->_alarm._times;
    }

protected:
    Semaphore _semaphore;
    Handler _handler;
    Alarm _alarm;
};

class RT_Thread: public Periodic_Thread
{
public:
    RT_Thread(void (* function)(), const Microsecond & deadline, const Microsecond & period = SAME, const Microsecond & capacity = UNKNOWN, const Microsecond & activation = NOW, int times = INFINITE, int cpu = ANY, const Color & color = WHITE, unsigned int stack_size = STACK_SIZE)
    : Periodic_Thread(Configuration(activation ? activation : period ? period : deadline, deadline, capacity, activation, activation ? 1 : times, cpu, SUSPENDED, Criterion(deadline, period ? period : deadline, capacity, cpu), color, 0, stack_size), &entry, this, function, activation, times) {
        if(monitored) {
            if(INARRAY(Traits<Monitor>::SYSTEM_EVENTS, Traits<Monitor>::THREAD_EXECUTION_TIME) || INARRAY(Traits<Monitor>::SYSTEM_EVENTS, Traits<Monitor>::CPU_EXECUTION_TIME) 
                || INARRAY(Traits<Monitor>::SYSTEM_EVENTS, Traits<Monitor>::CPU_WCET) || INARRAY(Traits<Monitor>::SYSTEM_EVENTS, Traits<Monitor>::THREAD_WCET)) {
                TSC::Time_Stamp ts = TSC::time_stamp();
                if(Thread::_Statistics::last_hyperperiod[_link.rank().queue()] == 0) {
                    Thread::_Statistics::last_hyperperiod[_link.rank().queue()] = ts+Convert::us2count<TSC::Time_Stamp, Time_Base>(TSC::frequency(), activation);
                    db<Thread>(TRC) << "period=" << period << ",hyperperiod=" << Convert::us2count<TSC::Time_Stamp, Time_Base>(TSC::frequency(), period) << endl;
                }
                // GLOBAL Hyperperiod
                if(Thread::_Statistics::hyperperiod[1] == 0) {
                    Thread::_Statistics::hyperperiod[1] = Convert::us2count<TSC::Time_Stamp, Time_Base>(TSC::frequency(), period);
                } else {
                    Thread::_Statistics::hyperperiod[1] = Math::lcm(Thread::_Statistics::hyperperiod[1], Convert::us2count<TSC::Time_Stamp, Time_Base>(TSC::frequency(), period));
                }
                _statistics.wcet = Convert::us2count<TSC::Time_Stamp, Time_Base>(TSC::frequency(), (capacity*100)/period);
                _statistics.last_execution = ts; // updated at dispatch
                _statistics.period = period;
                Thread::_Statistics::wcet_cpu[_link.rank().queue()] += _statistics.wcet;
                db<Thread>(TRC) << "hyperperiod=" << Thread::_Statistics::hyperperiod[1] << ",period=" << period 
                << ",WCET_c=" << Thread::_Statistics::wcet_cpu[_link.rank().queue()] << ",WCET=" << _statistics.wcet << endl;
                if (times != (int) INFINITE)
                    for (unsigned int i = 0; i < COUNTOF(Traits<Monitor>::PMU_EVENTS)+COUNTOF(Traits<Monitor>::SYSTEM_EVENTS); ++i)
                    {
                       _statistics.thread_monitoring[i] = new (SYSTEM) unsigned long long[times];
                    }
            }
        }

        if(activation && Criterion::dynamic)
            // The priority of dynamic criteria will be adjusted to the correct value by the
            // update() in the operator()() of Handler --> update does not change priority for Criterion::PERIODIC
            const_cast<Criterion &>(_link.rank())._priority = Criterion::PERIODIC;
        resume();
    }

private:
    static int entry(RT_Thread * t, void (*function)(), const Microsecond activation, int times) {
        if(activation) {
            // Wait for activation time
            t->_semaphore.p();

            // Adjust alarm period
            t->_alarm.~Alarm();
            new (&t->_alarm) Alarm(t->criterion().period(), &t->_handler, times);
            t->_statistics.times_p_count = times;
            if (Criterion::dynamic)
                const_cast<Criterion &>(t->_link.rank())._priority = Alarm::elapsed() + Alarm::ticks(t->criterion().period()); // should be deadline
        }

        // Periodic execution loop
        do {
//            Alarm::Tick tick;
//            if(Traits<Periodic_Thread>::simulate_capacity && t->criterion()._capacity)
//                tick = Alarm::elapsed() + Alarm::ticks(t->criterion()._capacity);

            // Release job
            function();

//            if(Traits<Periodic_Thread>::simulate_capacity && t->criterion()._capacity)
//                while(Alarm::elapsed() < tick);
        } while (wait_next());

        return 0;
    }
};

typedef Periodic_Thread::Configuration RTConf;

__END_SYS

#endif
