// EPOS Trustful SpaceTime Protocol Timekeeper Declarations

#ifndef __tstp_timekeeper_h
#define __tstp_timekeeper_h

#include <system/config.h>

#ifdef __tstp__

#include <machine/nic.h>

__BEGIN_SYS

class TSTP::Timekeeper: private SmartData, private Data_Observer<Buffer>
{
    friend class TSTP;

private:
    static const unsigned int MAX_DRIFT = 500000; // us
#ifdef __ieee802_15_4__
    static const unsigned int NIC_TIMER_INTERRUPT_DELAY = IEEE802_15_4::SHR_SIZE * 1000000 / IEEE802_15_4::BYTE_RATE; // us FIXME: this has to come from NIC
#else
    static const unsigned int NIC_TIMER_INTERRUPT_DELAY = 0; // us
#endif

    typedef NIC<NIC_Family>::Timer::Time_Stamp Time_Stamp;
    typedef NIC<NIC_Family>::Timer::Offset Offset;

public:
    // Epoch Control Message
    class Epoch: public Control
    {
    public:
        Epoch(const Region & dst, const Time & ep = TSTP::_epoch, const Global_Space & coordinates = Locator::absolute(Global_Space(0, 0, 0)))
        : Control(dst, 0, 0, 0, EPOCH), _epoch(ep), _coordinates(coordinates) { }

        Region destination() const { return Region(_origin, _radius, _t1); }
        const Time epoch() const { return _epoch; }
        const Global_Space & coordinates() const { return _coordinates; }

        friend Debug & operator<<(Debug & db, const Epoch & e) {
            db << reinterpret_cast<const Control &>(e) << ",d=" << e.destination() << ",e=" << e._epoch << ",c=" << e._coordinates;
            return db;
        }

    private:
        Time _epoch;
        Global_Space _coordinates;
    } __attribute__((packed));

    // Keep Alive Control Message
    class Keep_Alive: public Control
    {
    public:
        Keep_Alive(): Control(Spacetime(here(), now()), 0, 0, 0, KEEP_ALIVE) {}

        friend Debug & operator<<(Debug & db, const Keep_Alive & k) {
            db << reinterpret_cast<const Control &>(k);
            return db;
        }
    } __attribute__((packed));

public:
    Timekeeper();
    ~Timekeeper();

    static Time_Stamp absolute(const Offset & offset); //{ return _location + coordinates; }
    static Offset relative(const Time_Stamp & ts);// { return coordinates - _location; }

    static Time now() { return NIC<NIC_Family>::Timer::count2us(time_stamp()); }
    static bool synchronized() { return !sync_required(); }

private:
    void update(Data_Observed<Buffer> * obs, Buffer * buf);

    static void marshal(Buffer * buf);

    static Time_Stamp time_stamp() { return NIC<NIC_Family>::Timer::read(); }
    static Time_Stamp sync_period() { return (long long int)(MAX_DRIFT * 1000000) / (NIC<NIC_Family>::Timer::frequency() * NIC<NIC_Family>::Timer::accuracy() / 1000000000LL); } // us
    static bool sync_required() { return (_next_sync == 0) || (time_stamp() >= (_next_sync - sync_period() / 2)); }
    static void keep_alive();

private:
    static volatile Time_Stamp _next_sync;
    static Function_Handler _life_keeper_handler;
    static Alarm * _life_keeper;
};

__END_SYS

#endif

#endif
