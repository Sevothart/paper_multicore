/*
*   THIS CLASS CONTAINS METHODS AND ATTRIBUTES TO BUILD THE MESSAGE TO BE SENT OVER THE NETWORK
*   
*
* --> it stores an array with the data to be sent
* --> it builds the message based on the stored data

----------------METHODS
*
* --> build(...): receives a string and build the message to be sent on it
* --> getLength(): receives nothing but returns the length of the stored data array
* --> *getData(): returns the stored data
*/

#ifndef MESSAGES_H_
#define MESSAGES_H_

#include <utility/ostream.h>
#include <machine.h>
#include <machine/cortex_m/emote3_gptm.h>
#include <semaphore.h>

using namespace EPOS;

class MessagesHandler
{
public:
    MessagesHandler(){
        _seq = 0;
        _length = 0;
    }

    void build(char msg[]);

    unsigned int length(){return _length;};

    void data(const unsigned int d[], unsigned int length);

    unsigned int *data(){ return _data; };

    unsigned int _seq;
private:
    static const unsigned int DATA_MAX_LENGTH = 100;

    unsigned int _data[DATA_MAX_LENGTH];
    unsigned int _length;
    Semaphore _semaphore;
};

#endif
