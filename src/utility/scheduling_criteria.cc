// EPOS CPU Affinity Scheduler Component Implementation

#include <utility/scheduler.h>
#include <time.h>
#include <clerk.h>
#include <process.h>

__BEGIN_UTIL

// Class attributes
volatile unsigned int Scheduling_Criteria::Variable_Queue::_next_queue;

// The following Scheduling Criteria depend on Alarm, which is not available at scheduler.h
namespace Scheduling_Criteria {
    FCFS::FCFS(int p): Priority((p == IDLE) ? IDLE : Alarm::elapsed()) {}

    EDF::EDF(const Microsecond & d, const Microsecond & p, const Microsecond & c, int): RT_Common(Alarm::ticks(d), Alarm::ticks(d), p, c) {}

    void EDF::update() {
        if((_priority > PERIODIC) && (_priority < APERIODIC))
            _priority = Alarm::elapsed() + _deadline;
    }

    bool Priority::collect() {
        return false;
    }

    
    bool Priority::charge() {
        return false;
    }

    // check ANN result and actuate
    // each cpu stores a result buffer, only one runs voting and clears everything
    bool Priority::award(bool hyperperiod) {//unsigned int cpu) {
        return false;
    }

    bool PEDF::collect() {
        if (learning){
            unsigned int cpu = CPU::id();
            Thread *t;
            double pre_mean;
            for (unsigned int i = 0; i < Thread::_Statistics::t_count_cpu[cpu]; i++)
            {
                t = Thread::_Statistics::threads_cpu[cpu][i];
                if ((t->priority() > PEDF::PERIODIC) && (t->priority() < PEDF::APERIODIC)) { // !idle
                    // Collect all threads data
                    for (unsigned int j = 0; j < COUNTOF(Traits<Monitor>::PMU_EVENTS); ++j)
                    {
                        t->_statistics.input[j] = (t->_statistics.thread_pmu_accumulated[j]*1.0) /(Thread::_Statistics::hyperperiod[1]/100.);
                        t->_statistics.thread_monitoring[j][Thread::_Statistics::hyperperiod_count[cpu]] = t->_statistics.input[j];
                        pre_mean = t->_statistics.mean[j];
                        // mean[n] = mean[n-1] + (x - mean[n-1])/n
                        t->_statistics.mean[j] = pre_mean + (t->_statistics.input[j] - pre_mean)/(t->_statistics.captures+1);
                        // s[n] = s[n-1] + (x - mean[n-1])*(x - mean[n])
                        t->_statistics.variance[j] = t->_statistics.variance[j] + (t->_statistics.input[j] - pre_mean)*(t->_statistics.input[j] - t->_statistics.mean[j]);
                        t->_statistics.thread_pmu_accumulated[j] = 0;
                    }
                    t->_statistics.captures++;
                    t->_statistics.input[COUNTOF(Traits<Monitor>::PMU_EVENTS)+0]
                            = t->_statistics.jobs ? ((t->_statistics.average_execution_time*1.0)/Thread::_Statistics::hyperperiod[1])
                            : ((t->_statistics.execution_time*1.0)/Thread::_Statistics::hyperperiod[1]);
                    t->_statistics.thread_monitoring[COUNTOF(Traits<Monitor>::PMU_EVENTS)+0][Thread::_Statistics::hyperperiod_count[cpu]] = t->_statistics.input[COUNTOF(Traits<Monitor>::PMU_EVENTS)+0] * 10000;
                    //t->_statistics.thread_monitoring[COUNTOF(Traits<Monitor>::PMU_EVENTS)+0][Thread::_Statistics::hyperperiod_count[cpu]] = t->_statistics.jobs ? ((t->_statistics.average_execution_time))*1.0) /Thread::_Statistics::hyperperiod[1])*10000
                    //        : ((t->_statistics.execution_time))/**1.0)/Thread::_Statistics::hyperperiod[1])*10000*/;
                    t->_statistics.input[COUNTOF(Traits<Monitor>::PMU_EVENTS)+1] = Machine::frequency();
                    t->_statistics.thread_monitoring[COUNTOF(Traits<Monitor>::PMU_EVENTS)+1][Thread::_Statistics::hyperperiod_count[cpu]] = t->_statistics.input[COUNTOF(Traits<Monitor>::PMU_EVENTS)+1];
                    t->_statistics.average_execution_time = 0;
                    t->_statistics.jobs = 0;
                }
            }
            return true;
        }
        return false;
    }

