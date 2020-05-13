// EPOS ARMv8 CPU Mediator Declarations

#ifndef __armv8_h
#define __armv8_h

#include <architecture/cpu.h>
#include <architecture/armv7/armv7_cpu.h>

__BEGIN_SYS

class ARMv8_A: public ARMv7_A
{
protected:
    ARMv8_A() {};

public:
    static unsigned int cores() {
        // Cortex A53 cannot execute "mrc p15, 4, r0, c15, c0, 0".
        // The amount of cores booted is equal to the Traits value (defined at Raspberry_Pi3::pre_init::pre_init for instance, up to four)
        return Traits<Build>::CPUS; 
    }

    // we need to redefine because we also redefined cores()
    static void smp_barrier(unsigned long cores = cores()) { CPU_Common::smp_barrier<&finc>(cores, id()); }
};

class CPU: public ARMv8_A
{
    friend class Init_System;

private:
    typedef ARMv8_A Base;

public:
    // CPU Native Data Types
    using Base::Reg8;
    using Base::Reg16;
    using Base::Reg32;
    using Base::Reg64;
    using Base::Log_Addr;
    using Base::Phy_Addr;

    // CPU Context
    class Context
    {
    public:
        Context(const Log_Addr & entry, const Log_Addr & exit): _flags(FLAG_DEFAULTS), _lr(exit | (thumb ? 1 : 0)), _pc(entry | (thumb ? 1 : 0)) {}
//        _r0(0), _r1(1), _r2(2), _r3(3), _r4(4), _r5(5), _r6(6), _r7(7), _r8(8), _r9(9), _r10(10), _r11(11), _r12(12),

        void save() volatile  __attribute__ ((naked));
        void load() const volatile;

        friend Debug & operator<<(Debug & db, const Context & c) {
            db << hex
               << "{r0="  << c._r0
               << ",r1="  << c._r1
               << ",r2="  << c._r2
               << ",r3="  << c._r3
               << ",r4="  << c._r4
               << ",r5="  << c._r5
               << ",r6="  << c._r6
               << ",r7="  << c._r7
               << ",r8="  << c._r8
               << ",r9="  << c._r9
               << ",r10=" << c._r10
               << ",r11=" << c._r11
               << ",r12=" << c._r12
               << ",sp="  << &c
               << ",lr="  << c._lr
               << ",pc="  << c._pc
               << ",psr=" << c._flags
               << "}" << dec;
            return db;
        }

    public:
        Reg32 _flags;
        Reg32 _r0;
        Reg32 _r1;
        Reg32 _r2;
        Reg32 _r3;
        Reg32 _r4;
        Reg32 _r5;
        Reg32 _r6;
        Reg32 _r7;
        Reg32 _r8;
        Reg32 _r9;
        Reg32 _r10;
        Reg32 _r11;
        Reg32 _r12;
        Reg32 _lr;
        /*Reg32 _s0;
        Reg32 _s1;
        Reg32 _s2;
        Reg32 _s3;
        Reg32 _s4;
        Reg32 _s5;
        Reg32 _s6;
        Reg32 _s7;
        Reg32 _s8;
        Reg32 _s9;
        Reg32 _s10;
        Reg32 _s11;
        Reg32 _s12;
        Reg32 _s13;
        Reg32 _s14;
        Reg32 _s15;
        Reg32 _s16;
        Reg32 _s17;
        Reg32 _s18;
        Reg32 _s19;
        Reg32 _s20;
        Reg32 _s21;
        Reg32 _s22;
        Reg32 _s23;
        Reg32 _s24;
        Reg32 _s25;
        Reg32 _s26;
        Reg32 _s27;
        Reg32 _s28;
        Reg32 _s29;
        Reg32 _s30;
        Reg32 _s31;*/
        Reg32 _pc;
    };

    // I/O ports
    typedef Reg16 IO_Irq;

    // Interrupt Service Routines
    typedef void (ISR)();

