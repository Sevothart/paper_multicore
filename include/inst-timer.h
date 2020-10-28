#include "system/config.h"
#include "architecture/ia32/ia32_tsc.h"

__BEGIN_SYS

class ITimer
{
public:
    typedef unsigned long long Time_Stamp;

public:
    ITimer()
    {
        _startTime = timer.time_stamp();
    }

    void stop(const char * what, void * where);
    void printData();
    void cleanData();

public:
    TSC timer;
    Time_Stamp _startTime;

public:
    /* data_t container */
    static int _dataIterator;
    static const int MAX = 160000;

    static const char* _name[MAX];
    static Time_Stamp _ticks[MAX];
};

__END_SYS