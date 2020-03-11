// EPOS Trustful SpaceTime Protocol Declarations

#include <system/config.h>

#ifdef __tstp__

#ifndef __tstp_common_h
#define __tstp_common_h

#include <network/ethernet.h>
#include <network/ieee802_15_4.h>
#define __nic_common_only__
#include <machine/nic.h>
#undef __nic_common_only__
#include <smartdata.h>
#include <utility/hash.h>
#include <utility/string.h>
#include <utility/array.h>

__BEGIN_SYS

class TSTP: private Traits<TSTP>::NIC_Family, private SmartData, private NIC<Traits<TSTP>::NIC_Family>::Observer
{
    friend class Network_Common;
    friend class Epoch;

private:
    static const bool drop_expired = true;

public:
    // Parts
    typedef Traits<TSTP>::NIC_Family NIC_Family;
    typedef Traits<Traits<TSTP>::NIC_Family>::DEVICES::Get<0>::Result _NIC;
    template<typename Radio, bool duty_cycling> class MAC;
    class Router;
    class Locator;
    class Timekeeper;
    class Manager;
    class Security;

    // Imports
    using typename NIC_Family::Buffer;
    using NIC_Family::CRC;
    using SmartData::Header;
    typedef Data_Observer<Buffer, Unit> Observer;
    typedef Data_Observed<Buffer, Unit> Observed;

    // Packet Id
    typedef unsigned short Packet_Id;

    // Trailer
    class Trailer
    {
    public:
        Trailer(): _id(0), _crc(0) {}

        friend Debug & operator<<(Debug & db, const Trailer & t) {
            db << "{id=" << t._id << ",crc=" << t._crc << "}";
            return db;
        }

    protected:
        Packet_Id _id;
        CRC _crc;
    } __attribute__((packed));

    // Packet Data
    static const unsigned int MTU = NIC_Family::MTU - sizeof(NIC_Family::Header) - sizeof(Header) - sizeof(Trailer);
    struct Data
    {
    protected:
        unsigned char _data[MTU];
    };

    // Packet
    // Each TSTP message is encapsulated in a single package. TSTP does not need nor supports fragmentation.
    class Packet: public Header, public Data, public Trailer
    {
    public:
        Packet() {}

        Header * header() { return this; }

        void identify() {
            _id = 0;
            const Packet_Id * aux = reinterpret_cast<const Packet_Id *>(this);
            for(unsigned int i = 0; i < 5; i++)
                _id ^= aux[i + 1];
            _id &= 0xffff;
        }

        template<typename T>
        T * data() { return reinterpret_cast<T *>(&_data); }

        friend Debug & operator<<(Debug & db, const Packet & p);
    } __attribute__((packed));

    // This field is packed first and matches the Frame Type field in the Frame Control in IEEE 802.15.4 MAC.
    // A version number above 4 renders TSTP into the reserved frame type zone and should avoid interference.
    enum Version {
        V0 = 4
    };

protected:
    template<unsigned int UNIT = 0>
    TSTP(unsigned int nic = UNIT);

public:
    ~TSTP();

    static Buffer * alloc(unsigned int size);
    static int send(Buffer * buf);

    // Local Space-Time (network scope, sink at center)
    static Space here();
    static Time now();

    static Time local(const Time & global) { return global - _epoch; }
    static Space local(Global_Space global);
    static Space sink() { return Space(0, 0, 0); }

    // Global Space-Time
    static Global_Space absolute(const Space & s);
    static Time absolute(const Time & t) {
        if((t == static_cast<Time>(-1)) || (t == 0))
           return t;
        return _epoch + t;
    }
    static void epoch(const Time & t) { _epoch = t - now(); }

    static void attach(Data_Observer<Buffer> * part) { _parts.attach(part); }
    static void detach(Data_Observer<Buffer> * part) { _parts.detach(part); }
    static bool notify(Buffer * buf) { return _parts.notify(buf); }

