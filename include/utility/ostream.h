// EPOS OStream Interface

#include <system/config.h>

#ifndef __ostream_h
#define __ostream_h

extern "C" {
    void _print_preamble();
    void _print(const char * s);
    void _print_trailler(bool error);
}

__BEGIN_UTIL

class OStream
{
private:
    static constexpr double MAX_DOUBLE = 1.79769e+308;
public:
    struct Begl {};
    struct Endl {};
    struct Hex {};
    struct Dec {};
    struct Oct {};
    struct Bin {};
    struct Err {};

public:
    OStream(): _base(10), _error(false) {}

    OStream & operator<<(const Begl & begl) {
        if(Traits<System>::multicore)
            _print_preamble();
        return *this;
    }

    OStream & operator<<(const Endl & endl) {
        if(Traits<System>::multicore)
            _print_trailler(_error);
        print("\n");
        _base = 10;
        return *this;
    }

    OStream & operator<<(const Hex & hex) {
        _base = 16;
        return *this;
    }
    OStream & operator<<(const Dec & dec) {
        _base = 10;
        return *this;
    }
    OStream & operator<<(const Oct & oct) {
        _base = 8;
        return *this;
    }
    OStream & operator<<(const Bin & bin) {
        _base = 2;
        return *this;
    }

    OStream & operator<<(const Err & err)
    {
        _error = true;
        return *this;
    }

    OStream & operator<<(char c) {
        char buf[2];
        buf[0] = c;
        buf[1] = '\0';
        print(buf);
        return *this;
    }
    OStream & operator<<(unsigned char c) {
        return operator<<(static_cast<unsigned int>(c));
    }

    OStream & operator<<(int i) {
        char buf[64];
        buf[itoa(i, buf)] = '\0';
        print(buf);
        return *this;
    }
    OStream & operator<<(short s) {
        return operator<<(static_cast<int>(s));
    }
    OStream & operator<<(long l) {
        return operator<<(static_cast<int>(l));
    }

    OStream & operator<<(unsigned int u) {
        char buf[64];
        buf[utoa(u, buf)] = '\0';
        print(buf);
        return *this;
    }
    OStream & operator<<(unsigned short s) {
        return operator<<(static_cast<unsigned int>(s));
    }
    OStream & operator<<(unsigned long l) {
        return operator<<(static_cast<unsigned int>(l));
    }

    OStream & operator<<(long long int u) {
        char buf[64];
        buf[llitoa(u, buf)] = '\0';
        print(buf);
        return *this;
    }

    OStream & operator<<(unsigned long long int u) {
        char buf[64];
        buf[llutoa(u, buf)] = '\0';
        print(buf);
        return *this;
    }

    OStream & operator<<(const void * p) {
        char buf[64];
        buf[ptoa(p, buf)] = '\0';
        print(buf);
        return *this;
    }

    OStream & operator<<(const char * s) {
        print(s);
        return *this;
    }

OStream & operator<<(float f) {
        // cast to double in order to print in scientific notation.
        return operator<<(static_cast<double>(f));

        // Simple solution, but works fine
        /*
        if(f < 0.0001f && f > -0.0001f){
            (*this) << "0.0000";
            return *this;
        }
        long long b = (long long) f;
        (*this) << b << ".";
        long long m = (long long) ((f - b)  * 10000);
        (*this) << m;
        return *this;
        */
    }

    // Scientific notation double print
    // Source: https://blog.benoitblanchon.fr/lightweight-float-to-string/ (Access date: 06/02/2020)
    // Author: Benoit Blanchon, 2017.
    OStream & operator<<(double d) {
        if (d < 0.0) {
            *this << '-';
            d = -1 * d;
        }

        if (d == MAX_DOUBLE) {
            *this << "inf";
            return *this;
        }

        unsigned int integralPart, decimalPart;
        short exponent;

        splitFloat(d, integralPart, decimalPart, exponent);

        *this << integralPart;

        if (decimalPart){
            writeDecimals(decimalPart, *this);
        } else {
            *this << ".0";
        }

        if (exponent)
            *this << "e" << exponent;

        return *this;
    }

private:
    // Source: https://blog.benoitblanchon.fr/lightweight-float-to-string/ (Access date: 06/02/2020)
    // Author: Benoit Blanchon, 2017.
    void splitFloat(double value, unsigned int &integralPart,
        unsigned int &decimalPart, short &exponent) {
        exponent = normalizeFloat(value);

        integralPart = (unsigned int)value;
        double remainder = value - integralPart;

        remainder *= 1e9;
        decimalPart = (unsigned int)remainder;  

        // rounding
        remainder -= decimalPart;
        if (remainder >= 0.5) {
            decimalPart++;
            if (decimalPart >= 1000000000) {
                decimalPart = 0;
                integralPart++;
                if (exponent != 0 && integralPart >= 10) {
                    exponent++;
                    integralPart = 1;
                }
            }
        }
    }

