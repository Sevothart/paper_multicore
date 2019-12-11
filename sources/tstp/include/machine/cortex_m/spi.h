// EPOS Cortex_M SPI Mediator Declarations

#include __MODEL_H

#ifndef __cortex_m_spi_h__
#define __cortex_m_spi_h__

#include <cpu.h>
#include <spi.h>
#include <machine/cortex_m/emote3.h>
#include <machine/cortex_m/emote3_ssi.h>
#include <system/traits.h>

__BEGIN_SYS

class Cortex_M_SPI: public SPI_Common, public Cortex_M_Model_SPI
{
private:
    typedef Cortex_M_Model_SPI Engine;

public:

    static const unsigned int CLOCK = Traits<CPU>::CLOCK;

    using Cortex_M_Model_SPI::SSI_Frame_Format;
    using Cortex_M_Model::SSI_Mode;

    typedef Cortex_M_Model_SPI::SSI_Frame_Format SPI_Frame_Format;
    typedef Cortex_M_Model::SSI_Mode SPI_Mode;

    Cortex_M_SPI() : Engine(0) {
    }

    Cortex_M_SPI(Reg32 unit, Reg32 clock, SPI_Frame_Format protocol, SPI_Mode mode, Reg32 bit_rate, Reg32 data_width) :
        Engine(unit, clock, protocol, mode, bit_rate, data_width)
    {
    }

    //This method configures the synchronous serial interface.  It sets
    // the SSI protocol, mode of operation, bit rate, and data width.
    // data_width must be a value between 4 and 16, inclusive
    void config(Reg32 clock, SPI_Frame_Format protocol, SPI_Mode mode, Reg32 bit_rate, Reg32 data_width) {
        Engine::config(clock, protocol, mode, bit_rate, data_width);
    }

    void disable() { Engine::disable(); }
    void enable() { Engine::enable(); }
    void int_enable(Interrupt_Flag flag) { Engine::int_enable(flag); }
    void int_disable(Interrupt_Flag flag) { Engine::int_disable(flag); }
    void put_data(Reg32 data) { Engine::put_data(data); }
    Reg32 put_data_non_blocking(Reg32 data) { return Engine::put_data_non_blocking(data); }
    Reg32 get_data() { return Engine::get_data(); }
    Reg32 get_data_non_blocking() { return Engine::get_data_non_blocking(); }
    bool is_busy() { return Engine::is_busy(); }
    void clock_source(Clock_Source source) { Engine::clock_source(source); }
    Reg32 clock_source() { return Engine::clock_source(); }
};

__END_SYS

#endif
