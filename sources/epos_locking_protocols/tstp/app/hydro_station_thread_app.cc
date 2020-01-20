#include "hydro_station_thread/include/index.h"
#include <periodic_thread.h>

using namespace EPOS;

/*
*   This application uses the following classes:
*       -> Sender: read and store on flash, and send data over the air
*       -> MessagesHandler: store data acquired from the sensors, and builds the message to be sent
*       -> Interface: feedback to the user bliking LED or printing info through the usb
*       -> Sensors: variety of classes to read data from each sensor
*/

int sender(MessagesHandler *msg);
int read_sensor_data(MessagesHandler *msg);
void fill_message_data(MessagesHandler *msg, Level_Sensor & level, Turbidity_Sensor & turb, Pluviometric_Sensor & pluv, Voltages_ADC & bat_monitor);

OStream cout;

int main(){

    eMote3_GPTM::delay(5000000);

    //messagesHandler object, this object holds sensor read data and builds message to be sent to the server
    MessagesHandler msg;

    Periodic_Thread thread_a(RTConf(SENSORS_PERIOD, Periodic_Thread::INFINITE), &read_sensor_data, &msg);
    Periodic_Thread thread_b(RTConf(SENDER_PERIOD, Periodic_Thread::INFINITE), &sender, &msg);

    int status_a = thread_a.join();
    int status_b = thread_b.join();

    return 0;
}

int sender(MessagesHandler *msg)
{
    //this class contain methods to print messages or blink the board LED
    //if application is going for production, pass true as parameter to the object
    //when is true the object won't print any message and won't blink leds

    Interface interface(true);
    interface.show_life();

    /*  sender object, methods to read from the flash, write on the flash and send data through gprs module
     *  receives the address of the interface object (to give user a feedback)
     *  and the messageshandler object (to build the message according to the data acquired)
     */

    Sender sender(&interface, msg);
    sender.init();

    char buf[100];

    while(1) {
        msg->build(buf);
        cout << "Built: " << buf << "\n";

        //check if there is any data on the flash memory, if there is it will send this data
        sender.verify_flash_and_send();

        //try sensing data acquired for 3 times, if it fails will store on the flash memory
        sender.try_sending_and_store();

        //if 5 messages failed to send, it will reboot the system
        //if(sender.unsent_messages() == 5)
        //    Machine::reboot();

        Periodic_Thread::wait_next();
    }
    return 0;
}

int read_sensor_data(MessagesHandler *msg)
{
    //level sensor objects, lToggle is the relay pin, and lAdc is the ADC conversor
    GPIO lToggle = GPIO{'B', 0, GPIO::OUTPUT};
    auto lAdc = ADC{ADC::SINGLE_ENDED_ADC2};
    auto level = Level_Sensor{lAdc, lToggle};
    level.disable();

    //turbidity sensor objects, tToggle is the relay pin
    auto tAdc = ADC{ADC::SINGLE_ENDED_ADC5};
    auto tInfrared = GPIO{'B', 2, GPIO::OUTPUT};
    auto tToggle = GPIO{'B', 3, GPIO::OUTPUT};
    auto turbidity = Turbidity_Sensor{tAdc,tToggle,tInfrared};
    turbidity.disable();

    //pluviometric sensor objects, pToggle is the relay pin
    auto pToggle = GPIO{'B', 1, GPIO::OUTPUT};
    auto pInput = GPIO{'A', 3, GPIO::INPUT};
    auto pluviometric = Pluviometric_Sensor{pInput, pToggle};

    auto bat_monitor = Voltages_ADC{};

    //thermocouple objects, they are part of the battery monitor board
    //ThermoCouple batteryTemperature('D', 2);
    //ThermoCouple panelTemperature('D', 3);

    while(1) {
        //read temperature of the battery and panel got from the MAX31855
        //batteryTemperature.readValue();
        //eMote3_GPTM::delay(500000);
        //panelTemperature.readValue();

        //function to set data acquired from the sensors
        //fillMessageData(msg, level, turbidity, pluviometric, batteryTemperature, panelTemperature);
        fill_message_data(msg, level, turbidity, pluviometric, bat_monitor);
        Periodic_Thread::wait_next();
    }
    return 0;
}

/*  Fill message data with acquired data from the sensors
*
*   Receive as parameters: pointer to messageshandler object, level sensor object, turbidity sensor object,
*   pluviometric sensor object, first thermocouple object and second thermocouple object
*
*   This function returns nothing
*/
//inline void fillMessageData(MessagesHandler &m, Level_Sensor &level, Turbidity_Sensor &turb, Pluviometric_Sensor &pluv,ThermoCouple &t1,ThermoCouple &t2,Sender &sender){
inline void fill_message_data(MessagesHandler *msg, Level_Sensor & level, Turbidity_Sensor & turb, Pluviometric_Sensor & pluv, Voltages_ADC & bat_monitor){
    unsigned int d[7];
    bat_monitor.sample();
    d[0] = ++msg->_seq;
    d[1] = level.sample();
    d[2] = turb.sample();
    d[3] = pluv.countAndReset();
    d[4] = bat_monitor.battery_voltage(); //tensao bateria
    d[5] = bat_monitor.panel_voltage();; //tensao painel
    cout << "\nsignal_strength()...";
    d[6] = Sender::signal_strength(); //intensidade do sinal GPRS
    cout << "done\n";
    //d[7] = ;
    msg->data(d, 7);
    cout << "length = " << msg->length() << endl;
}
