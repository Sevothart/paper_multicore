// EPOS Acceleration Smart Transducer Implementation

#include <system/config.h>

#ifdef __ACCELEROMETER_H

#include <transducer.h>

__BEGIN_SYS

I2C Accelerometer::_i2c(Traits<I2C>::LSM330_UNIT, I2C::MASTER);

__END_SYS

#endif
