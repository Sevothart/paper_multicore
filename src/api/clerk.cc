// EPOS Clerk Implementation

#include <clerk.h>

__BEGIN_SYS

// Clerk
#ifdef __PMU_H

bool Clerk<PMU>::_in_use[Traits<Build>::CPUS][CHANNELS];

#endif

// System_Monitor
Simple_List<Monitor> Monitor::_monitors[Traits<Build>::CPUS];

void Monitor::run()
{
    db<Monitor>(TRC) << "Monitor::run()" << endl;

    Simple_List<Monitor> * monitor = &_monitors[CPU::id()];
    for(List::Iterator it = monitor->begin(); it != monitor->end(); it++)
        it->object()->capture();
}

void Monitor::init()
{
//    if(Traits<Monitor>::CPU_MONITOR_TEMPERATURE) {
//        Clerk<Termometer> * temperature_clerk = new Clerk<Termometer>(TEMPERATURE);
//        _monitors.insert(new Clerk_Monitor<temperature_clerk, Traits<Monitor>::CPU_MONITOR_TEMPERATURE>);
//    }

    db<Monitor>(TRC) << "Monitor::init()" << endl;

    if(Traits<Monitor>::MONITOR_ELAPSED_TIME && (CPU::id() == 0)) {
        db<Monitor>(TRC) << "Monitor::init: monitoring ELAPSED TIME at " << Traits<Monitor>::MONITOR_ELAPSED_TIME << " Hz" << endl;
        new Clerk<System>(ELAPSED_TIME, Traits<Monitor>::MONITOR_ELAPSED_TIME, true);
    }

    if(Traits<Monitor>::MONITOR_DEADLINE_MISS) {
        db<Monitor>(TRC) << "Monitor::init: monitoring DEADLINE MISS at " << Traits<Monitor>::MONITOR_DEADLINE_MISS << " Hz" << endl;
        new Clerk<System>(DEADLINE_MISS, Traits<Monitor>::MONITOR_DEADLINE_MISS, true);
    }

#ifdef __PMU_H

    unsigned int used_channels = 0;

    if(Traits<Monitor>::MONITOR_CLOCK && (used_channels < Clerk<PMU>::CHANNELS)) {
        db<Monitor>(TRC) << "Monitor::init: monitoring CLOCK at " << Traits<Monitor>::MONITOR_CLOCK << " Hz" << endl;
        new Clerk<PMU>(CLOCK, Traits<Monitor>::MONITOR_CLOCK, true);
        used_channels++;
    }

    if(Traits<Monitor>::MONITOR_DVS_CLOCK && (used_channels < Clerk<PMU>::CHANNELS)) {
        db<Monitor>(TRC) << "Monitor::init: monitoring DVS CLOCK at " << Traits<Monitor>::MONITOR_DVS_CLOCK << " Hz" << endl;
        new Clerk<PMU>(DVS_CLOCK, Traits<Monitor>::MONITOR_DVS_CLOCK, true);
        used_channels++;
    }

    if(Traits<Monitor>::MONITOR_INSTRUCTION && (used_channels < Clerk<PMU>::CHANNELS)) {
        db<Monitor>(TRC) << "Monitor::init: monitoring INSTRUCTION at " << Traits<Monitor>::MONITOR_INSTRUCTION << " Hz" << endl;
        new Clerk<PMU>(INSTRUCTION, Traits<Monitor>::MONITOR_INSTRUCTION, true);
        used_channels++;
    }

    if(Traits<Monitor>::MONITOR_BRANCH && (used_channels < Clerk<PMU>::CHANNELS)) {
        db<Monitor>(TRC) << "Monitor::init: monitoring BRANCH at " << Traits<Monitor>::MONITOR_BRANCH << " Hz" << endl;
        new Clerk<PMU>(BRANCH, Traits<Monitor>::MONITOR_BRANCH, true);
        used_channels++;
    }

    if(Traits<Monitor>::MONITOR_BRANCH_MISS && (used_channels < Clerk<PMU>::CHANNELS)) {
        db<Monitor>(TRC) << "Monitor::init: monitoring BRANCH_MISS at " << Traits<Monitor>::MONITOR_BRANCH_MISS << " Hz" << endl;
        new Clerk<PMU>(BRANCH_MISS, Traits<Monitor>::MONITOR_BRANCH_MISS, true);
        used_channels++;
    }

    if(Traits<Monitor>::MONITOR_L1_HIT && (used_channels < Clerk<PMU>::CHANNELS)) {
        db<Monitor>(TRC) << "Monitor::init: monitoring L1_HIT at " << Traits<Monitor>::MONITOR_L1_HIT << " Hz" << endl;
        new Clerk<PMU>(L1_HIT, Traits<Monitor>::MONITOR_L1_HIT, true);
        used_channels++;
    }

    if(Traits<Monitor>::MONITOR_L2_HIT && (used_channels < Clerk<PMU>::CHANNELS)) {
        db<Monitor>(TRC) << "Monitor::init: monitoring L2_HIT at " << Traits<Monitor>::MONITOR_L2_HIT << " Hz" << endl;
        new Clerk<PMU>(L2_HIT, Traits<Monitor>::MONITOR_L2_HIT, true);
        used_channels++;
    }

    if(Traits<Monitor>::MONITOR_L3_HIT && (used_channels < Clerk<PMU>::CHANNELS)) {
        db<Monitor>(TRC) << "Monitor::init: monitoring L3_HIT at " << Traits<Monitor>::MONITOR_L3_HIT << " Hz" << endl;
        new Clerk<PMU>(L3_HIT, Traits<Monitor>::MONITOR_L3_HIT, true);
        used_channels++;
    }

    if(Traits<Monitor>::MONITOR_LLC_HIT && (used_channels < Clerk<PMU>::CHANNELS)) {
        db<Monitor>(TRC) << "Monitor::init: monitoring LLC_HIT at " << Traits<Monitor>::MONITOR_LLC_HIT << " Hz" << endl;
        new Clerk<PMU>(LLC_HIT, Traits<Monitor>::MONITOR_LLC_HIT, true);
        used_channels++;
    }

    if(Traits<Monitor>::MONITOR_CACHE_HIT && (used_channels < Clerk<PMU>::CHANNELS)) {
        db<Monitor>(TRC) << "Monitor::init: monitoring CACHE_HIT at " << Traits<Monitor>::MONITOR_CACHE_HIT << " Hz" << endl;
        new Clerk<PMU>(CACHE_HIT, Traits<Monitor>::MONITOR_CACHE_HIT, true);
        used_channels++;
    }

    if(Traits<Monitor>::MONITOR_L1_MISS && (used_channels < Clerk<PMU>::CHANNELS)) {
        db<Monitor>(TRC) << "Monitor::init: monitoring L1_MISS at " << Traits<Monitor>::MONITOR_L1_MISS << " Hz" << endl;
        new Clerk<PMU>(L1_MISS, Traits<Monitor>::MONITOR_L1_MISS, true);
        used_channels++;
    }

    if(Traits<Monitor>::MONITOR_L2_MISS && (used_channels < Clerk<PMU>::CHANNELS)) {
        db<Monitor>(TRC) << "Monitor::init: monitoring L2_MISS at " << Traits<Monitor>::MONITOR_L2_MISS << " Hz" << endl;
        new Clerk<PMU>(L2_MISS, Traits<Monitor>::MONITOR_L2_MISS, true);
        used_channels++;
    }

    if(Traits<Monitor>::MONITOR_L3_MISS && (used_channels < Clerk<PMU>::CHANNELS)) {
        db<Monitor>(TRC) << "Monitor::init: monitoring L3_MISS at " << Traits<Monitor>::MONITOR_L3_MISS << " Hz" << endl;
        new Clerk<PMU>(L3_MISS, Traits<Monitor>::MONITOR_L3_MISS, true);
        used_channels++;
    }

    if(Traits<Monitor>::MONITOR_LLC_MISS && (used_channels < Clerk<PMU>::CHANNELS)) {
        db<Monitor>(TRC) << "Monitor::init: monitoring LLC_MISS at " << Traits<Monitor>::MONITOR_LLC_MISS << " Hz" << endl;
        new Clerk<PMU>(LLC_MISS, Traits<Monitor>::MONITOR_LLC_MISS, true);
        used_channels++;
    }

    if(Traits<Monitor>::MONITOR_CACHE_MISS && (used_channels < Clerk<PMU>::CHANNELS)) {
        db<Monitor>(TRC) << "Monitor::init: monitoring CACHE_MISS at " << Traits<Monitor>::MONITOR_CACHE_MISS << " Hz" << endl;
        new Clerk<PMU>(CACHE_MISS, Traits<Monitor>::MONITOR_CACHE_MISS, true);
        used_channels++;
    }

    if(Traits<Monitor>::MONITOR_LLC_HITM && (used_channels < Clerk<PMU>::CHANNELS)) {
        db<Monitor>(TRC) << "Monitor::init: monitoring LLC_HITM at " << Traits<Monitor>::MONITOR_LLC_HITM << " Hz" << endl;
        new Clerk<PMU>(LLC_HITM, Traits<Monitor>::MONITOR_LLC_HITM, true);
        used_channels++;
    }

    if(used_channels >= Clerk<PMU>::CHANNELS)
        db<Monitor>(WRN) << "Monitor::init: some events not monitored because all PMU channels are busy!" << endl;

#endif

}

__END_SYS
