#include <system/config.h>
#include <machine/cortex/engine/cortex_a53/bcm_arm_timer.h>

__BEGIN_SYS

class ITimer
{
public:
    ITimer(): start_ticks(0), timer(nullptr) {
        start_ticks = timer->count();
    }
    
    unsigned int stop(const char * what, void * where)
    {
        unsigned int val = timer->count() - start_ticks;
        kout << "||" << what << "@" << where << "||" << val << "ticks||\n";
        return val;
    } 
    ~ITimer() {}
private:
    unsigned int start_ticks;
    ARM_Timer * timer;
};

__END_SYS