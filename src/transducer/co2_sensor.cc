// EPOS CO2 Concentration Smart Transducer Implementation

#include <system/config.h>

#ifdef __CO2_SENSOR_H

#include <transducer.h>

__BEGIN_SYS

UART CO2_Sensor::_uart(Traits<UART>::CM1101_UNIT, UART_BAUD_RATE, UART_DATA_BITS, UART_PARITY, UART_STOP_BITS);

__END_SYS

#endif
