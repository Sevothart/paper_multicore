#include "inst-timer.h"

__BEGIN_SYS

int ITimer::_dataIterator = 0;

ITimer::Time_Stamp ITimer::_ticks[MAX];
const char * ITimer::_name[MAX];

void ITimer::stop(const char * what, void * where)
{
    ITimer::_name[_dataIterator] = what;
    ITimer::_ticks[_dataIterator] = timer.time_stamp() - _startTime;

    ITimer::_dataIterator++;
}

void ITimer::printData()
{
    for(int k = 0; k < _dataIterator; k++)
        kout << "|| "<< k << " || " << ITimer::_name[k] << " || " << ITimer::_ticks[k] << " ||\n";
}

void ITimer::cleanData()
{
    for(int k = 0; k < _dataIterator; k++)
    {
        ITimer::_name[k] = 0;
        ITimer::_ticks[k] = 0;
    }

    _dataIterator = 0;
}

__END_SYS