    static void attach(Data_Observer<Buffer, Unit> * sd, const Unit & unit) { _clients.attach(sd, unit); }
    static void detach(Data_Observer<Buffer, Unit> * sd, const Unit & unit) { _clients.detach(sd, unit); }
    static bool notify(const Unit & unit, Buffer * buf) { return _clients.notify(unit, buf); }

    friend Debug & operator<<(Debug & db, const Packet & p);

private:
    void update(NIC_Family::Observed * obs, const Protocol & prot, Buffer * buf);

    static void marshal(Buffer * buf);

    static void init(unsigned int unit);

private:
    static Security * _security;
    static Timekeeper * _timekeeper;
    static Locator * _locator;
    static Router * _router;
    static Manager * _manager;

    static Time _epoch;

    static _NIC * _nic;
    static Data_Observed<Buffer> _parts;
    static Data_Observed<Buffer, Unit> _clients;
};

__END_SYS

#endif

#if !defined(__tstp_h) && !defined(__tstp_common_only__)
#define __tstp_h

#include "mac.h"
#include "security.h"
#include "locator.h"
#include "timekeeper.h"
#include "router.h"
#include "manager.h"

__BEGIN_SYS

inline TSTP::Buffer * TSTP::alloc(unsigned int size)
{
    return _nic->alloc(Address::BROADCAST, PROTO_TSTP, 0, 0, size);
}

inline int TSTP::send(TSTP::Buffer * buf)
{
    db<TSTP>(TRC) << "TSTP::send(buf=" << buf << ")" << endl;
    marshal(buf);
    return _nic->send(buf);
}

inline TSTP::Space TSTP::here() {
    return Locator::here();
}

inline TSTP::Space TSTP::local(TSTP::Global_Space gs) {
    gs -= Locator::absolute(Space(0, 0, 0));
    return Space(gs.x, gs.y, gs.z);
}

inline TSTP::Global_Space TSTP::absolute(const TSTP::Space & s) {
    return Locator::absolute(Global_Space(s.x, s.y, s.z));
}

inline TSTP::Time TSTP::now() {
    return Timekeeper::now();
}

inline Debug & operator<<(Debug & db, const TSTP::Packet & p)
{
    switch(const_cast<TSTP::Packet *>(&p)->header()->type()) {
    case TSTP::INTEREST:
        db << reinterpret_cast<const TSTP::Interest &>(p);
        break;
    case TSTP::RESPONSE:
        db << reinterpret_cast<const TSTP::Response &>(p);
        break;
    case TSTP::COMMAND:
        db << reinterpret_cast<const TSTP::Command &>(p);
        break;
    case TSTP::CONTROL:
        switch(const_cast<TSTP::Packet *>(&p)->header()->subtype()) {
        case TSTP::DH_RESPONSE:
            db << reinterpret_cast<const TSTP::Security::DH_Response &>(p);
            break;
        case TSTP::AUTH_REQUEST:
            db << reinterpret_cast<const TSTP::Security::Auth_Request &>(p);
            break;
        case TSTP::DH_REQUEST:
            db << reinterpret_cast<const TSTP::Security::DH_Request &>(p);
            break;
        case TSTP::AUTH_GRANTED:
            db << reinterpret_cast<const TSTP::Security::Auth_Granted &>(p);
            break;
        case TSTP::REPORT:
            db << reinterpret_cast<const TSTP::Security::Report &>(p);
            break;
        case TSTP::KEEP_ALIVE:
            db << reinterpret_cast<const TSTP::Timekeeper::Keep_Alive &>(p);
            break;
        case TSTP::EPOCH:
            db << reinterpret_cast<const TSTP::Timekeeper::Epoch &>(p);
            break;
//        case TSTP::MODEL:
//            db << reinterpret_cast<const TSTP::Model &>(p);
//            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
    db << "}";
    return db;
};

__END_SYS

#endif

#endif
