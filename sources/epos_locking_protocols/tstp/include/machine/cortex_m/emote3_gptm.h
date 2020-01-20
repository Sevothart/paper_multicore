#ifndef __emote3_gptm_h_
#define __emote3_gptm_h_

#include "emote3.h"

__BEGIN_SYS

// CC2538's General Purpose Timer definitions
class CC2538_GPTIMER 
{
protected:
    enum Base 
    {
        GPTIMER0_BASE = 0x40030000,
        GPTIMER1_BASE = 0x40031000,
        GPTIMER2_BASE = 0x40032000,
        GPTIMER3_BASE = 0x40033000
    };

    enum Offset 
    {
        //Register Name  Offset  Type  Width  Reset Value
        CFG           =   0x00,  //RW    32    0x00000000
        TAMR          =   0x04,  //RW    32    0x00000000
        TBMR          =   0x08,  //RW    32    0x00000000
        CTL           =   0x0C,  //RW    32    0x00000000
        SYNC          =   0x10,  //RW    32    0x00000000
        IMR           =   0x18,  //RW    32    0x00000000
        RIS           =   0x1C,  //RO    32    0x00000000
        MIS           =   0x20,  //RO    32    0x00000000
        ICR           =   0x24,  //RW    32    0x00000000
        TAILR         =   0x28,  //RW    32    0xFFFFFFFF
        TBILR         =   0x2C,  //RW    32    0x0000FFFF
        TAMATCHR      =   0x30,  //RW    32    0xFFFFFFFF
        TBMATCHR      =   0x34,  //RW    32    0x0000FFFF
        TAPR          =   0x38,  //RW    32    0x00000000
        TBPR          =   0x3C,  //RW    32    0x00000000
        TAPMR         =   0x40,  //RW    32    0x00000000
        TBPMR         =   0x44,  //RW    32    0x00000000
        TAR           =   0x48,  //RO    32    0xFFFFFFFF
        TBR           =   0x4C,  //RO    32    0x0000FFFF
        TAV           =   0x50,  //RW    32    0xFFFFFFFF
        TBV           =   0x54,  //RW    32    0x0000FFFF
        TAPS          =   0x5C,  //RO    32    0x00000000
        TBPS          =   0x60,  //RO    32    0x00000000
        TAPV          =   0x64,  //RO    32    0x00000000
        TBPV          =   0x68,  //RO    32    0x00000000
        PP            =  0xFC0,  //RO    32    0x00000000
    };

    enum CTL 
    {
        TBPWML = 1 << 14, // GPTM Timer B PWM output level
                          // 0: Output is unaffected.
                          // 1: Output is inverted. RW 0
        TBOTE = 1 << 13, // GPTM Timer B output trigger enable
                         // 0: The ADC trigger of output Timer B is disabled.
                         // 1: The ADC trigger of output Timer B is enabled.
        TBEVENT = 1 << 10, // GPTM Timer B event mode
                           // 0x0: Positive edge
                           // 0x1: Negative edge
                           // 0x2: Reserved
                           // 0x3: Both edges RW 0x0
        TBSTALL = 1 << 9, // GPTM Timer B stall enable
                          // 0: Timer B continues counting while the processor is halted by the
                          // debugger.
                          // 1: Timer B freezes counting while the processor is halted by the
                          // debugger. RW 0
        TBEN = 1 << 8, // GPTM Timer B enable
                       // 0: Timer B is disabled.
                       // 1: Timer B is enabled and begins counting or the capture logic is
                       // enabled based on the GPTMCFG register. RW 0
        TAPWML = 1 << 6, // GPTM Timer A PWM output level
                         // 0: Output is unaffected.
                         // 1: Output is inverted. RW 0
        TAOTE = 1 << 5, // GPTM Timer A output trigger enable
                        // 0: The ADC trigger of output Timer A is disabled.
                        // 1: The ADC trigger of output Timer A is enabled. RW 0
        TAEVENT = 1 << 2, // GPTM Timer A event mode
                          // 0x0: Positive edge
                          // 0x1: Negative edge
                          // 0x2: Reserved
                          // 0x3: Both edges RW 0x0
        TASTALL = 1 << 1, // GPTM Timer A stall enable
                          // 0: Timer A continues counting while the processor is halted by the
                          // debugger.
                          // 1: Timer A freezes counting while the processor is halted by the
                          // debugger. RW 0
        TAEN_GPTM = 1 << 0, // GPTM Timer A enable
                       // 0: Timer A is disabled.
                       // 1: Timer A is enabled and begins counting or the capture logic is
                       // enabled based on the GPTMCFG register.
    };

