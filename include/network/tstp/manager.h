// EPOS Trustful SpaceTime Protocol Manager Declarations

#ifndef __tstp_Manager_h
#define __tstp_Manager_h

#include <system/config.h>

#ifdef __tstp__

__BEGIN_SYS

class TSTP::Manager: private SmartData, private Data_Observer<Buffer>
{
    friend class TSTP;

private:
    static const bool forwarder = true;
    static const bool drop_expired = true;

    static const unsigned int RANGE = Traits<TSTP>::RADIO_RANGE;

    // Model Control Message
    class Model: public Control
    {
    protected:
        typedef unsigned char Data[MTU - sizeof(Unit) - sizeof(int) - sizeof(Time) - sizeof(CRC)];
    public:
        template<typename M>
        Model(const Region & dst, const M & model)
        : Control(dst, 0, 0, 0, EPOCH), _model(model) {}

        Region destination() const { return Region(_origin, _radius, _t1); }

        template<typename T>
        T * model() { return reinterpret_cast<T *>(&_model); }
        template<typename T>
        void model(const T & m) { *reinterpret_cast<T *>(&_model) = m; }

        friend Debug & operator<<(Debug & db, const Model & m) {
            db << reinterpret_cast<const Control &>(m) << ",d=" << m.region() << ",e=" << m._model;
            return db;
        }

    private:
        Data _model;
    } __attribute__((packed));

//    // TSTP Smart Data bindings
//    // Predictive (binder between Model messages and Smart Data)
//    class Predictive: public Model
//    {
//    public:
//        template<typename M>
//        Predictive(const M & model, const Unit & unit, const Error & error = 0, const Time & expiry = 0)
//        : Model(model, unit, error, expiry), _size(sizeof(Model) - sizeof(Model::Data) + sizeof(M)) { }
//
//        void t0(const Time & t) { _t0 = t; }
//        void t1(const Time & t) { _t1 = t; }
//        Time t0() const { return _t0; }
//        Time t1() const { return _t1; }
//
//        void respond(const Time & expiry) { send(expiry); }
//
//    private:
//        void send(const Time & expiry) {
//            if((_origin.time() >= _t0) && (_origin.time() <= _t1)) {
//                assert(expiry > now());
//                Buffer * buf = alloc(_size);
//                Model * model = buf->frame()->data<Model>();
//                memcpy(model, this, _size);
//                model->expiry(expiry);
//                marshal(buf);
//                _nic->send(buf);
//            }
//        }
//    private:
//        unsigned int _size;
//        Time _t0;
//        Time _t1;
//    };

public:
    Manager();
    ~Manager();

    bool synchronized();

    static Region destination(Buffer * buf);

private:
    void update(Data_Observed<Buffer> * obs, Buffer * buf);

    bool forward(Buffer * buf) {
        // Not a forwarder
        if(!forwarder)
            return false;

        if(buf->my_distance >= buf->sender_distance) {
            if(!buf->destined_to_me) { // Message comes from node closer to the destination
                return false;
            } else {
                if(buf->frame()->data<Header>()->type() == INTEREST) {
                    return false;
                }
            }
        }

        // Do not forward messages that come from too far away, to avoid radio range asymmetry
        Space::Distance d = here() - buf->frame()->data<Header>()->last_hop();
        if(d > RANGE)
            return false;

        Time expiry = buf->deadline;

        if(expiry == Metadata::INFINITE) // Message will not expire
            return true;
        else if (expiry <= now()) // Expired message
            return !drop_expired;

        Time best_case_delivery_time = (buf->my_distance + RANGE - 1) / RANGE * buf->period;
        Time relative_expiry = expiry - now();

        // Message will expire soon
        if(best_case_delivery_time > relative_expiry)
            return false;

        buf->deadline -= best_case_delivery_time;

        return true;
    }

    // Apply distance routing metric
    static void offset(Buffer * buf) {
        if(buf->is_new)
            buf->offset *= 1 + (buf->my_distance % RANGE);
        else
            // forward() guarantees that my_distance < sender_distance
            buf->offset *= RANGE + buf->my_distance - buf->sender_distance;
        buf->offset /= RANGE;
    }

    static void marshal(Buffer * buf);
};

__END_SYS

#endif

#endif
