// EPOS Machine Mediator

#ifndef __clerk_h
#define __clerk_h

#include <utility/convert.h>
#include <architecture.h>
#include <machine.h>
#include <system.h>
#include <time.h>
#include <transducer.h>
#include <process.h>
#include <utility/fann.h>

extern "C" { void __pre_main(); }

__BEGIN_SYS

class Monitor
{
    friend void ::__pre_main(); // for init()
    friend class Thread;        // for init()
    friend class FANN_EPOS;

protected:
    typedef TSC::Time_Stamp Time_Stamp;
    typedef Simple_List<Monitor> List;

    // Anomaly detection parameters
    static const unsigned int STD_DEV_ACCEPTED_DRIFT = 2;         // +/- 2 * std_dev
    static const unsigned int AVERAGE_ACCEPTED_DRIFT = 2;         // +/- 2 * average
    static const unsigned int TIME_ACCEPTED_DRIFT = 2;            // ts difference between captures +/- 2 * frequency
    static const unsigned int MINIMUN_SNAPSHOTS_TO_VALIDATE = 20; // start verification if #SNAPSHOTS greater than this value
public:
    static const unsigned int TOTAL_EVENTS_MONITORED = COUNTOF(Traits<Monitor>::PMU_EVENTS) + COUNTOF(Traits<Monitor>::SYSTEM_EVENTS);

public:
    Monitor(): _captures(0), _t0(TSC::time_stamp()) {}
    virtual ~Monitor() {}

    virtual unsigned int last_capture(unsigned int index) = 0;
    virtual Time_Stamp last_time_stamp() = 0;
    virtual unsigned int get_capture(unsigned int i) = 0;
    virtual Time_Stamp get_capture_ts(unsigned int i) = 0;
    virtual unsigned int captures() = 0;

    virtual void capture() = 0;
    virtual unsigned long long read() = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void reset() = 0;
    virtual void reset_accounting_clerk() = 0;
    virtual void print(OStream & os) const = 0;

    static void run();

    static unsigned int get_ann_out (unsigned int i) {
        if (i < ann_captures[CPU::id()] && i >= 0) {
            return ann_out[CPU::id()][i];
        }
        return 1;
    }

    static unsigned int get_ann_captures() {
        return ann_captures[CPU::id()];
    }

    static void reset_accounting() {
        process_batch();
        for(unsigned int n = 0; n < CPU::cores(); n++) {
            for(List::Iterator it = _monitors[n].begin(); it != _monitors[n].end(); it++) {
                it->object()->reset_accounting_clerk();
            }
        }
        Thread::reset_statistics();
    }

    static void process_batch() {
        disable_captures();
        OStream os; // we are using OStream instead of db to avoid <CPU_ID> print in each line.
        db<Monitor>(TRC) << "Monitor::process_batch()" << endl;
        os << "FINAL_TS<" << count2us(_monitors[0].begin()->object()->time_since_t0()) << ">" << endl;
        os << "ANN-OUT" << endl;
        unsigned int i;

        if (Thread::Criterion::learning) {
            os << "MIGRATION=" << Thread::_Statistics::migration_hyperperiod[0] << "," << Thread::_Statistics::migration_hyperperiod[1] << ","<< Thread::_Statistics::migration_hyperperiod[2] << endl;
            os << "ALLOCATION_M=x;x;x" << endl;
            os << "Votes:" << endl;
            for (unsigned int j = 0; j < Traits<Build>::EXPECTED_SIMULATION_TIME*2; ++j)
            {
                for (i = 1; i < Traits<Build>::CPUS; ++i)
                {
                    os << "voting cpu[" << i << ",i" << j << "]=" << Thread::_Statistics::votes[i][j] 
                         << ",idle=" << Thread::_Statistics::idle_time_vote[i][j] 
                         << ",Overhead=" << Thread::_Statistics::overhead[i][j]
                         << ",variance=" << Thread::_Statistics::max_variance[i][j] << endl;
                }
            }

            os << "Trains:" << endl;
            for (unsigned int j = 0; j < 6; ++j)
            {
                for (i = 1; i < Traits<Build>::CPUS; ++i)
                {
                    os << "train cpu[" << i << ",t" << j << "]=\n";
                    for (unsigned int k = 0; k < 4*2*2; ++k) // TODO threshold at traits
                    {
                        os << Thread::_Statistics::trains_err[i][j][k] << "\n";
                    } 
                }
            }
        }

        os << "begin_data" << endl;
        for(unsigned int n = 0; n < CPU::cores(); n++) {
            i = 0;
            os << "CPU" << n << endl;
            for(List::Iterator it = _monitors[n].begin(); it != _monitors[n].end(); it++) {
                os << "TS," << i << endl;
                os << *it->object();
                i++;
            }
        }
        os << "end_data" << endl;
    }

