#include "../include/sender.h"

bool Sender::_initialized = false;
EPOS::eMote3_GPRS * Sender::_gprs = 0;

int Sender::send_data(const char msg[]){
    auto send = _gprs->send_http_post(DATA_SERVER, msg, (unsigned int) strlen(msg));
    
    //try to send again for 2 times    
    if(!send) {
        for(unsigned int i = 0; i < 2; i++) {
            _interface->blink_error(Interface::ERROR::TRYINGSENDAGAIN);

            _gprs->off();
            eMote3_GPTM::delay(200000);
            _gprs->on();
                        
            _gprs->use_dns(); // This parameter is (or should be) reset when the module resets.
            eMote3_GPTM::delay(EMOTEGPTMLONGDELAY); // Make sure network is up.            
            
            send = _gprs->send_http_post(DATA_SERVER, msg, (unsigned int) strlen(msg));
            if(send){
                _interface->blink_success(Interface::SUCCESS::MESSAGESENT);
                break;
            }
        }
    }
    return send;
}

int Sender::try_sending_and_store(){
    EPOS::OStream x;
    x << "Try sending....\n";
    char buf[100];
    x << "Buffer created, calling _msg->build\n";
    _msg->build(buf);
    x << "Building: " << buf << "\n";
    eMote3_GPTM::delay(EMOTEGPTMSHORTDELAY);

    auto send = send_data(buf);

    x << "After trying result: " << send << "\n";
    //data has not been sent after 3 attempts, store data into the flash
    if(!send) {
        x << "Storing on flash\n";
        Flash::write(_current_flash_address, _msg->data(), sizeof(unsigned int) * _msg->length());
        _current_flash_address += (_msg->length() * sizeof(unsigned int));
        Flash::write(FLASH_CURRENT_VALUE, (unsigned int *) &_current_flash_address, sizeof(unsigned int));
        _unsent_messages++;
        x << "Unsent messages: " << _unsent_messages << "\n";
    }

    return send;
}

void Sender::verify_flash_and_send(){
    unsigned int data[5];
    unsigned int zero = 0;
    bool data_not_sent = false;

    char buf[100];

    if(_current_flash_address == FLASH_START_ADDRESS)
        return;

    //if _current_flash_address is pointing to some address, there was data in flash that must be sent
    //so, check the flash strating from the first address (FLASH_START_ADDRESS)
    for(unsigned int i = 0; i < FLASH_DATA_SIZE; i += (_msg->length() * sizeof(unsigned int))) {
        data[0] = Flash::read(FLASH_START_ADDRESS + i);
        if(data[0] != 0) {
            for(unsigned int a = 1; a < _msg->length(); a++){
                data[a] = Flash::read(FLASH_START_ADDRESS + (i + sizeof(unsigned int) * a));
            }
    
            _msg->build(buf);
            auto send = send_data(buf);
            
            //if data has been sent, erase it from the flash
            if(send) {
                for(unsigned int a = 0; a < _msg->length(); a++){
                    Flash::write(FLASH_START_ADDRESS + (i + sizeof(unsigned int) * a), &zero, sizeof(unsigned int));
                }
            } else
                data_not_sent = true;

            eMote3_GPTM::delay(EMOTEGPTMLONGDELAY); //5 sec delay before sending again if needed
        } else {
            break;
        }
    }
    //all data in flash has been sent. Restart the counter
    if(!data_not_sent)
        _current_flash_address = FLASH_START_ADDRESS;
}

int Sender::verify_and_set_current_flash_address(){
    int seq = 1;
    _current_flash_address = Flash::read(FLASH_START_ADDRESS);
    if(_current_flash_address == 0) {
        _current_flash_address = FLASH_START_ADDRESS;
        return seq;
    } else {
        for(unsigned int i = 0; i < FLASH_DATA_SIZE; i += (_msg->length() * sizeof(unsigned int))) {
            int tmp = Flash::read(FLASH_START_ADDRESS + i);
            if(tmp == 0) {
                _current_flash_address = FLASH_START_ADDRESS + i;
                return seq + 1;
            }
            seq = tmp;
        }
    }
    return seq + 1;
}


bool Sender::init(){
    eMote3_GPTM::delay(EMOTEGPTMSHORTDELAY / 2);
    if(init_network()){
        if(init_config()){
            _interface->blink_success(Interface::SUCCESS::SIMCARDINITIALIZED);
            _gprs->use_dns();
            return true;
        }else{          
            _interface->blink_error(Interface::ERROR::NOGPRSCARD);
            return false;
        }
    }else{
        _interface->blink_error(Interface::ERROR::NONETWORK);
        return false;
    }
    return false;
}

int Sender::signal_strength(){
    if(_initialized)
        return _gprs->get_signal_quality();
    else
        return 0;
}

bool Sender::init_config(){
    bool res;
    res = false;
    _interface->print_message(Interface::MESSAGE::GPRSSETUP,0);

    eMote3_GPTM::delay(EMOTEGPTMSHORTDELAY);

    while(!res) {
        _gprs->send_command("AT+CGATT=1");
        res = _gprs->await_response("OK", RESPONSETIMEOUT);
        
        _interface->print_message(Interface::MESSAGE::GPRSSTATUS, res);
                
        if(_status->get()==0)
            _gprs->on();
        
        eMote3_GPTM::delay(EMOTEGPTMSHORTDELAY);        
    }
    res = false;
    while(!res) {
        _gprs->send_command("AT+CGACT=1,1");
        res = _gprs->await_response("OK", RESPONSETIMEOUT);
        _interface->print_message(Interface::MESSAGE::GPRSSTATUS, res);
        if(_status->get()==0)
            _gprs->on();
        eMote3_GPTM::delay(EMOTEGPTMSHORTDELAY);
    }
    _interface->print_message(Interface::MESSAGE::GPRSSTATUS, _status->get());
    
    return res;
}

bool Sender::init_network(){
    bool res = _gprs->sim_card_ready();
    unsigned int tmp = 1;
    
    _interface->print_message(Interface::MESSAGE::STARTINGNETWORK, 0);
    eMote3_GPTM::delay(EMOTEGPTMSHORTDELAY);

    while(!res) {
        _interface->print_message(Interface::MESSAGE::SENDCOMMAND, tmp);

        _gprs->send_command("AT+CGDCONT=1,\"IP\",\"gprs.oi.com.br\"");
        res = _gprs->await_response("OK", EMOTEGPTMSHORTDELAY);

        _interface->print_message(Interface::MESSAGE::WAITINGRESPONSE, tmp);

        if(!_status->get())
            _gprs->on();
        eMote3_GPTM::delay(EMOTEGPTMSHORTDELAY);
        
        _gprs->send_command("AT+CGDCONT?");
        res = _gprs->await_response("OK", RESPONSETIMEOUT);

        tmp++;
    }    
    return res;
}
