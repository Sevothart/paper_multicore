#ifndef __bcm_gpio_h
#define __bcm_gpio_h

#include <architecture/cpu.h>

#define __timer_common_only__
#include <machine/gpio.h>
#undef __timer_common_only__

__BEGIN_SYS

class BCM_GPIO: public GPIO_Common
{
private:
    typedef CPU::Reg32 Reg32;

    // Registers offset from BASE
    enum {
        GPFSEL0   = 0x00000,    // Function Select: Port A
        GPFSEL1   = 0x00004,    // Function Select: Port B
        GPFSEL2   = 0x00008,    // Function Select: Port C
        GPFSEL3   = 0x0000C,    // Function Select: Port D
        GPFSEL4   = 0x00010,    // Function Select: Port E
        GPFSEL5   = 0x00014,    // Function Select: Port F
        GPSET0    = 0x0001C,    // Pin Output Set
        GPSET1    = 0x00020,
        GPCLR0    = 0x00028,    // Pin Output Clear
        GPCLR1    = 0x0002C,
        GPLEV0    = 0x00034,    // Pin Level
        GPLEV1    = 0x00038,
        GPEDS0    = 0x00040,    // Pin Event Detect Status
        GPEDS1    = 0x00044,
        GPREN0    = 0x0004C,    // Pin Rising Edge Detect Enable
        GPREN1    = 0x00050,
        GPFEN0    = 0x00058,    // Pin Falling Edge Detect Enable
        GPFEN1    = 0x0005C,
        GPHEN0    = 0x00064,    // Pin High Detect Enable
        GPHEN1    = 0x00068,
        GPLEN0    = 0x00070,    // Pin Low Detect Enable
        GPLEN1    = 0x00074,
        GPAREN0   = 0x0007C,    // Pin Async Raising Edge Detect
        GPAREN1   = 0x00080,
        GPAFEN0   = 0x00088,    // Pin Async Falling Edge Detect
        GPAFEN1   = 0x0008C,
        GPPUD     = 0x00094,    // Pin Pull-up/down Enable 
        GPPUDCLK0 = 0x00098,    // GPIO Pin Pull-up/down Enable Clock
        GPPUDCLK1 = 0x0009C
    };

    // Function select enable values
    enum {
        GPFSEL_IN   = 0,
        GPFSEL_OUT  = 1,	 
        GPFSEL_ALT0 = 4,	 
        GPFSEL_ALT1 = 5,	 
        GPFSEL_ALT2 = 6,	 
        GPFSEL_ALT3 = 7,	 
        GPFSEL_ALT4 = 3,	 
        GPFSEL_ALT5 = 2,
        GPFSEL_MASK = 7
    };

    // Pull-up and down enable values
    enum {
        GPPUD_NONE  = 0,	 
        GPPUD_DOWN  = 1,	 
        GPPUD_UP    = 2,
        GPPUD_MASK  = 3
    };
    
public:

    bool get(const Pin & mask){
        return (gpio(GPLEV0) & (1 << mask)) ? 1 : 0;
    }

    void set(const Pin & mask) {
        gpio(GPSET0) = 1 << mask;
    }
    
    void clear(const Pin & mask) {
        gpio(GPCLR0) = 1 << mask;
    }
    
    void direction(const Pin & mask, const Direction & dir, const int & port) {
        int shift = (mask % 10) * 3;

        if( dir == Direction::IN || dir == Direction::INOUT ) {
            set_bits( GPFSEL_MASK << shift, GPFSEL_IN << shift, port );
        }
        if( dir == Direction::OUT ) {
            set_bits( GPFSEL_MASK << shift, GPFSEL_OUT << shift, port );
        }
    }

    void select_pin_function(const int & func, const Pin & mask, const int & port){
        int shift = (mask % 10) * 3;
        set_bits( GPFSEL_MASK << shift, func << shift, port );
    }
    
    void pull( const Pin & mask, const Pull & p ){

        switch (p)
        {
            case Pull::FLOATING :
                gpio(GPPUD) = GPPUD_NONE; break;
            case Pull::UP :
                gpio(GPPUD) = GPPUD_UP  ; break;
            case Pull::DOWN :
                gpio(GPPUD) = GPPUD_DOWN; break;
            default:
                break;
        }

        for(int i = 0; i < 150; i++) asm volatile ("nop"); // 150 cycles to synchronize
        gpio(GPPUDCLK0) = 1 << mask;
        for(int i = 0; i < 150; i++) asm volatile ("nop"); // 150 cycles to synchronize

        gpio(GPPUD) = GPPUD_NONE; /* Remove the control signal */
        gpio(GPPUDCLK0) = 0;      /* Remove the clock */  
    }
    
    void int_enable(){}
    void int_disable(){}
    void int_enable(const Level & level, bool power_up = false, const Level & power_up_level = HIGH){}
    void int_enable(const Edge & edge, bool power_up = false, const Edge & power_up_edge = RISING){}

    void clear_interrupts(){}
    static void init(){}

private:
    volatile Reg32 & gpio(unsigned int o) { return reinterpret_cast<volatile Reg32 *>(this)[o / sizeof(Reg32)]; }

    void set_bits(Reg32 value, Reg32 mask, const int & port){
        unsigned int aux;
        aux = gpio(port);
        aux = (aux & ~mask) | (value & mask);
        gpio(port) = aux;
    }
};

__END_SYS

#endif