// EPOS IEEE 802.15.4 MAC Declarations

#ifndef __ieee802_15_4_mac_h
#define __ieee802_15_4_mac_h

#include <utility/random.h>
#include <utility/math.h>
#include <ieee802_15_4.h>

__BEGIN_SYS

template<typename Radio>
class IEEE802_15_4_MAC: public IEEE802_15_4, public IEEE802_15_4::Observed, public Radio
{
private:
    static const unsigned int CSMA_CA_MIN_BACKOFF_EXPONENT = 3;
    static const unsigned int CSMA_CA_MAX_BACKOFF_EXPONENT = 5;
    static const unsigned int CSMA_CA_UNIT_BACKOFF_PERIOD = 320; // us
    static const unsigned int CSMA_CA_RETRIES = Traits<_API::ELP>::RETRIES > 4 ? 4 : Traits<_API::ELP>::RETRIES;

    static const unsigned int ACK_TIMEOUT = 3 * 832; // us

    static const bool acknowledged = Traits<_API::ELP>::acknowledged;
    static const bool auto_listen = !Traits<_API::ELP>::synchronous; // TODO: add in all traits files

public:
    using IEEE802_15_4::Address;

protected:
    IEEE802_15_4_MAC() {
        Radio::promiscuous(false);
        if(auto_listen) {
            Radio::power(Power_Mode::FULL);
            Radio::listen();
        } else {
            Radio::power(Power_Mode::SLEEP);
        }
    }

public:
    unsigned int marshal(Buffer * buf, const Address & src, const Address & dst, const Type & type, const void * data, unsigned int size) {
        if(size > Frame::MTU)
            size = Frame::MTU;
        Frame * frame = new (buf->frame()) Frame(type, src, dst, size);
        frame->ack_request(acknowledged && dst != broadcast());
        memcpy(frame->data<void>(), data, size);
        buf->size(size);
        return size;
    }

    unsigned int unmarshal(Buffer * buf, Address * src, Address * dst, Type * type, void * data, unsigned int size) {
        Frame * frame = reinterpret_cast<Frame *>(buf->frame());
        unsigned int data_size = frame->length() - sizeof(Header) - sizeof(CRC) + sizeof(Phy_Header); // Phy_Header is included in Header, but is already discounted in frame_length
        if(size > data_size)
            size = data_size;
        *src = frame->src();
        *dst = frame->dst();
        *type = frame->type();
        memcpy(data, frame->data<void>(), size);
        return size;
    }

    int send(Buffer * buf) {
        const bool do_ack = acknowledged && reinterpret_cast<Frame *>(buf->frame())->ack_request();

        Radio::power(Power_Mode::LIGHT);
        Radio::copy_to_nic(buf->frame());
        bool sent, ack_ok;
        ack_ok = sent = backoff_and_send();

        if(do_ack) {
            if(sent) {
                while(!Radio::tx_done());
                Radio::power(Power_Mode::FULL);
                ack_ok = Radio::wait_for_ack(ACK_TIMEOUT);
            }

            for(unsigned int i = 0; !ack_ok && (i < CSMA_CA_RETRIES); i++) {
                db<IEEE802_15_4_MAC>(TRC) << "IEEE802_15_4_MAC::retransmitting" << endl;
                Radio::power(Power_Mode::LIGHT);
                ack_ok = sent = backoff_and_send();
                if(sent) {
                    while(!Radio::tx_done());
                    Radio::power(Power_Mode::FULL);
                    ack_ok = Radio::wait_for_ack(ACK_TIMEOUT);
                }
                if(ack_ok)
                    break;
            }
        } else if(sent)
            while(!Radio::tx_done());

        if(auto_listen) {
            Radio::power(Power_Mode::FULL);
            Radio::listen();
        } else
            Radio::power(Power_Mode::OFF);

        return ack_ok ? buf->size() : 0;
    }

    void receive() {
        if(!auto_listen) {
            Radio::power(Power_Mode::FULL);
            Radio::listen();
        }
        while(!Radio::rx_done());
        if(!auto_listen) {
            Radio::power(Power_Mode::OFF);
        }
    }

    bool filter() { return Radio::filter(); }

    void copy_from_nic(Buffer * buf) {
        IEEE802_15_4::Phy_Frame * frame = buf->frame();
        Radio::copy_from_nic(frame);
        buf->size(frame->length() - sizeof(Header) + sizeof(Phy_Header) - sizeof(CRC)); // Phy_Header is included in Header, but is already discounted in frame_length
    }

private:
    bool backoff_and_send() {
        unsigned int exp = CSMA_CA_MIN_BACKOFF_EXPONENT;
        unsigned int backoff = pow(2, exp);

        unsigned int retry = 0;
        for(; (retry < CSMA_CA_RETRIES) ; retry++) {
            unsigned int time = (Random::random() % backoff) * CSMA_CA_UNIT_BACKOFF_PERIOD;

            if(Radio::cca(time) && Radio::transmit())
                break; // Success

            if(exp < CSMA_CA_MAX_BACKOFF_EXPONENT) {
                exp++;
                backoff *= 2;
            }
        }
        return (retry < CSMA_CA_RETRIES);
    }
};

__END_SYS

#endif
