// EPOS Temperature Smart Transducer Implementation

#include <system/config.h>

#ifdef __THERMOMETER_H

#include <transducer.h>

__BEGIN_SYS

I2C Thermometer::_i2c(Traits<I2C>::SI7020_UNIT);
UART Alternate_Thermometer::_uart(Traits<UART>::CM1101_UNIT);

__END_SYS

#endif
