#ifndef __raspberry_pi3_gpio_h
#define __raspberry_pi3_gpio_h

#include <machine/gpio.h>
#include <machine/cortex/engine/cortex_a53/bcm_gpio.h>
#include <architecture/cpu.h>

__BEGIN_SYS

class GPIO_Engine: public GPIO_Common
{
protected:
    static const unsigned int UNITS = Traits<GPIO>::UNITS;

public:
    GPIO_Engine(const Port & port, const Pin & pin, const Direction & dir, const Pull & p, const Edge & int_edge)
    {
        _port = 4 * port;
        _pin_mask = (10 * port) + pin;

        direction(dir);
        pull(p);
    }

    bool get() {
        assert( _direction == Direction::IN || _direction == Direction::INOUT );
        return _gpio->get( _pin_mask );
    }

    void set() {
        assert( _direction == Direction::OUT || _direction == Direction::INOUT );

        if( _direction == Direction::INOUT)
            _gpio->direction( _pin_mask, Direction::OUT, _port);
        _gpio->set( _pin_mask );
        if(_direction == INOUT)
            _gpio->direction(_pin_mask, Direction::IN, _port);
    }
    void clear(){ _gpio->clear( _pin_mask ); }

    void direction( const Direction & dir ){
        _direction = dir;
        _gpio->direction(_pin_mask, _direction, _port);
    }

    void select_pin_function(const int & func){ _gpio->select_pin_function(func, _pin_mask, _port); }
    void pull( const Pull & p){ _gpio->pull( _pin_mask, p ); }

private:
    Port _port;
    Pin _pin_mask;
    Direction _direction;
    BCM_GPIO * _gpio;
};

__END_SYS
#endif