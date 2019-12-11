#include "hydro_station/include/index.h"

using namespace EPOS;

/*
*   This application uses the following classes:
*       -> Sender: read and store on flash, and send data over the air
*       -> MessagesHandler: store data acquired from the sensors, and builds the message to be sent
*       -> Interface: feedback to the user bliking LED or printing info through the usb
*       -> Sensors: variety of classes to read data from each sensor
*/

/*  Fill message data with acquired data from the sensors
*
*   Receive as parameters: pointer to messageshandler object, level sensor object, turbidity sensor object,
*   pluviometric sensor object, first thermocouple object and second thermocouple object
*
*   This function returns nothing
*/
//inline void fillMessageData(MessagesHandler &m, Level_Sensor &level, Turbidity_Sensor &turb, Pluviometric_Sensor &pluv,ThermoCouple &t1,ThermoCouple &t2,Sender &sender){
inline void fillMessageData(MessagesHandler &m, Level_Sensor &level, Turbidity_Sensor &turb, Pluviometric_Sensor &pluv,Sender &sender){
    unsigned int d[8];
    d[0] = ++m.seq;
    d[1] = level.sample();
    d[2] = turb.sample();
    d[3] = pluv.countAndReset();
    d[4] = 1;
    d[5] = 0;
    d[6] = 0;
    d[7] = sender.getSignalStrength();
    m.setData(d,8);    
}

int main(){
    //this class contain methods to print messages or blink the board LED
    //if application is going for production, pass true as parameter to the object
    //when is true the object won't print any message and won't blink leds
    Interface interface(true);
    interface.showLife();

    //level sensor objects, lToggle is the relay pin, and lAdc is the ADC conversor
    auto lToggle = GPIO{'b', 0, GPIO::OUTPUT};  
    auto lAdc = ADC{ADC::SINGLE_ENDED_ADC2};
    auto level = Level_Sensor{lAdc, lToggle};
    level.disable();

    //turbidity sensor objects, tToggle is the relay pin
    auto tAdc = ADC{ADC::SINGLE_ENDED_ADC5};
    auto tInfrared = GPIO{'b', 2, GPIO::OUTPUT};
    auto tToggle = GPIO{'b', 3, GPIO::OUTPUT};
    auto turbidity = Turbidity_Sensor{tAdc,tToggle,tInfrared};
    turbidity.disable();
    
    //pluviometric sensor objects, pToggle is the relay pin
    auto pToggle = GPIO{'b', 1, GPIO::OUTPUT};
    auto pInput = GPIO{'a', 3, GPIO::INPUT};
    auto pluviometric = Pluviometric_Sensor{pInput, pToggle};

    //thermocouple objects, they are part of the battery monitor board
    //ThermoCouple batteryTemperature('d',2);
    //ThermoCouple panelTemperature('d',3);

    //messagesHandler object, this object holds sensor read data and builds message to be sent to the server
    MessagesHandler msg;

    //sender object, methods to read from the flash, write on the flash and send data through gprs module
    //  
    //  receives the address of the interface object (to give user a feedback) 
    //  and the messageshandler object (to build the message according to the data acquired)
    Sender sender(&interface, &msg);    
    sender.__init();

    //timer used to count 5 minutes
    eMote3_GPTM timer(1);

    while(1){
        timer.set(MINUTE_IN_US*2);
        timer.enable();

        //read temperature of the battery and panel got from the MAX31855
        //batteryTemperature.readValue();
        //eMote3_GPTM::delay(500000);
        //panelTemperature.readValue();

        //function to set data acquired from the sensors
        //fillMessageData(msg, level, turbidity, pluviometric,batteryTemperature,panelTemperature,sender);
        fillMessageData(msg, level, turbidity, pluviometric,sender);
       
        //check if there is any data on the flash memory, if there is it will send this data
        sender.verifyFlashAndSend();
	    
        //try sensing data acquired for 3 times, if it fails will store on the flash memory
        sender.trySendingAndStore();

        //wait for 5 minutes
        //  PS: a note for the future, check on the m95 datasheet about putting gprs in sleep mode (it will save battery)
        while(timer.running());
        for(int i = 0; i<3; i++){
            eMote3_GPTM::delay(MINUTE_IN_US);            
        }

        //if 5 messages failed to send, it will reboot the system
        if(sender.getUnsentMessages()==5) Machine::reboot();
    }
    return 0;
}