    // Only start capturing when enters in main (enabled by CPU 0 at the end of init, which is only called at pre_init)
    static void enable_captures(Time_Stamp t0) { 
        // Adjust each Clerk Monitor instantiated.
        for(unsigned int n = 0; n < CPU::cores(); n++) {
            for(List::Iterator it = _monitors[n].begin(); it != _monitors[n].end(); it++) {
                 // Starts t0 with current ts
                it->object()->_t0 = t0;
            }
        }
        _enable = true;
    }

    static bool is_enable() {
        return _enable;
    }

    static void disable_captures() { _enable = false; }

    friend OStream & operator<<(OStream & os, const Monitor & m) {
        m.print(os);
        return os;
    }

protected:
    inline Time_Stamp time_since_t0() { return TSC::time_stamp() - _t0; }

    static inline Time_Stamp us2count(Microsecond t) {
        return Convert::us2count<Time_Stamp, Time_Base>(TSC::frequency(), t);
    }

    static inline Microsecond count2us(Time_Stamp t) {
        return Convert::count2us<Hertz, Time_Stamp, Time_Base>(TSC::frequency(), t);
    }

private:
    template<unsigned int CHANNEL>
    static void init_system_monitoring();

    template<unsigned int CHANNEL>
    static void init_pmu_monitoring();

    static void init();

protected:
    unsigned int _captures;
    Time_Stamp _t0;

private:
    static bool _enable;

public:
    static Simple_List<Monitor> _monitors[Traits<Build>::CPUS];
    static struct FANN_EPOS::fann *ann[Traits<Build>::CPUS];
    static unsigned int ann_out[Traits<Build>::CPUS][100*30];
    static unsigned int ann_captures[Traits<Build>::CPUS];
};


template<typename Clerk>
class Clerk_Monitor: public Monitor
{
    friend class Monitor;

public:
    typedef typename Clerk::Data Data;

    struct Snapshot
    {
        Time_Stamp ts;
        Data data;
    };

public:
    Clerk_Monitor(Clerk * clerk, const Hertz frequency, bool data_to_us = false): _clerk(clerk), _frequency(frequency), _period(us2count((frequency > 0) ? 1000000 / frequency : -1UL)), _last_capture(0), _average(0), _data_to_us(data_to_us), _link(this) {
        db<Clerk>(TRC) << "Clerk_Monitor(clerk=" << clerk << ") => " << this << ")" << endl;
        _snapshots = Traits<Build>::EXPECTED_SIMULATION_TIME * frequency;
        _buffer = new (SYSTEM) Snapshot[_snapshots];

        _monitors[CPU::id()].insert(&_link);
    }
    ~Clerk_Monitor() {
        _monitors[CPU::id()].remove(&_link);
        delete _buffer;
    }

    Snapshot & operator[](unsigned int i) const { return (i < _captures) ? _buffer[i] : _buffer[_captures]; }

    unsigned int captures() { return _captures; }

