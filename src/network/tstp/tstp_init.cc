// EPOS Trustful SpaceTime Protocol Initialization

#include <system/config.h>

#ifdef __tstp__

#include <machine/nic.h>
#include <network/tstp/tstp.h>
#include <system.h>
#include <time.h>

__BEGIN_SYS

TSTP::TSTP(NIC<NIC_Family> * nic)
{
    db<Init, TSTP>(TRC) << "TSTP::TSTP(nic=" << nic << ")" << endl;

    _nic = nic;
    _nic->attach(this, PROTO_TSTP);

    // The order parts are created defines the order they get notified when packets arrive!
    _security = new (SYSTEM) Security;
    _timekeeper = new (SYSTEM) Timekeeper;
    _locator = new (SYSTEM) Locator;
    _router = new (SYSTEM) Router;
    _manager = new (SYSTEM) Manager;
}

TSTP::Security::Security()
{
    db<TSTP>(TRC) << "TSTP::Security()" << endl;

    new (&_id) Node_Id(Machine::uuid(), sizeof(UUID));

    db<TSTP>(INF) << "Node ID: " << _id << endl;

    assert(_AES::KEY_SIZE == sizeof(Node_Id));
    _aes.encrypt(_id, _id, _auth);

    attach(this);

    /*
    // TODO: what is this?
    if((TSTP::here() != TSTP::sink()) && (!Traits<Radio>::promiscuous)) {
        Peer * peer = new (SYSTEM) Peer(_id, Region(TSTP::sink(), 0, 0, -1));
        _pending_peers.insert(peer->link());

        // Wait for key establishment
        while(_trusted_peers.size() == 0)
            Thread::self()->yield();
    }
    */
}

TSTP::Timekeeper::Timekeeper()
{
    db<TSTP>(TRC) << "TSTP::Timekeeper()" << endl;
    db<TSTP>(INF) << "TSTP::Timekeeper: timer frequency = " << NIC<NIC_Family>::Timer::frequency() << " Hz" << endl;
    db<TSTP>(INF) << "TSTP::Timekeeper: timer accuracy = " << NIC<NIC_Family>::Timer::accuracy() << " ppb" << endl;
    db<TSTP>(INF) << "TSTP::Timekeeper: maximum drift = " << MAX_DRIFT << " us" << endl;
    db<TSTP>(INF) << "TSTP::Timekeeper: sync period = " << sync_period() << " us" << endl;

    if(here() == sink())
        _next_sync = -1ull; // Just so that the sink will always have synchronized() return true

    attach(this);

    if(here() != sink()) {
        keep_alive();
        _life_keeper = new (SYSTEM) Alarm(sync_period(), &_life_keeper_handler, INFINITE);

        // Wait for time synchronization
        while(sync_required())
            Thread::self()->yield();
    }
}

TSTP::Locator::Locator()
{
    db<TSTP>(TRC) << "TSTP::Locator()" << endl;

    System_Info::Boot_Map * bm = &System::info()->bm;
    if(bm->space_x != Space::UNKNOWN) {
        _engine.here(Space(bm->space_x, bm->space_y, bm->space_z));
        _engine.confidence(100);
    } else {
        _engine.here(Space(Space::UNKNOWN, Space::UNKNOWN, Space::UNKNOWN));
        _engine.confidence(0);
    }
    attach(this);

    db<TSTP>(INF) << "TSTP::Locator::here=" << here() << endl;

    // Wait for spatial localization
    while(confidence() < 80)
        Thread::self()->yield();

    // _absolute_location is initialized later through an Epoch message
}

TSTP::Router::Router()
{
    db<TSTP>(TRC) << "TSTP::Router()" << endl;

    attach(this);
}

TSTP::Manager::Manager()
{
    db<TSTP>(TRC) << "TSTP::Manager()" << endl;

    attach(this);
}

void TSTP::init()
{
    db<Init, TSTP>(TRC) << "TSTP::init()" << endl;

    NIC<NIC_Family> * nic = Traits<NIC_Family>::DEVICES::Get<Traits<TSTP>::NICS[0]>::Result::get(Traits<TSTP>::NICS[0]);

    new (SYSTEM) TSTP(nic);
}

__END_SYS

#endif