    // Fault Service Routines (exception handlers)
    typedef void (FSR)();

public:
    CPU() {}

    static Hertz clock() { return _cpu_clock; }
    static Hertz bus_clock() { return _bus_clock; }

    using Base::id;
    using Base::cores;
    using Base::flags;
    using Base::smp_barrier;

    using Base::int_enable;
    using Base::int_disable;
    using Base::int_enabled;
    using Base::int_disabled;

    using Base::sp;
    using Base::fr;
    using Base::ip;
    using Base::pdp;

    using Base::tsl;
    using Base::finc;
    using Base::fdec;
    using Base::cas;

    using Base::halt;

    static void switch_context(Context ** o, Context * n) __attribute__ ((naked));

    template<typename ... Tn>
    static Context * init_stack(const Log_Addr & usp, Log_Addr sp, void (* exit)(), int (* entry)(Tn ...), Tn ... an) {
        sp -= sizeof(Context);
        Context * ctx = new(sp) Context(entry, exit);
        init_stack_helper(&ctx->_r0, an ...);
        return ctx;
    }
    template<typename ... Tn>
    static Log_Addr init_user_stack(Log_Addr sp, void (* exit)(), Tn ... an) {
        sp -= sizeof(Context);
        Context * ctx = new(sp) Context(0, exit);
        init_stack_helper(&ctx->_r0, an ...);
        return sp;
    }

    static int syscall(void * message);
    static void syscalled();

    using Base::htole64;
    using Base::htole32;
    using Base::htole16;
    using Base::letoh64;
    using Base::letoh32;
    using Base::letoh16;

    using Base::htobe64;
    using Base::htobe32;
    using Base::htobe16;
    using Base::betoh64;
    using Base::betoh32;
    using Base::betoh16;

    using Base::htonl;
    using Base::htons;
    using Base::ntohl;
    using Base::ntohs;

private:
    template<typename Head, typename ... Tail>
    static void init_stack_helper(Log_Addr sp, Head head, Tail ... tail) {
        *static_cast<Head *>(sp) = head;
        init_stack_helper(sp + sizeof(Head), tail ...);
    }
    static void init_stack_helper(Log_Addr sp) {}

    static void init();

private:
    static unsigned int _cpu_clock;
    static unsigned int _bus_clock;
};

inline CPU::Reg64 htole64(CPU::Reg64 v) { return CPU::htole64(v); }
inline CPU::Reg32 htole32(CPU::Reg32 v) { return CPU::htole32(v); }
inline CPU::Reg16 htole16(CPU::Reg16 v) { return CPU::htole16(v); }
inline CPU::Reg64 letoh64(CPU::Reg64 v) { return CPU::letoh64(v); }
inline CPU::Reg32 letoh32(CPU::Reg32 v) { return CPU::letoh32(v); }
inline CPU::Reg16 letoh16(CPU::Reg16 v) { return CPU::letoh16(v); }

inline CPU::Reg64 htobe64(CPU::Reg64 v) { return CPU::htobe64(v); }
inline CPU::Reg32 htobe32(CPU::Reg32 v) { return CPU::htobe32(v); }
inline CPU::Reg16 htobe16(CPU::Reg16 v) { return CPU::htobe16(v); }
inline CPU::Reg64 betoh64(CPU::Reg64 v) { return CPU::betoh64(v); }
inline CPU::Reg32 betoh32(CPU::Reg32 v) { return CPU::betoh32(v); }
inline CPU::Reg16 betoh16(CPU::Reg16 v) { return CPU::betoh16(v); }

inline CPU::Reg32 htonl(CPU::Reg32 v) { return CPU::htonl(v); }
inline CPU::Reg16 htons(CPU::Reg16 v) { return CPU::htons(v); }
inline CPU::Reg32 ntohl(CPU::Reg32 v) { return CPU::ntohl(v); }
inline CPU::Reg16 ntohs(CPU::Reg16 v) { return CPU::ntohs(v); }

__END_SYS

#endif
