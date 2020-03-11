// EPOS Tracer Utility

// All we need to do here is to define the static event storage buffer and other static variables
// If not used they are (hopefully) cleaned by the symbol stripper

#include <system/config.h>
#include <tracer.h>

__BEGIN_SYS

char Tracer::_s_buf[Tracer::buffer_size];
int Tracer::_s_used = 0;
bool Tracer::_s_overflow = false;
__END_SYS