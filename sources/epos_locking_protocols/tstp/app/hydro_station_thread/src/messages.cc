#include "../include/messages.h"

void MessagesHandler::data(const unsigned int d[], unsigned int length){
    _semaphore.p();
    for(unsigned int i = 0; i < length; i++){
        _data[i] = d[i];
        _length = i+1;
        if(i==(DATA_MAX_LENGTH-1)) break;             
    }
    _semaphore.v();
}

void MessagesHandler::build(char msg[]){
    EPOS::OStream x;
    x << "Starting to build length = " << _length << "\n";
    char aux[32] = "";
    strcpy(msg, "data=");
    x << "Message is: " << msg << "\n";
    strcat(msg, "hydro_joi_1,");
    x << "Message is: " << msg << "\n";
    _semaphore.p();
    for(unsigned int i = 0; i < _length; i++){
        aux[utoa(_data[i],(char *)aux)] = '\0';
        strcat(msg,(char *)aux);
        if(i < _length - 1)
            strcat(msg, ",");
        x << "Message is: " << msg << "\n";
    }
    _semaphore.v();
    x << "End of build\n";
}
