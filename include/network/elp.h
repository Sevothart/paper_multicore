// EPOS Light Protocol Declarations

#include <system/config.h>

#ifdef __NIC_H

#ifndef __elp_h
#define __elp_h

#include <machine/nic.h>
#include <utility/binding.h>
#include <system.h>

__BEGIN_SYS

class ELP: private NIC<Traits<ELP>::NIC_Family>::Observer
{
    friend class Network;

private:
    static const unsigned int UNITS = COUNTOF(Traits<ELP>::NICS);

public:
    typedef Traits<ELP>::NIC_Family NIC_Family;
    static const bool connectionless = true;
    static const unsigned int PROTOCOL = NIC_Family::PROTO_ELP;

    typedef unsigned short Port;

    class Address
    {
    public:
        enum Null { NULL = 0xffff };

    public:
        Address() {}
        Address(const Null & null): _mac(NIC_Family::Address::NULL), _port(NULL) {}
        Address(const typename NIC_Family::Address & mac, const Port & port): _mac(mac), _port(port) {}

        const typename NIC_Family::Address & mac() const { return _mac; }
        const Port & port() const { return _port; }

        operator bool() const {
            return (_mac || _port);
        }

        bool operator==(const Address & a) {
            return (_mac == a._mac) && (_port == a._port);
        }

        friend OStream & operator<<(OStream & db, const Address & a) {
            db << a._mac << ":" << hex << a._port;
            return db;
        }

    private:
        typename NIC_Family::Address _mac;
        Port _port;
    };

    typedef NIC_Family::Protocol Protocol;
    typedef NIC_Family::Buffer Buffer;

    typedef Data_Observer<Buffer, Port> Observer;
    typedef Data_Observed<Buffer, Port> Observed;

    class Header {
    public:
        Header(): _from(0), _to(0), _size(0) {}
        Header(const Port & from, const Port & to, unsigned int size): _from(from), _to(to), _size(size) {}

        Port from() const { return _from; }
        void from(const Port & from) { _from = from; }

        Port to() const { return _to; }
        void to(const Address & to){ _to = to; }

        friend OStream & operator<<(OStream & os, const Header & h) {
            os << "{f=" << h._from
                << ",t=" << h._to
                << ",s=" << h._size
                << "}";
            return os;
        }

    private:
        Port _from;
        Port _to;
        unsigned short _size;
    } __attribute__((packed));

    static const unsigned int MTU = NIC<NIC_Family>::MTU -sizeof(NIC_Family::Header) - sizeof(Header);
    typedef unsigned char Data[MTU];

    class Packet: public Header
    {
    public:
        Packet() {}

        Header * header() { return this; }

        template<typename T>
        T * data() { return reinterpret_cast<T *>(&_data); }

        friend Debug & operator<<(Debug & db, const Packet & p) {
            db << "{head=" << reinterpret_cast<const Header &>(p) << ",data=" << p._data << "}";
            return db;
        }

    private:
        Data _data;
    } __attribute__((packed));

    typedef _UTIL::Binding<ELP, Port, NIC<NIC_Family>, NIC_Family::Address> Binding;

protected:
    ELP(NIC<NIC_Family> * nic): _nic(nic) {
        db<ELP>(TRC) << "ELP::ELP(nic=" << _nic << ")" << endl;
        _nic->attach(this, PROTOCOL);
    }

public:
    ~ELP() {
        db<ELP>(TRC) << "ELP::~ELP()" << endl;
        _nic->detach(this, PROTOCOL);
    }

    static int send(const Address & from, const Address & to, const void * data, unsigned int size) {
        db<ELP>(TRC) << "ELP::send(f=" << from << ",t=" << to << ",d=" << data << ",s=" << size << ")" << endl;

        Binding * binding = Binding::get_by_key(from.port());
        if(!binding)
            return 0;

        ELP * elp = binding->a();

        db<ELP>(INF) << "ELP::send:elp=" << elp << ", nic=" << elp->_nic << endl;

        Buffer * buf = elp->_nic->alloc(to.mac(), PROTOCOL, 0, sizeof(Header), sizeof(Data));
        if(!buf)
            return 0;

        Packet * packet = buf->frame()->data<Packet>();
        unsigned int s = (s >= sizeof(Packet)) ? sizeof(Packet) : size;
        memcpy(packet, data, s);

        buf->nic()->send(buf);

        return size;
    }

    static int receive(Buffer * buf, void * data, unsigned int size) {
        db<ELP>(TRC) << "ELP::receive(buf=" << buf << ",d=" << data << ",s=" << size << ")" << endl;

        Packet * packet = buf->frame()->data<Packet>();
        unsigned int s = (size >= sizeof(Packet)) ? sizeof(Packet) : size;
        memcpy(data, packet, s);
        buf->nic()->free(buf);

        return size;
    }

    static int receive(Buffer * buf, Address * from, void * data, unsigned int size) {
        db<ELP>(TRC) << "ELP::receive(buf=" << buf << ",d=" << data << ",s=" << size << ")" << endl;

        Packet * packet = buf->frame()->data<Packet>();
        unsigned int s = (size >= sizeof(Packet)) ? sizeof(Packet) : size;
        *from = Address(buf->frame()->src(), packet->from());
        memcpy(data, packet, s);
        buf->nic()->free(buf);

        return size;
    }

    NIC<NIC_Family> * nic() { return _nic; }

    static void attach(Observer * obs, const Address & address) { Binding::rebind(Address::NULL, 0); _observed.attach(obs, 0); }
    static void detach(Observer * obs, const Address & address) { _observed.detach(obs, 0); Binding::unbind(0); }

private:
    void update(NIC_Family::Observed * obs, const Protocol & prot, Buffer * buf) {
        if(!notify(0, buf))
            _nic->free(buf);
    }

    static bool notify(const Port & port, Buffer * buf) { return _observed.notify(0, buf); }

    template<unsigned int UNIT>
    inline static void init_helper() {
        NIC<NIC_Family> * nic = Traits<NIC_Family>::DEVICES::Get<Traits<ELP>::NICS[UNIT]>::Result::get(Traits<ELP>::NICS[UNIT]);
        ELP * elp = new (SYSTEM) ELP(nic);
        new (SYSTEM) Binding(elp, Port(Address::NULL), nic, nic->address());

        init_helper<UNIT + 1>();
    };

    static void init();

private:
    NIC<NIC_Family> * _nic;

    static Observed _observed; // Channel protocols are singletons
};

template<>
inline void ELP::init_helper<ELP::UNITS>() {};

__END_SYS

#endif

#endif