    bool PEDF::charge() {
        if (learning) {
            unsigned int cpu = CPU::id();
            if (!cpu)
                return true;
            if (collect()) {
                Thread *t;
                for (unsigned int i = 0; i < Thread::_Statistics::t_count_cpu[cpu]; i++)
                {
                    t = Thread::_Statistics::threads_cpu[cpu][i];
                    if ((t->priority() > PEDF::PERIODIC) && (t->priority() < PEDF::APERIODIC)) { // !idle
                        t->_statistics.output = *FANN_EPOS::fann_run(Monitor::ann[cpu], t->_statistics.input, true);
                        t->_statistics.thread_monitoring[COUNTOF(Traits<Monitor>::PMU_EVENTS)+2][Thread::_Statistics::hyperperiod_count[cpu]] = (int) (t->_statistics.output*10000);
                        db<Thread>(TRC) << "<"
                            << t->_statistics.input[0] << ","
                            << t->_statistics.input[1] << ","
                            << t->_statistics.input[2] << ","
                            << t->_statistics.input[3] << ","
                            << t->_statistics.input[4] << ","
                            << t->_statistics.input[5] << ","
                            << t->_statistics.input[6] << ","
                            << t->_statistics.input[7] << "," 
                            << "out: " << t->_statistics.output << ">" << endl;
                    }
                }
                return true;
            }
        }
        return false;
    }

