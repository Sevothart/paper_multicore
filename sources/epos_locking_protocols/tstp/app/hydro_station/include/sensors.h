#ifndef HS_SENSORS_H_
#define HS_SENSORS_H_

#include <gpio.h>
#include <adc.h>
#include <machine/cortex_m/emote3_gptm.h>

using namespace EPOS;

class Sensor_Base {
public:
    Sensor_Base(GPIO &relay):
        relay(relay)
    {}

    int enable()
    {
        relay.set();
	return 0;
    }

    int disable()
    {
        relay.clear();
	return 0;
    }

protected:
    GPIO &relay;
};

class Analog_Sensor_Base: public Sensor_Base{
public:
    Analog_Sensor_Base(ADC &adc, GPIO &relay):
        Sensor_Base(relay),
        adc(adc)
    {}

protected:
    ADC &adc;
};

class Level_Sensor: public Analog_Sensor_Base {
public:
    Level_Sensor(ADC &adc, GPIO &relay):
        Analog_Sensor_Base{adc, relay}
    {}

    int sample()
    {
        return adc.read();
    }
};

class Turbidity_Sensor: public Analog_Sensor_Base {
public:
    Turbidity_Sensor(ADC &adc, GPIO &relay, GPIO &infrared):
        Analog_Sensor_Base{adc, relay},
        infrared(infrared)
    {}
    
    void __init__(){}

    int sample()
    {
        // Filter daylight as directed by sensor manufacturer
        eMote3_GPTM::delay(250000); // Wait 250 ms before reading
        auto daylight = adc.read();
        eMote3_GPTM::delay(250000); // Wait more 250 ms because we've been told to
        infrared.set();
        eMote3_GPTM::delay(450000); // Wait 200+250 ms before reading again
        auto mixed = adc.read();
        infrared.clear();
        return mixed - daylight;
    }

private:
    GPIO &infrared;
};

class Pluviometric_Sensor: public Sensor_Base {
public:
  Pluviometric_Sensor(GPIO &input, GPIO &relay):
    Sensor_Base(relay),
    input(input),
    _count(0)
  {
    _instance = this;
    
    input.input();
    input.pull_up();
    
    input.handler(&handler);
    input.int_enable(GPIO::FALLING, false);
  }
  
  static void handler(const unsigned int & i)
  {
    if(_instance && !_debouncing){
      _instance->_count++;
      _instance->_debouncing = true;
      eMote3_GPTM timer(0);
      timer.set(1200000);
      timer.enable();
      while(timer.running());
      _instance->_debouncing = false;
    } 
  }
  
  int count()
  {
    return _count;
  }
  
  void resetCount()
  {
    _count = 0;
  }
  
  int countAndReset()
  {
    int ret = _count;  
    _count -= ret;
    return ret;
  }
  
private:
  GPIO &input;
  int _count;
  static bool _debouncing;
  
  // this is needed because we treat the interrupt in a static function
  static Pluviometric_Sensor * _instance;
};



#endif