    unsigned int last_capture(unsigned int index) {
        // handle reset (unsigned long long) 0xffffffff)
        if (index >= COUNTOF(Traits<Monitor>::PMU_EVENTS))
            return (FANN_EPOS::fann_type)_buffer[_captures-1].data;
        if(_captures >= 2) {
            // buffer store a unsigned long long but stores a int
            long long aux = ((unsigned long long)_buffer[_captures-1].data) - _buffer[_captures-2].data;
            if (aux < 0) {
                return ((((unsigned long long) 0xffffffff) - _buffer[_captures-2].data) + _buffer[_captures-1].data);
            }
            return aux;
        }
        return -1;
    }

    Time_Stamp last_time_stamp() {
        if (_captures > 0)
            return _buffer[_captures-1].ts;
        return 0;
    }

    unsigned int get_capture(unsigned int i ) { // fix
        if (i < _captures && i >= 0)
            return _buffer[i].data;
        return 0;
    }

    Time_Stamp get_capture_ts(unsigned int i ) {
        if (i < _captures && i >= 0)
            return _buffer[i].ts;
        return 0;
    }

    void capture() {
        Time_Stamp ts = time_since_t0();
        if(_captures < _snapshots && ((ts - _last_capture) >= _period)) {
            _buffer[_captures].ts = ts;
            _buffer[_captures].data = _clerk->read();
            // a counter reset happens frequently depending on the selected feature and its capacity (32 or 64 bits)
            _average = (_average * _captures + _buffer[_captures].data) / (_captures + 1);
            _captures++;
            _last_capture = ts;
        }
    }

    unsigned long long read() {
        return (unsigned long long) _clerk->read();
    }

    void start() {
        _clerk->start();
    }

    void stop() {
        _clerk->stop();
    }

    void reset() {
        _clerk->reset();
    }

    void reset_accounting_clerk() {
        _captures = 0;
    }

    void print(OStream & os) const {
        if (_data_to_us) {
            for(unsigned int i = 0; i < _captures; i++)
                os << count2us(_buffer[i].ts) << "," << count2us(_buffer[i].data) << endl;
        } else {
            for(unsigned int i = 0; i < _captures; i++)
                os << count2us(_buffer[i].ts) << "," << _buffer[i].data << endl;
        }
    }

    // Validate a snapshot against the collected time series
    bool validate(const Snapshot & s) {
        if(_captures > MINIMUN_SNAPSHOTS_TO_VALIDATE) {
            // Statistic validation
            // Drift is a deviation from a predicted value, a statistical error, that
            // can be measured as standard deviation from the predicted values (v - predicted_v) ^ 2
            // The statistical predictor has a default acceptable value that can be adjusted.
            // If the prediction is the average, the accepted drift may be on 'x' times average...
            // need to always update model (average is updated every capture)
            // This detector is used on level.c, spectrum.c,

            // First verification
            // On a gateway scenario... anomaly detection by delay 'x' times bigger than average delay or expected delay
            // this detector is used on loss.c

            // Second verification

            // Percentage of high (or low) values...
            // if the buffer is filled up, check the number of SNAPSHOTS with value equal to the maximum or equal to 0
            // if the number of repetitions of the max value is bigger than a quarter of the total number of elements
            // there is an anomaly
            // This detector is used on clip.c
            // not implemented (not sure if this is a valid verification here), maybe set a base value (everything greater or lower would be an anomaly)
            // at current event begin, if previous event not succeed, there is an anomaly
            // this detector is used on processtrace.c
            // not implemented (on gateway scenarios, would be great)
            // using dynamic std_dev calc.
            // Welford's algorithm computes the sample variance incrementally.
            // https://stackoverflow.com/questions/5543651/computing-standard-deviation-in-a-stream
            // for each of the captures:
            // delta = cur_value - mean;
            // mean += delta / SNAPSHOTS;
            // M2 += delta * (cur_value - mean);
            // var = M2 / SNAPSHOTS;
            // std = squareRoot(var);
            // with a pre-defined hard-limit and soft limit
            // if curr_value out of (mean +/- std * (hard-limit or limit)): anomaly
            // this detector is used on spike.c

            // Third verification
            // Voter.c analysis the amount of anomalies on a channel (here Clerk_Monitor)
            // if this is greater than number of detectors / 5 (here, detectors are Clerks)

            unsigned int drift;
            unsigned int time_deviation;
            unsigned int delta;
            Data std;

            // first verification
            drift = (s.data - _average);
            drift *= drift;
            if(drift > AVERAGE_ACCEPTED_DRIFT * _average)
                return false;

            // second verification
            if((s.ts - _period) < 0)
                time_deviation = -1 * (s.ts - _period);
            else
                time_deviation = s.ts - _period;
            if(time_deviation > TIME_ACCEPTED_DRIFT * _period)
                return false;

            // third verification
            delta = s.data - _average;
            std = delta*(_average-(delta/_captures));
            std /= _captures;
            std = squareRoot(std);
            if((s.data > (_average + std * STD_DEV_ACCEPTED_DRIFT)) || (s.data < (_average - std * STD_DEV_ACCEPTED_DRIFT)))
                return false;
        }
        return true;
    }

private:
    Clerk * _clerk;
    Hertz _frequency;
    Time_Stamp _period;
    Time_Stamp _last_capture;
    Data _average;
    unsigned int _snapshots;
    Snapshot * _buffer;
    bool _data_to_us;
    List::Element _link;
};


