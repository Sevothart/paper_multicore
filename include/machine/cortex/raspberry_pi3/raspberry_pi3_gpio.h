#ifndef __raspberry_pi3_gpio_h
#define __raspberry_pi3_gpio_h

#include <machine/cortex/raspberry_pi3/raspberry_pi3_memory_map.h>
#include <machine/gpio.h>

#include <architecture/cpu.h>
#include <machine/ic.h>

#define GPIO_SET *(gpio+7)              // sets   bits which are 1 ignores bits which are 0
#define GPIO_CLR *(gpio+10)             // clears bits which are 1 ignores bits which are 0
#define GET_GPIO(g) (*(gpio+13)&(1<<g)) // 0 if LOW, (1<<g) if HIGH

__BEGIN_SYS

class GPIO_Engine: public GPIO_Common
{
private:
    typedef CPU::Reg32 Reg32;
    typedef IC_Common::Interrupt_Id Interrupt_Id;

protected:
    static const unsigned int PORTS = Traits<GPIO>::UNITS;

private:
    /*
    There are 54 GPIO pins, divided in 6 ports, from 0 to 5
    Ports from 0 to 4 have 10 pins in each [0, 27] with 3 bits length
    Port 5 has 4 pins from [0, 12] with 3 bits length
    */

    // Registers offset from BASE
    enum {                    // Description
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

    // Function Select Values
    enum {
        GPFSEL_IN   = 0,
        GPFSEL_OUT  = 1,	 
        GPFSEL_ALT0 = 4,	 
        GPFSEL_ALT1 = 5,	 
        GPFSEL_ALT2 = 6,	 
        GPFSEL_ALT3 = 7,	 
        GPFSEL_ALT4 = 3,	 
        GPFSEL_ALT5 = 2
    };

    // Pull-up/down Enable Values
    enum {
        GPPUD_NONE  = 0,	 
        GPPUD_DOWN  = 1,	 
        GPPUD_UP    = 2
    };

    // Masks
    enum {
        GPFSEL_MASK             = 7,
        GPPUD_MASK              = 3,
        GP_SET_CLEAR_LEV_MASK   = 1
    };

public:
    /* Ports[A,F] Pins[0,9] */
    GPIO_Engine(const Port & port, const Pin & pin, const Direction & dir, const Pull & p, const Edge & int_edge)
    {
        _port = GPFSEL0 + (4 * port);   // Get the FSEL port adress
        _pin = 3 * pin;                 // Represent the bit that's used to write on direction and select_pin_function
        _mask = (10 * port) + pin;      // Represents the bit_mask from 0 to 53, used on restant methods

        direction( dir );
        select_pin_function( GPFSEL_ALT0 );
        pull( p );
        set();
        
        kout << "\t" << get() << endl;

    }

    bool get(){
        return gpio(GPLEV0 + _mask);
    }

    void set() {
        gpio(GPSET0) = 1 << _mask;
    }

    void clear() {
        gpio(GPCLR0) = 1 << _mask;
    }

    void direction(const Direction & dir) {
        _direction = dir;

        if( dir == GPIO_Common::IN ){
            gpio(_port) &= ~(GPFSEL_MASK << _mask);
            gpio(_port) |= (GPFSEL_IN << _mask);
        }
        if( dir == GPIO_Common::OUT ) {
            gpio(_port) &= ~(GPFSEL_MASK << _mask);
            gpio(_port) |= (GPFSEL_IN << _mask);
            gpio(_port) |= (GPFSEL_OUT << _mask);
        }
    }

    void select_pin_function(const int &func){
        gpio(_port) |= (func << _mask);
    }
    
    void pull( const Pull & p ){

        if ( p==Pull::DOWN )
            gpio(GPPUD) = GPPUD_DOWN;

        for(int i = 0; i < 150; i++) asm volatile ("nop"); // 150 cycles to synchronize
        gpio(GPPUDCLK0) = (1 << _mask);
        for(int i = 0; i < 150; i++) asm volatile ("nop"); // 150 cycles to synchronize

        /* flush GPIO setup */
        // gpio(GPPUD) = 0;
        gpio(GPPUDCLK0) = 0;                               
    }

    void int_enable(){}
    void int_disable(){}
    void int_enable(const Level & level, bool power_up = false, const Level & power_up_level = HIGH){}
    void int_enable(const Edge & edge, bool power_up = false, const Edge & power_up_edge = RISING){}

    void clear_interrupts(){}
    static void init(){}

private:
    volatile Reg32 & gpio(unsigned int o) { return reinterpret_cast<volatile Reg32 *>(Memory_Map::GPIO_BASE)[o / sizeof(Reg32)]; }

    Port _port;
    Pin _pin;
    Pin _mask;
    Direction _direction;
};

__END_SYS
#endif