    bool PEDF::award(bool hyperperiod) {
        static bool imbalanced = false;
        unsigned int cpu = CPU::id();
        if (learning) {
            if (Thread::_Statistics::to_learn[cpu]) {// last hyperperiod was decrease
                if (Thread::_Statistics::cooldown[cpu]) {
                    Thread *t;
                    for (unsigned int i = 0; i < Thread::_Statistics::t_count_cpu[cpu]; i++)
                    {
                        t = Thread::_Statistics::threads_cpu[cpu][i];
                        if ((t->priority() > PEDF::PERIODIC) && (t->priority() < PEDF::APERIODIC)) { // !idle
                            t->_statistics.average_execution_time = 0;
                            t->_statistics.jobs = 0;
                            for(unsigned int j = 0; j < COUNTOF(Traits<Monitor>::PMU_EVENTS); j++) {
                                t->_statistics.thread_pmu_accumulated[j] = 0;
                                // mean[n] = mean[n-1] + (x - mean[n-1])/n
                                t->_statistics.mean[j] = 0;
                                // s[n] = s[n-1] + (x - mean[n-1])*(x - mean[n])
                                t->_statistics.variance[j] = 0;
                                t->_statistics.captures = 0;
                            }
                        }
                        
                    }
                    Thread::_Statistics::cooldown[cpu] = false;
                    return false;
                }
                float aux = 0;
                unsigned int train_count = 0;
                const unsigned int train_limit = 16;
                FANN_EPOS::fann_type *trains[train_limit];
                FANN_EPOS::fann_type desired_output[train_limit];

                Thread *t;
                double pre_mean;
                double value;
                for (unsigned int i = 0; i < Thread::_Statistics::t_count_cpu[cpu]; i++)
                {
                    t = Thread::_Statistics::threads_cpu[cpu][i];
                    if ((t->priority() > PEDF::PERIODIC) && (t->priority() < PEDF::APERIODIC)) { // !idle
                        aux = t->_statistics.jobs ? ((t->_statistics.average_execution_time*1.0)/t->_statistics.jobs)/t->_statistics.period
                                : t->_statistics.execution_time/t->_statistics.period;
                        desired_output[train_count] = aux;
                        trains[train_count] = t->_statistics.input;
                        train_count++;
                        t->_statistics.average_execution_time = 0; //as collect isn't been run, we need to clear them here.
                        t->_statistics.jobs = 0;
                        for(unsigned int j = 0; j < COUNTOF(Traits<Monitor>::PMU_EVENTS); j++) {
                            pre_mean = t->_statistics.mean[j];
                            value = (t->_statistics.thread_pmu_accumulated[j]*1.0)/(t->_statistics.period/100.);
                            // mean[n] = mean[n-1] + (x - mean[n-1])/n
                            t->_statistics.mean[j] = t->_statistics.mean[j] + (value - pre_mean)/(t->_statistics.captures+1);
                            // s[n] = s[n-1] + (x - mean[n-1])*(x - mean[n])
                            t->_statistics.variance[j] = t->_statistics.variance[j] + (value - pre_mean)*(value - t->_statistics.mean[j]);
                            t->_statistics.captures++;
                            t->_statistics.thread_pmu_accumulated[j] = 0;
                        }
                    }
                }
                if (train_count > 0) {
                    FANN_EPOS::fann_reset_MSE(Monitor::ann[cpu]);
                    float error = 0;
                    unsigned int max_train = Traits<PEDF>::MAX_TRAINS;
                    bool end = false;
                    int count = 0;
                    while(!end && max_train) {
                        end = true;
                        for (unsigned int i = 0; i < train_count; ++i)
                        {
                            error = FANN_EPOS::fann_train_data_incremental(Monitor::ann[cpu], trains[i], &desired_output[i]);
                            if (error > Traits<PEDF>::TRAIN_MIN_ERROR) { // TODO traits
                                error = FANN_EPOS::fann_train_data_incremental(Monitor::ann[cpu], trains[i], &desired_output[i]);
                            }
                            end &= error <= Traits<PEDF>::TRAIN_MIN_ERROR;
                            Thread::_Statistics::trains_err[cpu][7 - ((int)(trains[i][7]*6+1))][count] = error;
                            count++;
                            max_train--;
                        }
                        error = FANN_EPOS::fann_train_data_incremental(Monitor::ann[cpu], trains[0], &desired_output[0]);
                        if (error > Traits<PEDF>::TRAIN_MIN_ERROR)
                            end = false;
                    }
                }
                Thread::_Statistics::to_learn[cpu] = false;
            } else if (Thread::_Statistics::cooldown[cpu]) {
                Thread *t;
                for (unsigned int i = 0; i < Thread::_Statistics::t_count_cpu[cpu]; i++)
                {
                    t = Thread::_Statistics::threads_cpu[cpu][i];
                    if ((t->priority() > PEDF::PERIODIC) && (t->priority() < PEDF::APERIODIC)) { // !idle
                        t->_statistics.average_execution_time = 0;
                        t->_statistics.jobs = 0;
                        for(unsigned int j = 0; j < COUNTOF(Traits<Monitor>::PMU_EVENTS); j++) {
                            t->_statistics.thread_pmu_accumulated[j] = 0;
                            // mean[n] = mean[n-1] + (x - mean[n-1])/n
                            t->_statistics.mean[j] = 0;
                            // s[n] = s[n-1] + (x - mean[n-1])*(x - mean[n])
                            t->_statistics.variance[j] = 0;
                            t->_statistics.captures = 0;
                        }
                    }
                }
                Thread::_Statistics::cooldown[cpu] = false;
            } else {
                Thread::_Statistics::prediction_ready[cpu] = false;
                if(cpu && charge()) {
                    FANN_EPOS::fann_type usage = 0;
                    Thread *t;
                    bool error = false;
                    int max_variance_t = 0;
                    double aux = 0;
                    for (unsigned int i = 0; i < Thread::_Statistics::t_count_cpu[cpu]; i++)
                    {
                        t = Thread::_Statistics::threads_cpu[cpu][i];
                        if ((t->priority() > PEDF::PERIODIC) && (t->priority() < PEDF::APERIODIC)) { // !idle
                            usage += t->_statistics.output;
                            for(unsigned int j = 0; j < COUNTOF(Traits<Monitor>::PMU_EVENTS); j++) {
                                aux = t->_statistics.variance[j]/(t->_statistics.mean[j]*t->_statistics.mean[j]);//Math::babylonian_sqrt(t->_statistics.variance[j]/t->_statistics.captures)/t->_statistics.mean[j];
                                if (aux > max_variance_t)
                                    max_variance_t = (int)aux;
                            }
                        }
                        if (t->_statistics.input[6] == 0) {
                            error = true;
                        }
                    }
                    double threshold = 0; // TODO Traits
                    if (max_variance_t < Traits<PEDF>::VARIANCE_RANGES[0]) {
                        threshold = Traits<PEDF>::VARIANCE_THRESHOLDS[0]; // 0.05;
                    } else if (max_variance_t < Traits<PEDF>::VARIANCE_RANGES[1]) {
                        threshold = Traits<PEDF>::VARIANCE_THRESHOLDS[1]; // 0.1
                    } else {
                        threshold = Traits<PEDF>::VARIANCE_THRESHOLDS[2]; // 0.2
                    }
                    Thread::_Statistics::max_variance[cpu][Thread::_Statistics::hyperperiod_count[cpu]] = max_variance_t;

                    // margem para baixar pode ser um algoritmo estatistico para medir a variação do sistema
                    // variancia para margem, se o comportamento esta regular, redução mais agressiva.
                    // Beta, taxa de desconfiança da IA
                    // Soma tudo (overhead do SO ta incorporada nos contadores) < 1 - Beta_variancia (.95, .9, .8)
                    Thread::_Statistics::decrease_frequency[cpu] = usage < (1 - threshold);

                    Thread::_Statistics::prediction_ready[cpu] = !error;
                    return false;
                } else if (!cpu && Thread::_Statistics::prediction_ready[1] && Thread::_Statistics::prediction_ready[2] && Thread::_Statistics::prediction_ready[3]) {
                    // CPU0 orchestrate frequency downgrade 
                    bool vote = true;
                    for (unsigned int i = 1; i < Traits<Build>::CPUS; ++i)
                    {
                        Thread::_Statistics::votes[i][Thread::_Statistics::hyperperiod_count[i]] = Thread::_Statistics::decrease_frequency[i];
                        Thread::_Statistics::idle_time_vote[i][Thread::_Statistics::hyperperiod_count[i]] = Thread::_Statistics::hyperperiod_idle_time[i];
                        vote &= Thread::_Statistics::decrease_frequency[i];
                        Thread::_Statistics::prediction_ready[i] = false;
                    }

                    if (vote) { // all CPUS decided for decrease
                        Hertz freq = Machine::frequency();
                        if (freq > 600000000) {
                            for (unsigned int i = 1; i < Traits<Build>::CPUS; ++i) {
                                Thread::_Statistics::to_learn[i] = true;
                                Thread::_Statistics::cooldown[i] = true;
                            }
                            Machine::clock(freq - (100 * 1000 * 1000));
                        }
                    } else {
                        if (!imbalanced) {
                            imbalanced = true;
                            /* TS4
                            Thread::_Statistics::migration_hyperperiod[0] = Thread::_Statistics::hyperperiod_count[1];
                            Thread::_Statistics::migration_hyperperiod[1] = Thread::_Statistics::hyperperiod_count[2];
                            Thread::_Statistics::migration_hyperperiod[2] = Thread::_Statistics::hyperperiod_count[3];
                            //db<Thread>(WRN) << "MIGRATION=" << Thread::_Statistics::hyperperiod_count[1] << "," << Thread::_Statistics::hyperperiod_count[2] << "," << Thread::_Statistics::hyperperiod_count[3] << endl;

                            unsigned int x = 2; //from
                            unsigned int y = 4; // thread
                            unsigned int z = 1; // to
                            Thread::_Statistics::threads_cpu[x][y]->_statistics.migrate_to = z;
                            Thread::_Statistics::threads_cpu[z][Thread::_Statistics::t_count_cpu[z]] = Thread::_Statistics::threads_cpu[x][y];
                            Thread::_Statistics::t_count_cpu[z]++;
                            for (unsigned int i = y+1; i < Thread::_Statistics::t_count_cpu[x]; ++i)
                            {
                                Thread::_Statistics::threads_cpu[x][i-1] = Thread::_Statistics::threads_cpu[x][i];
                            }
                            Thread::_Statistics::t_count_cpu[x]--;
                            Thread::_Statistics::cooldown[x] = true;
                            Thread::_Statistics::cooldown[z] = true;

                            x = 3; //from
                            y = 1; // thread
                            z = 1; // to
                            Thread::_Statistics::threads_cpu[x][y]->_statistics.migrate_to = z;
                            Thread::_Statistics::threads_cpu[z][Thread::_Statistics::t_count_cpu[z]] = Thread::_Statistics::threads_cpu[x][y];
                            Thread::_Statistics::t_count_cpu[z]++;
                            for (unsigned int i = y+1; i < Thread::_Statistics::t_count_cpu[x]; ++i)
                            {
                                Thread::_Statistics::threads_cpu[x][i-1] = Thread::_Statistics::threads_cpu[x][i];
                            }
                            Thread::_Statistics::t_count_cpu[x]--;
                            Thread::_Statistics::cooldown[x] = true;
                            Thread::_Statistics::cooldown[z] = true;

                            x = 2; // from
                            y = 3; // thread
                            z = 3; // to
                            Thread::_Statistics::threads_cpu[x][y]->_statistics.migrate_to = z;
                            Thread::_Statistics::threads_cpu[z][Thread::_Statistics::t_count_cpu[z]] = Thread::_Statistics::threads_cpu[x][y];
                            Thread::_Statistics::t_count_cpu[z]++;
                            for (unsigned int i = y+1; i < Thread::_Statistics::t_count_cpu[x]; ++i)
                            {
                                Thread::_Statistics::threads_cpu[x][i-1] = Thread::_Statistics::threads_cpu[x][i];
                            }
                            Thread::_Statistics::t_count_cpu[x]--;
                            Thread::_Statistics::cooldown[x] = true;
                            Thread::_Statistics::cooldown[z] = true;
                            */

                            /* TS3
                            unsigned int x = 2; //from
                            unsigned int y = 1; // thread
                            unsigned int z = 1; // to
                            Thread::_Statistics::threads_cpu[x][y]->_statistics.migrate_to = z;
                            Thread::_Statistics::threads_cpu[z][Thread::_Statistics::t_count_cpu[z]] = Thread::_Statistics::threads_cpu[x][y];
                            Thread::_Statistics::t_count_cpu[z]++;
                            for (unsigned int i = y+1; i < Thread::_Statistics::t_count_cpu[x]; ++i)
                            {
                                Thread::_Statistics::threads_cpu[x][i-1] = Thread::_Statistics::threads_cpu[x][i];
                            }
                            Thread::_Statistics::t_count_cpu[x]--;
                            Thread::_Statistics::cooldown[x] = true;
                            Thread::_Statistics::cooldown[z] = true;

                            x = 3; // from
                            y = 3; // thread
                            z = 1; // to
                            Thread::_Statistics::threads_cpu[x][y]->_statistics.migrate_to = z;
                            Thread::_Statistics::threads_cpu[z][Thread::_Statistics::t_count_cpu[z]] = Thread::_Statistics::threads_cpu[x][y];
                            Thread::_Statistics::t_count_cpu[z]++;
                            for (unsigned int i = y+1; i < Thread::_Statistics::t_count_cpu[x]; ++i)
                            {
                                Thread::_Statistics::threads_cpu[x][i-1] = Thread::_Statistics::threads_cpu[x][i];
                            }
                            Thread::_Statistics::t_count_cpu[x]--;
                            Thread::_Statistics::cooldown[x] = true;
                            Thread::_Statistics::cooldown[z] = true;

                            x = 3; // from
                            y = 3; // thread // thread 4 - 1
                            z = 1; // to
                            Thread::_Statistics::threads_cpu[x][y]->_statistics.migrate_to = z;
                            Thread::_Statistics::threads_cpu[z][Thread::_Statistics::t_count_cpu[z]] = Thread::_Statistics::threads_cpu[x][y];
                            Thread::_Statistics::t_count_cpu[z]++;
                            for (unsigned int i = y+1; i < Thread::_Statistics::t_count_cpu[x]; ++i)
                            {
                                Thread::_Statistics::threads_cpu[x][i-1] = Thread::_Statistics::threads_cpu[x][i];
                            }
                            Thread::_Statistics::t_count_cpu[x]--;
                            Thread::_Statistics::cooldown[x] = true;
                            Thread::_Statistics::cooldown[z] = true;
                            */
                            //Machine::clock(1200000000);//freq - (100 * 1000 * 1000));
                            //Monitor::ann[1] = FANN_EPOS::fann_create_from_config();
                            //Monitor::ann[2] = FANN_EPOS::fann_create_from_config();
                            //Monitor::ann[3] = FANN_EPOS::fann_create_from_config();
                        }
                        // check for imbalance
                        /*
                        Thread::_Statistics::threads_cpu[x][y]->_statostics.migrate_to = z;
                        
                        for (int i = y+1; i < Thread::_Statistics::t_count_cpu[x]; ++i)
                        {
                            Thread::_Statistics::threads_cpu[x][i-1] = Thread::_Statistics::threads_cpu[x][i];
                        }
                        Thread::_Statistics::threads_cpu[z][Thread::_Statistics::t_count_cpu[z]] = next;
                        Thread::_Statistics::t_count_cpu[z]++;
                        Thread::_Statistics::t_count_cpu[cpu]--;
                        Thread::_Statistics::cooldown[x] = true;
                        Thread::_Statistics::cooldown[z] = true;
                        if (!imbalanced && (greater_idle - smaller_idle > balance_threshold)) { // create imbalanced and balance_threshold
                            
                        }
                        */
                    }
                }
            }
        }
        return false;
    }

};

__END_UTIL
