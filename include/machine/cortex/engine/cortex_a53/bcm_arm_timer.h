// EPOS ARM Cortex-A53 ARM Timer Mediator Declarations

#ifndef __arm_timer_h
#define __arm_timer_h

#include <architecture/cpu.h>

#include <machine/cortex/raspberry_pi3/raspberry_pi3_gpio.h>
#include <machine/cortex/cortex_ic.h>
#include <system/memory_map.h>

#define __common_only__
#include <machine/rtc.h>
#include <machine/timer.h>
#undef __common_only__

__BEGIN_SYS

/*
ADDED: Methods init, enable, disable, arm_eoi, int_enable, handler and arm_timer 

TODO: Add ARM_Timer to Alarm and Chronometer of the system, instead of TSC.
system/config.h:
    Should add a macro like this #define __ARM_TIMER_H        __HEADER_ARCH(arm_timer);
architecture/armv7/cpu_init.cc:
    Should have a new #ifdef __ARM_TIMER_H that would call init method if Traits<ARM_TIMER>::enabled
*/

class ARM_Timer : public Timer_Common
{
    friend class CPU;

private:
    typedef CPU::Reg32 Reg32;
    typedef CPU::Reg16 Reg16;
    typedef CPU::Reg8 Reg8;
    typedef IC_Common::Interrupt_Id Interrupt_Id;
    typedef IC_Engine::Interrupt_Handler Interrupt_Handler;

public:
    typedef CPU::Reg64 Count;

    static const unsigned int CLOCK = 250000000 / 1; // 250 MHz / PRE_DIV (0xFA == 250)

    // Usefull Off-set Register from TIMER1_BASE
    enum {
        LOAD    = 0x000, // Load the content to VALUE when VALUE goes to 0 or when this register is written
        VALUE   = 0X004, // Read-Only, Count Down Value
        CONTROL = 0X008, // Timer control operations (enable, enable interruptions...)
        IRQ_CLR = 0x00c, // EOI
        RAW_IRQ = 0X010, // 1 if pendding INT
        MSK_IRQ = 0X014, // 1 if pendding INT and INT enable
        RELOAD  = 0X018, // Copy of LOAD, writing in this register does not reset VALUE Countdown
        PRE_DIV = 0X01c, // timer_clock = apb_clock/(pre_divider+1) - The reset value of this register is 0x7D so gives a divide by 126.
        CNTR    = 0x020  // Free Counter | Read Only
    };

    // CONTROL USEFULL BITS
    enum { // Reset = 3E0020
        CNTR_SIZE   = 1    << 1, // 1 == 23 bit counter, 0 == 16 bit counter
        PRE_SCALE   = 0x00 << 2, // 00 == Clock/1 | 01 == Clock/16 | 10 == Clock/256 | 11 == Clock/1 (?)
        INT_EN      = 1    << 5,
        TIMER_EN    = 1    << 7,
        FREE_CNTR   = 1    << 9,
        FREE_CLOCK  = 0x3e << 16
    };

public:
    void config(unsigned int unit, const Count & count) {
        timer(CONTROL) = FREE_CLOCK;
        timer(RELOAD) = count;
        timer(LOAD) = count;
        timer(PRE_DIV) = 0x01; //0xFA
        timer(IRQ_CLR) = 0;
        timer(CONTROL) = TIMER_EN | PRE_SCALE | CNTR_SIZE; //0x3e00a2
    }

    void eoi(IC::Interrupt_Id id) {
        timer(IRQ_CLR) = 0;
        /* ASM("dsb \t\n isb"); */
        while (timer(RAW_IRQ));
    }

    /* This is added to _eoi_vector of system in raspberry_pi3_ic_init.cc */
    static void arm_eoi(IC::Interrupt_Id id){ arm_timer()->eoi(id); }

    void int_enable() {
        timer(CONTROL) |= INT_EN;

        IC::enable( IC::INT_ARM_TIMER );
        IC::int_vector( IC::INT_ARM_TIMER, reinterpret_cast<Interrupt_Handler>(&handler) );
    }

    static void handler(Interrupt_Id a) {
        /* TODO: How to update GPIO level inside a static function */
        kout << "." << endl;
    }

    Count count() { return static_cast<Count>(timer(CNTR)); }

    void enable() {
        config(0, 10000000);
        timer(CONTROL) |= FREE_CNTR;
    }

    void disable() {
        timer(CONTROL) = FREE_CLOCK;
        timer(CONTROL) |= 0 << 9; 
        timer(CONTROL) |= 0 << 7;
    }

    void set(const Count & count) { timer(RELOAD) = count; }
    Hertz clock() { return CLOCK; }

private:
    /* Reference to base adress of the timer, access to registers */
    volatile Reg32 & timer(unsigned int o) { return reinterpret_cast<volatile Reg32 *>(Memory_Map::TIMER1_BASE)[o / sizeof(Reg32)]; }

    /* Reference to timer itself, to call methods without the creation of a ARM_Timer object */
    static ARM_Timer * arm_timer() { return reinterpret_cast<ARM_Timer *>(Memory_Map::TIMER1_BASE); }
    
    static void init() {
        arm_timer()->enable();
        // arm_timer()->int_enable();
    }

private:
    /* This GPIO pin bellow can be utilized to ensure ARM_Timer frequency on an oscilloscope */
    GPIO_Engine * _osc_pin;
};

__END_SYS

#endif