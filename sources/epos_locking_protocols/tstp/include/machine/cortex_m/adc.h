#ifndef __emote3_adc_h_
#define __emote3_adc_h_

#include <adc.h>

__BEGIN_SYS

class Cortex_M_ADC : private ADC_Common {
public:
    typedef CPU::Reg32 Reg32;

    // Parts that do not comply with EPOS' ADC api are commented out right now.

    enum Channel {
        SINGLE_ENDED_ADC0 = 0,
        SINGLE_ENDED_ADC1 = 1,
        SINGLE_ENDED_ADC2 = 2,
        SINGLE_ENDED_ADC3 = 3,
        SINGLE_ENDED_ADC4 = 4,
        SINGLE_ENDED_ADC5 = 5,
        SINGLE_ENDED_ADC6 = 6,
        SINGLE_ENDED_ADC7 = 7,
        /*
        DIFF8 = 8,
        DIFF9 = 9,
        DIFF10 = 10,
        DIFF11 = 11,
        */
        GND = 12, // Non-compliant with EPOS' generic ADC API
        /*
        RESERVED = 13,
        TEMPERATURE = 14,
        AVDD5_3 = 15,
        */
    };

    enum Reference {
        INTERNAL_REF = 0,
        EXTERNAL_REF = 1,  // External reference on AIN7 pin
        SYSTEM_REF = 2,
        /*
        EXTERNAL_DIFF = 3
        */
    };

    enum Decimation {
        D64 = 0,   //  7 bit resolution
        D128 = 1,  //  9 bit resolution
        D256 = 2,  // 10 bit resolution
        D512 = 3   // 12 bit resolution
    };

    Cortex_M_ADC(Channel channel = SINGLE_ENDED_ADC0,
        Reference reference = SYSTEM_REF):
        _channel{channel},
        _reference{reference}
    {}

    // Make a single conversion using the ADC, as documented on page 375 of
    // the user guide. AVDD5 is connected to VDD in the eMote3, so it is
    // the most reliable reference, thus it is the default argument.
    // Using the internal reference yields totally different values, meaning
    // it is different from VDD.
    int read()
    {
        start_single_conversion(_channel, D512, _reference);
        while (!single_conversion_ended());
        return read_single_conversion_result();
    }

private:
    enum {
        ADC_BASE = 0x400D7000
    };

    enum Offset {
        //Register Name      Offset  Type  Width  Reset Value
        ADCCON1           =  0x00, //RW    32     0x00000033
        ADCCON2           =  0x04, //RW    32     0x00000010
        ADCCON3           =  0x08, //RW    32     0x00000000
        ADCL              =  0x0C, //RW    32     0x00000000
        ADCH              =  0x10, //RO    32     0x00000000
    };

    enum Shifts {
        EOC_START = 7,
        SIGN_BITS = sizeof(int)*8 - 14,
        LOW_START = 2,
        LOW_SIZE = 6,
        REF_START = 6,
        DECIMATION_START = 4
    };

    enum Mask {
        ADCCON3_CLEAR = ~0xFF,
        EOC = (1 << EOC_START),
        HIGH = 0xFF,
        LOW = (0x3F << LOW_START)
    };


    static volatile Reg32 & reg(Offset o) { return reinterpret_cast<volatile Reg32 *>(ADC_BASE)[o / sizeof(Reg32)]; }

    static void start_single_conversion(Channel channel, Decimation decimation, Reference reference)
    {
        auto r = reference << REF_START;
        auto d = decimation << DECIMATION_START;
        reg(ADCCON3) = (reg(ADCCON3) & ADCCON3_CLEAR) | r | d | channel;
    }

    static bool single_conversion_ended()
    {
        return reg(ADCCON1) & EOC;
    }

    static int read_single_conversion_result()
    {
        auto whole = int{read_high() | read_low()};
        return (whole << SIGN_BITS) >> SIGN_BITS;  // Extends signal from signal bit.
    }

    static unsigned int read_high()
    {
        auto r = reg(ADCH);
        auto value = (r & HIGH) << LOW_SIZE;
        return value;
    }

    static unsigned int read_low()
    {
        auto r = reg(ADCL);
        auto value = (r & LOW) >> LOW_START;
        return value;
    }

    Channel _channel;
    Reference _reference;
};

__END_SYS

#endif