// Transducer Clerk
template<typename T>
class Clerk: private T
{
public:
    using Event = Traits_Tokens::Transducer_Event;
    typedef typename T::Value Data;

public:
    Clerk(const Event event, unsigned int dev, const Hertz frequency = 0, bool monitored = false): _dev(dev), _monitor(monitored ? new (SYSTEM) Clerk_Monitor<Clerk>(this, frequency) : 0) {}
    ~Clerk() {}

    Data read() { return T::sense(_dev); }

    void start() {}
    void stop() {}
    void reset() {}

private:
    unsigned int _dev;
    Clerk_Monitor<Clerk> * _monitor;
};


// System Clerk
template<>
class Clerk<System>
{
public:
    using Event = Traits_Tokens::System_Event;
    typedef unsigned long long Data;

public:
    Clerk(const Event event, const Hertz frequency = 0, bool monitored = false): _event(event), 
        _monitor(monitored ? new (SYSTEM) Clerk_Monitor<Clerk>(this, frequency, event == Event::THREAD_WCET || event == Event::CPU_WCET || event == Event::THREAD_EXECUTION_TIME || event == Event::CPU_EXECUTION_TIME) : 0) {}
    ~Clerk() {}

    Data read() {
        Thread * t = Thread::self();
        switch(_event) {
        case Event::ELAPSED_TIME:
            return Alarm::elapsed();
        case Event::DEADLINE_MISSES:
            return t->_statistics.missed_deadlines;
        case Event::RUNNING_THREAD:
            return reinterpret_cast<volatile unsigned int>(t);
        case Event::THREAD_EXECUTION_TIME:
            if((t->priority() > Thread::Criterion::PERIODIC) && (t->priority() < Thread::Criterion::APERIODIC)) // real-time
                return t->_statistics.jobs ? t->_statistics.average_execution_time / t->_statistics.jobs : t->_statistics.execution_time;
            return t->_statistics.execution_time;
        case Event::CPU_EXECUTION_TIME:
            return Thread::_Statistics::hyperperiod_idle_time[CPU::id()];
        case Event::CPU_FREQUENCY:
            return Machine::frequency();
        case Event::CPU_WCET:
            return Thread::_Statistics::wcet_cpu[CPU::id()]; 
        case Event::THREAD_WCET:
            return t->_statistics.wcet;
        default:
            return 0;
        }
    }