    enum TAMR 
    {
        TAPLO = 1 << 11, // Legacy PWM operation
                         // 0: Legacy operation
                         // 1: CCP is set to 1 on time-out. RW 0
        TAMRSU = 1 << 10, // Timer A match register update mode
                          // 0: Update GPTMAMATCHR and GPTMAPR if used on the next
                          // cycle.
                          // 1: Update GPTMAMATCHR and GPTMAPR if used on the next
                          // time-out. If the timer is disabled (TAEN is clear) when this bit is set,
                          // GPTMTAMATCHR and GPTMTAPR are updated when the timer is
                          // enabled. If the timer is stalled (TASTALL is set), GPTMTAMATCHR
                          // and GPTMTAPR are updated according to the configuration of this
                          // bit. RW 0
        TAPWMIE = 1 << 9, // GPTM Timer A PWM interrupt enable
                          // This bit enables interrupts in PWM mode on rising, falling, or both
                          // edges of the CCP output.
                          // 0: Interrupt is disabled.
                          // 1: Interrupt is enabled.
                          // This bit is valid only in PWM mode. RW 0
        TAILD = 1 << 8, // GPTM Timer A PWM interval load write
                        // 0: Update the GPTMTAR register with the value in the GPTMTAILR
                        // register on the next cycle. If the prescaler is used, update the
                        // GPTMTAPS register with the value in the GPTMTAPR register on
                        // the next cycle.
                        // 1: Update the GPTMTAR register with the value in the GPTMTAILR
                        // register on the next cycle. If the prescaler is used, update the
                        // GPTMTAPS register with the value in the GPTMTAPR register on
                        // the next time-out. RW 0
        TASNAPS = 1 << 7, // GPTM Timer A snap-shot mode
                          // 0: Snap-shot mode is disabled.
                          // 1: If Timer A is configured in periodic mode, the actual free-running
                          // value of Timer A is loaded at the time-out event into the GPTM
                          // Timer A (GPTMTAR) register. RW 0
        TAWOT = 1 << 6, // GPTM Timer A wait-on-trigger
                        // 0: Timer A begins counting as soon as it is enabled.
                        // 1: If Timer A is enabled (TAEN is set in the GPTMCTL register),
                        // Timer A does not begin counting until it receives a trigger from the
                        // Timer in the previous position in the daisy-chain. This bit must be
                        // clear for GP Timer module 0, Timer A. RW 0
        TAMIE = 1 << 5, // GPTM Timer A match interrupt enable
                        // 0: The match interrupt is disabled.
                        // 1: An interrupt is generated when the match value in the
                        // GPTMTAMATCHR register is reached in the one-shot and periodic
                        // modes. RW 0
        TACDIR = 1 << 4, // GPTM Timer A count direction
                         // 0: The timer counts down.
                         // 1: The timer counts up. When counting up, the timer starts from a
                         // value of 0x0. RW 0
        TAAMS = 1 << 3, // GPTM Timer A alternate mode
                        // 0: Capture mode is enabled.
                        // 1: PWM mode is enabled.
                        // Note: To enable PWM mode, the TACM bit must be cleared and the
                        // TAMR field must be configured to 0x2. RW 0
        TACMR = 1 << 2, // GPTM Timer A capture mode
                        // 0: Edge-count mode
                        // 1: Edge-time mode
        TAMR_TAMR = 1 << 0, // GPTM Timer A mode
                            // 0x0: Reserved
                            // 0x1: One-shot mode
                            // 0x2: Periodic mode
                            // 0x3: Capture mode
                            // The timer mode is based on the timer configuration defined by bits
                            // [2:0] in the GPTMCFG register.
    };
};

class eMote3_GPTM : private CC2538_GPTIMER, private Cortex_M_Model
{
public:
    const static unsigned int CLOCK = Traits<CPU>::CLOCK;

    static void delay(unsigned int time_microseconds, unsigned int timer = 3)
    {
        eMote3_GPTM g(timer, time_microseconds);
        g.enable();
        while(g.running());
    }

    typedef CPU::Reg32 Reg32;

    eMote3_GPTM(unsigned int which_timer, unsigned int time_microseconds = 1):
        _base(reinterpret_cast<volatile Reg32*>(GPTIMER0_BASE + 0x1000 * (which_timer < 4 ? which_timer : 0)))
    {
        disable();
        Cortex_M_Model::config_GPTM(which_timer);
        reg(CFG) = 0; // 32-bit timer
        reg(TAMR) = 1; // One-shot
        set(time_microseconds);
    }
    ~eMote3_GPTM()
    {
        disable();
    }
    volatile Reg32 read() 
    { 
        return reg(TAV); 
    }
    void set(unsigned int time_microseconds)
    {
        reg(TAILR) = time_microseconds * (CLOCK / 1000000);
    }
    void disable()
    {
        reg(CTL) &= ~TAEN_GPTM; // Disable timer A
    }
    void enable()
    {
        reg(CC2538_GPTIMER::ICR) = -1; // Clear interrupts
        reg(CTL) |= TAEN_GPTM; // Enable timer A
    }
    volatile bool running()
    {
        return !reg(RIS);
    }
protected:
    volatile Reg32 & reg(unsigned int o) { return _base[o / sizeof(Reg32)]; }

    volatile Reg32* _base;
};

__END_SYS

#endif
