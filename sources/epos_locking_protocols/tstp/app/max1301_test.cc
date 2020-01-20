#include "hydro_station_thread/include/sensors.h"
#include <utility/ostream.h>

using namespace EPOS;

OStream cout;

int main(void)
{
    Voltages_ADC sensor;
    while(1) {
        sensor.sample();
        cout << "Bateria = " << sensor.battery_voltage() << " Painel = " << sensor.panel_voltage() << endl;
        eMote3_GPTM::delay(1000000);
    }
}

