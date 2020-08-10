// EPOS Paspberry Pi3 (ARM Cortex-A53) UART Mediator Declarations

#ifndef __raspberry_pi3_uart_h
#define __raspberry_pi3_uart_h

#include <machine/uart.h>
#include <machine/cortex/engine/cortex_a53/bcm_uart.h>
#include <system/memory_map.h>

__BEGIN_SYS

class UART_Engine: public UART_Common
{
private:
    static const unsigned int UNITS = Traits<UART>::UNITS;

public:
    UART_Engine(unsigned int unit, unsigned int baud_rate, unsigned int data_bits, unsigned int parity, unsigned int stop_bits)
    : _unit(unit), _uart(new(reinterpret_cast<void *>(Memory_Map::/* FIXME: cannot instantiate an UART ona GPIO */ xxxx GPIO_BASE)) BCM_UART) {
        assert(unit < UNITS);
        power(FULL);  // physically enable the UART in SysCtrl before configuring it
        config(baud_rate, data_bits, parity, stop_bits);
    }

    void config(unsigned int baud_rate, unsigned int data_bits, unsigned int parity, unsigned int stop_bits) {
        _uart->disable();
        _uart->config(baud_rate, data_bits, parity, stop_bits);
        _uart->enable();
    }

    void config(unsigned int * baud_rate, unsigned int * data_bits, unsigned int * parity, unsigned int * stop_bits) {
        _uart->config(baud_rate, data_bits, parity, stop_bits);
    }

    char rxd() { return _uart->rxd(); }
    void txd(char c) { return _uart->txd(c); }

    bool rxd_ok() { return _uart->rxd_ok(); }
    bool txd_ok() { return _uart->txd_ok(); }

    bool rxd_full() { return _uart->rxd_full(); }
    bool txd_empty() { return _uart->txd_empty(); }

    void int_enable(bool receive = true, bool send = true, bool line = true, bool modem = true) {
        _uart->int_enable(receive, send, line, modem);
    }
    void int_disable(bool receive = true, bool send = true, bool line = true, bool modem = true) {
        _uart->int_disable(receive, send, line, modem);
    }

    void reset() { _uart->reset(); }
    void loopback(bool flag) {} // there is no loopback support on miniUART

    void power(const Power_Mode & mode) {
        switch(mode) {
        case ENROLL:
            break;
        case DISMISS:
            break;
        case SAME:
            break;
        case FULL:
        case LIGHT:
        case SLEEP:
            // TODO: clock_uart(mode)
            break;
        case OFF:
            break;
        }
    }

    static void init() {}

private:
    unsigned int _unit;
    BCM_UART * _uart;
};

__END_SYS

#endif
