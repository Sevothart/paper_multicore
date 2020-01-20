// EPOS Cortex_M TI CC2538 IEEE 802.15.4 NIC Mediator Initialization

#include <system/config.h>
#ifndef __no_networking__

#include <system.h>
#include <machine/cortex_m/machine.h>
#include <machine/cortex_m/cc2538.h>

__BEGIN_SYS

CC2538::CC2538(unsigned int unit): _unit(unit), _send_lock(0), _receive_lock(0)
{
    db<CC2538>(TRC) << "CC2538(unit=" << unit << ")" << endl;

    // Set Address
    ffsm(SHORT_ADDR0) = _address[0];
    ffsm(SHORT_ADDR1) = _address[1];
    _address[0] = ffsm(SHORT_ADDR0);
    _address[1] = ffsm(SHORT_ADDR1);

    // Enable frame filtering
    xreg(FRMFILT0) |= FRAME_FILTER_EN;
    xreg(FRMFILT1) &= ~ACCEPT_FT2_ACK; // ACK frames are handled only when expected

    // Enable automatic source address matching
    xreg(SRCMATCH) |= SRC_MATCH_EN;

    // Enable auto-CRC
    xreg(FRMCTRL0) |= AUTO_CRC;

    channel(15);

    // Enable auto ACK
    xreg(FRMCTRL0) |= AUTO_ACK;

    // Reset statistics
    reset();

    xreg(FRMCTRL1) |= SET_RXENMASK_ON_TX; // Enter receive mode after TX

    // Enable useful device interrupts
    // WARNING: do not enable INT_TXDONE, because _send_and_wait handles it
    xreg(RFIRQM0) = INT_FIFOP;
    xreg(RFIRQM1) = 0;
    xreg(RFERRM) = 0;

    // Issue the listen command
    sfr(RFST) = ISTXON;
}


void CC2538::init(unsigned int unit)
{
    db<Init, CC2538>(TRC) << "Radio::init(unit=" << unit << ")" << endl;

    // Initialize the device
    CC2538 * dev = new (SYSTEM) CC2538(unit);

    // Register the device
    _devices[unit].device = dev;
    _devices[unit].interrupt = IC::irq2int(IC::IRQ_RFTXRX);

    // Install interrupt handler
    IC::int_vector(_devices[unit].interrupt, &int_handler);

    // Enable interrupts for device
    IC::enable(IC::IRQ_RFTXRX);
}

__END_SYS

#endif
