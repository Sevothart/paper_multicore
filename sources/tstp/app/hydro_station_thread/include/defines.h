#ifndef DEFINES_H_
#define DEFINES_H_

const auto MINUTES = 5u;
const auto MINUTE_IN_US = 60000000u;

const auto SENDER_PERIOD  = MINUTE_IN_US * MINUTES;
const auto SENSORS_PERIOD = SENDER_PERIOD - 1000;

#endif
