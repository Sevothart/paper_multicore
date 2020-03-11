// EPOS Tracer Utility

#ifndef __tracer_h
#define __tracer_h

#include <system/config.h>
#include <system/traits.h>
#include <architecture/armv7/tsc.h>
#include <machine/timer.h>

__BEGIN_SYS

class Tracer {
public:
    static const bool enabled = Traits<Tracer>::enabled;
    
    static const bool use_timer = enabled && true;
    static const bool enable_store = enabled && Traits<Tracer>::store_enabled;
    static const bool enable_print = enabled && Traits<Tracer>::print_enabled;
    static const int buffer_size = enable_store ? Traits<Tracer>::buffer_size : 0;
    
    class Event {
        
    public:    
        
        enum EventType {
            E_UNDEFINED,
            E_THR_CREATE = 0x30,
            E_THR_START,
            E_SEM_CREATE = 0x40,
            E_SEM_CHANGE,
            E_USR_BASE = 0xf0
        };
        
        Event( EventType t ): type(t), ptr1(0), int1(0), int2(0) { timestamp = 0; } //TSC::time_stamp(); }
        
        
        typedef TSC::Time_Stamp Timestamp;
        
        static int get_size ( EventType et )
        {
            
            return 1 + // one byte for event type
                   sizeof(Timestamp) + // some bytes for timestamp
                   ( et >= E_USR_BASE? (2*sizeof(int)) : (sizeof(int) + sizeof(void*)) ); // some bytes for content
        };
        
        int write ( void * loc, int capacity) const
        {
            int s = get_size(type);
            if(s > capacity)
                return -1; // do not store if event size exceeds capacity
            
            char * buf = (char*) loc;
            
            buf[0] = type; //(char) type;
            buf++;
            
            *((Timestamp*)(buf)) = timestamp;
            buf += sizeof(timestamp);
            
            switch(type)
            {
                // ptr1, int1
                case E_THR_CREATE:
                case E_THR_START:
                case E_SEM_CREATE:
                case E_SEM_CHANGE:
                    *((void**)buf) = ptr1;
                    buf += sizeof(void*);
                    *((int*)buf) = int2;
                    buf += sizeof(int);
                    break;
                    
                // int1, int2
                default:
                    *((int*)buf) = int1;
                    buf += sizeof(int);
                    *((int*)buf) = int2;
                    buf += sizeof(int);
                    break;
            }
            
            return s;
        };
        
        void print() const
        {
            kout << "@" << timestamp << ":" << "EVENT\n"; // TODO implement printing
        };
        
        EventType type;
        Timestamp timestamp;
        void * ptr1;
        int int1, int2;
        
    };
    
    static void threadStarted(void * thread, int cpu)
    {
        if(enabled)
        {
            Event evt(Event::E_THR_START);
            evt.ptr1 = thread;
            evt.int1 = cpu;            
            eventDispatch(evt);
            
        }
    }
    
    static void semaphoreCreated(void * semaphore, int value)
    {
        if(enabled)
        {
            Event evt(Event::E_SEM_CREATE);
            evt.ptr1 = semaphore;
            evt.int1 = value;            
            eventDispatch(evt);            
        }
    }
    
    static void semaphoreChanged(void * semaphore, int value)
    {
        if(enabled)
        {
            Event evt(Event::E_SEM_CHANGE);
            evt.ptr1 = semaphore;
            evt.int1 = value;            
            eventDispatch(evt);  
        }
    }
    
    static void userEvent(char eventOffset, int param1, int param2)
    {
        if(enabled)
        {
            Event evt((Event::EventType) (Event::E_USR_BASE + 0x0f));
            evt.int1 = param1;
            evt.int2 = param2;            
            eventDispatch(evt);  
        }
    }
    
    static void dump_nibble(unsigned char n)
    {
        kout << (char)(n > 9? 'A' + (n - 10) : '0' + n);
    }
    
    static void dump_byte(unsigned char byte)
    {
        dump_nibble(byte >> 4);
        dump_nibble(byte & 0xf);
    }
    
    static void dump()
    {
        if(_s_overflow)
            kout << "WARNING: buffer is incomplete, not all events fit";
        kout << "@@BEGIN_TRACER_DUMP\n";
        
        for(int i = 0; i < _s_used; i++)
        {
            dump_byte(_s_buf[i]);
        }
        
        kout << "\n@@END_TRACER_DUMP\n";
    }
    
private:
    inline static void eventDispatch(const Event& evt)
    {
        if(enable_store)
            {
                int written = evt.write(_s_buf+_s_used, buffer_size - _s_used);
                if(written >= 0)
                {
                    _s_used += written;
                }
                else _s_overflow = true;
            }
            
            if(enable_print)
            {
                evt.print();
            }
    }
    
    static char _s_buf[buffer_size];
    static int _s_used;
    static bool _s_overflow;
};

__END_SYS

#endif