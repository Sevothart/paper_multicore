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

    // Function Selector Helper
    // From 0 to 9 -> start-bits: 0 3 6 9 12 15 18 21 24 27
    enum {
        FSEL_IN   = 0b000, //GPIO pin n has input
        FSEL_OUT  = 0b001, //GPIO pin n has output
        FSEL_ALT0 = 0b100, //GPIO pin n takes alternate function 0
        FSEL_ALT1 = 0b101, //GPIO pin n takes alternate function 1
        FSEL_ALT2 = 0b110, //GPIO pin n takes alternate function 2
        FSEL_ALT3 = 0b111, //GPIO pin n takes alternate function 3
        FSEL_ALT4 = 0b011, //GPIO pin n takes alternate function 4
        FSEL_ALT5 = 0b010  //GPIO pin n takes alternate function 5
    };

    // Pull helper
    enum {
        DISABLE     = 0,
        ENABLE_DOWN = 1,
        ENABLE_UP   = 2,
        RESERVED    = 3
    };

public:
    /* Ports[A,F] Pins[0,9] */
    GPIO_Engine(const Port & port, const Pin & pin, const Direction & dir, const Pull & p, const Edge & int_edge)
    {

        kout << "1.1: Definindo atributos privados" << endl;
        _port = GPFSEL0 + (4 * port);   // Get the FSEL port adress
        _pin = 3 * pin;                 // Represent the bit that's used to write on direction and select_pin_function
        _mask = (10 * port) + pin;      // Represents the bit_mask from 0 to 53, used on restant methods

        kout << "1.2: Reading initial state" << endl;
        get();

        kout << "1.3: Setting" << endl;
        direction( dir );

        kout << "1.4: Setting function selector" << endl;
        select_pin_function( FSEL_ALT0 );

        kout << "1.5: Setting pull" << endl;
        pull();

        kout << "1.6: Setting value" << endl;
        set();

        kout << "1.7: Reading current value" << endl;
        get();

    }

    void get(){
        for(int i = 0; i<54; i++){
            kout << i << ": " <<  gpio(GPLEV0 + i) << endl;
        }
        // return gpio(GPLEV0 + _mask);
    }

    void set() {
        gpio(GPSET0) = (1 << _mask);
    }

    void clear() {
        gpio(GPCLR0) = (1 << _mask);
    }

    void direction(const Direction & dir) {
        _direction = dir;

        if( dir == GPIO_Common::IN )
            gpio(_port) |= (FSEL_IN  << _pin);
        if( dir == GPIO_Common::OUT )
            gpio(_port) |= (FSEL_OUT << _pin);
        else
            gpio(_port) = (FSEL_IN << _pin) | (FSEL_OUT << _pin);
    }

    void select_pin_function(const int &func){
        gpio(_port) |= (func << _pin);
    }
    
    void pull(/*const Pin & mask, const Pull & p*/){
        gpio(GPPUD) = 0b01;
        for(int i = 0; i < 150; i++) asm volatile ("nop"); // 150 cycles to synchronize
        gpio(GPPUDCLK0) = (1 << _mask);
        for(int i = 0; i < 150; i++) asm volatile ("nop"); // 150 cycles to synchronize
        gpio(GPPUDCLK0) = 0;                               // flush GPIO setup
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