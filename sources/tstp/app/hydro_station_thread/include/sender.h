/*
*   THIS CLASS CONTAINS METHODS TO SEND THE DATA OVER THE NETWORK OR STORE IT IN THE FLASH IN CASE IT'S NEEDED
*  
* --> it uses the board port C4 as the power key to the GPRS module
* --> it uses the board port C1 as to get the status of the GPRS module
* --> it uses the UART 1 with a baud rate of 9600 to communicate with the GPRS module
*
----------------METHODS
*
* --> __init(): called to initiate the GPRS module and connect it to the network
* --> verifyFlashAndSend(): is checks if there is any data stored in the flash memory, if there is tries to send it
* --> sendData(...): receives the message to be sent and tries to send it 3 times
* --> trySendingAndStore(): tries to send the message using [sendData(...)], if trial is not successfull store it on the flash
* --> __initNetwork(): send commands to the GPRS module to initialize
* --> __initConfig(): send commands to the GPRS module to initialize
* --> verifyAndSetCurrentFlashAddress(): check what's the current flash address to be read or written
*/

#ifndef SENDER_H_
#define SENDER_H_

#include "flash.h"
#include "interface.h"
#include "messages.h"
//#include "watchdog.h"

#include <alarm.h>
#include <machine.h>
#include <gpio.h>
#include <uart.h>
#include <machine/cortex_m/emote3_gprs.h>
#include <machine/cortex_m/emote3_gptm.h>

using namespace EPOS;

class Sender{
    static const auto DATA_SERVER = "http://lishascada.ddns.net:80/hidro";
    static const auto EMOTEGPTMSHORTDELAY = 2000000u;
    static const auto EMOTEGPTMLONGDELAY = 5000000u;
    static const auto RESPONSETIMEOUT = 3000000u;

    static const unsigned int FLASH_CURRENT_VALUE = 128 * 1024; //flash start address is 128k. This address will storage the current flash address that is being written
    static const unsigned int FLASH_START_ADDRESS = FLASH_CURRENT_VALUE + sizeof(unsigned int); //flash start address for storage (128k + 4)
    static const unsigned int FLASH_DATA_SIZE = 256 * 1024 / 4;  //65536 is the maximum number of words to be written to the flash (256k of storage)

public:
    Sender(Interface *x, MessagesHandler *m) : _interface(x), _msg(m) {

        _initialized = true;

        verify_and_set_current_flash_address();

    	_pwrkey = new EPOS::GPIO{'C', 4, EPOS::GPIO::OUTPUT};
    	_status = new EPOS::GPIO{'C', 1, EPOS::GPIO::INPUT};
    	_uart = new EPOS::UART{9600, 8, 0, 1, 1};

    	_gprs = new EPOS::eMote3_GPRS{*_pwrkey, *_status, *_uart};
    	_interface->print_message(Interface::MESSAGE::GPRSCREATED, _status->get());

        _gprs->use_dns();
    }
    
    bool init();
    void verify_flash_and_send();
    int try_sending_and_store();
    int unsent_messages(){ return _unsent_messages; }
    static int signal_strength();

private:
    bool init_network();
    bool init_config();
    int verify_and_set_current_flash_address();
    int send_data(const char msg[]);

private:
    static EPOS::eMote3_GPRS *_gprs;
    EPOS::GPIO *_pwrkey;
    EPOS::GPIO *_status;
    EPOS::UART *_uart;
    Interface *_interface;
    MessagesHandler *_msg;
    volatile unsigned int _current_flash_address;
    int _unsent_messages;
    static bool _initialized;
};

#endif