    void start() {}
    void stop() {}
    void reset() {
        Thread* t = Thread::self();
        switch(_event) {
        case Event::ELAPSED_TIME:
            break;
        case Event::DEADLINE_MISSES:
            t->_statistics.missed_deadlines = 0;
            break;
        case Event::RUNNING_THREAD:
            break;
        case Event::THREAD_EXECUTION_TIME:
            t->_statistics.execution_time = 0;
            t->_statistics.jobs = 0;
            t->_statistics.average_execution_time = 0;
            break;
        case Event::CPU_EXECUTION_TIME:
            Thread::_Statistics::idle_time[CPU::id()] = 0;
            Thread::_Statistics::hyperperiod_idle_time[CPU::id()] = 0;
            break;
        default:
            break;
        }
    }

private:
    Event _event;
    Clerk_Monitor<Clerk> * _monitor;
};


#ifdef __PMU_H

// PMU Clerk
template<>
class Clerk<PMU>: private PMU
{
public:
    using PMU::CHANNELS;
    using PMU::EVENTS;
    using PMU::FIXED;
    typedef PMU::Count Data;
    typedef PMU::Event Event;

public:
    Clerk(Event event, const Hertz frequency = 0, bool monitored = false) {
        if((FIXED > 0) && (event < FIXED)) {
            _channel = event;
        } else {
            for(_channel = FIXED; _in_use[CPU::id()][_channel] && _channel < CHANNELS; _channel++);
        }
        if(_channel != CHANNELS) {
            _in_use[CPU::id()][_channel] = true;
            PMU::config(_channel, event);

            if(monitored)
                new (SYSTEM) Clerk_Monitor<Clerk>(this, frequency);
        }
    }

    ~Clerk() {
        if(_channel < CHANNELS) {
            PMU::stop(_channel);
            _in_use[CPU::id()][_channel] = false;
        }
    }

    Data read() { return (_channel < CHANNELS) ? PMU::read(_channel) : 0; }
    void start() { if(_channel < CHANNELS) PMU::start(_channel); }
    void stop() { if(_channel < CHANNELS) PMU::stop(_channel); }
    void reset() { if(_channel < CHANNELS) PMU::reset(_channel); }

private:
    Channel _channel;

    static bool _in_use[Traits<Build>::CPUS][CHANNELS];
};

template<unsigned int CHANNEL>
inline void Monitor::init_pmu_monitoring() {
    if(Traits<Monitor>::PMU_EVENTS_FREQUENCIES[CHANNEL]) {
        if(CPU::id() == 0)
            db<Monitor>(TRC) << "Monitor::init: monitoring PMU event " << Traits<Monitor>::PMU_EVENTS[CHANNEL] << " at " << Traits<Monitor>::PMU_EVENTS_FREQUENCIES[CHANNEL] << " Hz" << endl;
        new (SYSTEM) Clerk<PMU>(Traits<Monitor>::PMU_EVENTS[CHANNEL], Traits<Monitor>::PMU_EVENTS_FREQUENCIES[CHANNEL], true);
    }

    init_pmu_monitoring<CHANNEL + 1>();
};

template<>
inline void Monitor::init_pmu_monitoring<COUNTOF(Traits<Monitor>::PMU_EVENTS)>() {}

#endif

template<unsigned int CHANNEL>
inline void Monitor::init_system_monitoring() {
    if(Traits<Monitor>::SYSTEM_EVENTS_FREQUENCIES[CHANNEL]) {
            db<Monitor>(TRC) << "Monitor::init: monitoring system event " << Traits<Monitor>::SYSTEM_EVENTS[CHANNEL] << " at " << Traits<Monitor>::SYSTEM_EVENTS_FREQUENCIES[CHANNEL] << " Hz" << endl;
        new (SYSTEM) Clerk<System>(Traits<Monitor>::SYSTEM_EVENTS[CHANNEL], Traits<Monitor>::SYSTEM_EVENTS_FREQUENCIES[CHANNEL], true);
    }
    init_system_monitoring<CHANNEL + 1>();
};

template<>
inline void Monitor::init_system_monitoring<COUNTOF(Traits<Monitor>::SYSTEM_EVENTS)>() {}

__END_SYS

#endif
