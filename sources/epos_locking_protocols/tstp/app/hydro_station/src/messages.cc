#include "../include/messages.h"

void MessagesHandler::setData(const unsigned int d[], unsigned int length){      
    for(unsigned int i = 0; i<length; i++){
        data[i] = d[i];
        dataLength = i+1;
        if(i==(DATA_MAX_LENGTH-1)) break;             
    }    
}

void MessagesHandler::build(char msg[]){
    EPOS::OStream x;
    x<<"Starting to build\n";
    char aux[32] = "";
    strcpy(msg,"data=");
    x<<"Message is: "<<msg<<"\n";
    strcat(msg,"hydro_joi_1,");
    x<<"Message is: "<<msg<<"\n";
    for(unsigned int i = 0; i<dataLength; i++){
        aux[utoa(data[i],(char *)aux)] = '\0';
        strcat(msg,(char *)aux);
        if(i<dataLength-1) strcat(msg, ",");
        //eMote3_GPTM::delay(500000);     
        x<<"Message is: "<<msg<<"\n";
    }
    x<<"End of build\n";
}