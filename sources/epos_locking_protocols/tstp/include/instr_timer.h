#include "system/config.h"
#include "machine/cortex_m/timer.h"

__BEGIN_SYS

class ITimer
{
public:
    
    static const unsigned int tmrval = 0x7fffffff;
    
    ITimer():
    tmr( 2, 0, false, false, false)
    {
        tmr.load(tmrval);
        tmr.enable();
    }
    
    void stop(const char * what, void * where)
    {
        unsigned int val = tmrval - tmr.read();
        kout << "||" << what << "@" << where << "|" << val << " ticks||\n";
    }
    
    ~ITimer()
    {
    }
    
    Cortex_M_GPTM tmr;
};

__END_SYS