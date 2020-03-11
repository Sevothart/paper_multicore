#include <system/config.h>
#include <machine/cortex/engine/cortex_m3/gptm.h>
#include <architecture/armv7/tsc.h>

__BEGIN_SYS

class ITimer
{
public:
    ITimer(){
        start_time = TSC::time_stamp();
    }
    
    unsigned int stop(const char * what, void * where)
    {
        unsigned int val = TSC::time_stamp() - start_time;
        //kout << "||" << what << "@" << where << "||" << val << "ticks||\n";
        return val;
    } 
    ~ITimer() {}
private:
    unsigned int start_time; 
};

__END_SYS