#ifndef __raspberry_pi3_gpio_h
#define __raspberry_pi3_gpio_h

#include <machine/cortex/raspberry_pi3/raspberry_pi3_memory_map.h>
#include <machine/gpio.h>

#include <architecture/cpu.h>
#include <machine/ic.h>

__BEGIN_SYS

class GPIO_Engine: public GPIO_Common
{
private:
    typedef CPU::Reg32 Reg32;
    typedef IC_Common::Interrupt_Id Interrupt_Id;

protected:
    static const unsigned int PORTS = Traits<GPIO>::UNITS;

public:
    //Registers offset from BASE
    enum {                    // Description
        GPFSEL0   = 0x000,    // Function Select
        GPFSEL1   = 0x004,    
        GPFSEL2   = 0x008,    
        GPFSEL3   = 0x00C,
        GPFSEL4   = 0x010,
        GPFSEL5   = 0x014,
        GPSET0    = 0x01C,    // Pin Output Set
        GPSET1    = 0x020,
        GPCLR0    = 0x028,    // Pin Output Clear
        GPCLR1    = 0x02C,
        GPLEV0    = 0x034,    // Pin Level
        GPLEV1    = 0x038,
        GPEDS0    = 0x040,    // Pin Event Detect Status
        GPEDS1    = 0x044,
        GPREN0    = 0x04C,    // Pin Rising Edge Detect Enable
        GPREN1    = 0x050,
        GPFEN0    = 0x058,    // Pin Falling Edge Detect Enable
        GPFEN1    = 0x05C,
        GPHEN0    = 0x064,    // Pin High Detect Enable
        GPHEN1    = 0x068,
        GPLEN0    = 0x070,    // Pin Low Detect Enable
        GPLEN1    = 0x074,
        GPAREN0   = 0x07C,    // Pin Async Raising Edge Detect
        GPAREN1   = 0x080,
        GPAFEN0   = 0x088,    // Pin Async Falling Edge Detect
        GPAFEN1   = 0x08C,
        GPPUD     = 0x094,    // Pin Pull-up/down Enable 
        GPPUDCLK0 = 0x098,    // GPIO Pin Pull-up/down Enable Clock
        GPPUDCLK1 = 0x09C
    };

    // Function Selector Helper
    // From 0 to 9 -> start-bits: 0 3 6 9 12 15 18 21 24 27
    enum {
        FSEL_IN   = 000, //GPIO pin n has input
        FSEL_OUT  = 001, //GPIO pin n has output
        FSEL_ALT0 = 100, //GPIO pin n takes alternate function 0
        FSEL_ALT1 = 101, //GPIO pin n takes alternate function 1
        FSEL_ALT2 = 110, //GPIO pin n takes alternate function 2
        FSEL_ALT3 = 111, //GPIO pin n takes alternate function 3
        FSEL_ALT4 = 011, //GPIO pin n takes alternate function 4
        FSEL_ALT5 = 010  //GPIO pin n takes alternate function 5
    };

public:

    GPIO_Engine(const Port & port, const Pin & pin, const Direction & dir, const Pull & p, const Edge & int_edge)
    {    
        /*
        _gpio = new(reinterpret_cast<void *>(Memory_Map::GPIO_BASE + port * 0x1000)) PL061;
        _gpio->select_pin_function(_pin_mask, PL061::FUN_GPIO);
        */

        /* I will alternatly write high and low on GPIO26 -> Physical pin 37 on the board. */
        gpio(GPFSEL2) = FSEL_OUT  << 18;
        gpio(GPFSEL2) = FSEL_ALT0 << 18;
        set(26);
    }

    bool get(const Pin & mask){
        if(mask > 31)
            return gpio(GPLEV0);
        else
            return gpio(GPLEV1);
    }

    void set(const Pin & mask) {
        if(mask > 31)
            gpio(GPSET0) = 1 << mask;
        else
            gpio(GPSET1) = 1 << mask;
    }

    void clear(const Pin & mask) {
        if(mask > 31)
            gpio(GPCLR0) = 1 << mask;
        else
            gpio(GPCLR1) = 1 << mask;
    }

    void direction(const Pin & mask, const Direction & dir){}
    
    void int_enable(){}
    void int_disable(){}
    void int_enable(const Level & level, bool power_up = false, const Level & power_up_level = HIGH){}
    void int_enable(const Edge & edge, bool power_up = false, const Edge & power_up_edge = RISING){}

    void pull(const Pin & mask, const Pull & p){}
    void clear_interrupts(){}
    static void init(){}

private:
    volatile Reg32 & gpio(unsigned int o) { return reinterpret_cast<volatile Reg32 *>(this)[o / sizeof(Reg32)]; }
};

__END_SYS
#endif