    // Source: https://blog.benoitblanchon.fr/lightweight-float-to-string/ (Access date: 06/02/2020)
    // Author: Benoit Blanchon, 2017.
    int normalizeFloat(double& value) {
        const double positiveExpThreshold = 1e7;
        const double negativeExpThreshold = 1e-5;
        int exponent = 0;

        if (value >= positiveExpThreshold) {
            if (value >= 1e256) {
                value /= 1e256;
                exponent += 256;
            }
            if (value >= 1e128) {
                value /= 1e128;
                exponent += 128;
            }
            if (value >= 1e64) {
                value /= 1e64;
                exponent += 64;
            }
            if (value >= 1e32) {
                value /= 1e32;
                exponent += 32;
            }
            if (value >= 1e16) {
                value /= 1e16;
                exponent += 16;
            }
            if (value >= 1e8) {
                value /= 1e8;
                exponent += 8;
            }
            if (value >= 1e4) {
                value /= 1e4;
                exponent += 4;
            }
            if (value >= 1e2) {
                value /= 1e2;
                exponent += 2;
            }
            if (value >= 1e1) {
                value /= 1e1;
                exponent += 1;
            }
        }

        if (value > 0 && value <= negativeExpThreshold) {
            if (value < 1e-255) {
                value *= 1e256;
                exponent -= 256;
            }
            if (value < 1e-127) {
                value *= 1e128;
                exponent -= 128;
            }
            if (value < 1e-63) {
                value *= 1e64;
                exponent -= 64;
            }
            if (value < 1e-31) {
                value *= 1e32;
                exponent -= 32;
            }
            if (value < 1e-15) {
                value *= 1e16;
                exponent -= 16;
            }
            if (value < 1e-7) {
                value *= 1e8;
                exponent -= 8;
            }
            if (value < 1e-3) {
                value *= 1e4;
                exponent -= 4;
            }
            if (value < 1e-1) {
                value *= 1e2;
                exponent -= 2;
            }
            if (value < 1e0) {
                value *= 1e1;
                exponent -= 1;
            }
        }
        return exponent;
    }

    // Source: https://blog.benoitblanchon.fr/lightweight-float-to-string/ (Access date: 06/02/2020)
    // Author: Benoit Blanchon, 2017.
    void writeDecimals(unsigned int value, OStream & s) {
        int width = 9;

        // remove trailing zeros
        while (value % 10 == 0 && width > 0) {
            value /= 10;
            width--;
        }

        char buffer[16];
        char *ptr = buffer + sizeof(buffer) - 1;

        // write the string in reverse order
        *ptr = 0;
        while (width--) {
            *--ptr = value % 10 + '0';
            value /= 10;
        }
        *--ptr = '.';

        // and dump it in the right order
        s << ptr;
    }

    void print(const char * s) { _print(s); }

    int itoa(int v, char * s);
    int utoa(unsigned int v, char * s, unsigned int i = 0);
    int llitoa(long long int v, char * s);
    int llutoa(unsigned long long int v, char * s, unsigned int i = 0);
    int ptoa(const void * p, char * s);

private:
    int _base;
    volatile bool _error;

    static const char _digits[];
};

constexpr OStream::Begl begl;
constexpr OStream::Endl endl;
constexpr OStream::Hex hex;
constexpr OStream::Dec dec;
constexpr OStream::Oct oct;
constexpr OStream::Bin bin;

__END_UTIL

__BEGIN_SYS
extern OStream kout, kerr;
__END_SYS

#endif
