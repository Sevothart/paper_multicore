// EPOS NIC Test Programs

#include <machine/nic.h>
#include <network/elp.h>
#include <time.h>
#include <communicator.h>

using namespace EPOS;

OStream cout;

static const unsigned int ITERATIONS = 10;

int elp_link_test()
{
    cout << "Link<ELP> Test" << endl;

    NIC<Ethernet> * nic = Traits<Ethernet>::DEVICES::Get<0>::Result::get(0);
    NIC<Ethernet>::Address mac = nic->address();
    cout << "  MAC address: " << mac << endl;

    if(mac[5] % 2) { // sender
        cout << "Sender:" << endl;

        NIC<Ethernet>::Address peer_mac = mac;
        peer_mac[5]--;
        ELP::Address peer(peer_mac, 0);

        Link<ELP> com(ELP::Address(mac, 0), peer);

        NIC<Ethernet>::Address self = nic->address();
        cout << "  MAC: " << self << endl;

        char data[ELP::MTU];

        for(int i = 0; i < ITERATIONS; i++) {
            memset(data, '0' + i, ELP::MTU - 1);
            data[ELP::MTU - 1] = '\0';
            int sent = com.send(data, sizeof(data));
            if(sent == sizeof(data))
                cout << "  Data to " << peer << ": " << data << endl;
            else
                cout << "  Data was not correctly sent. It was " << sizeof(data) << " bytes long, but only " << sent << " bytes were sent!"<< endl;
        }
    } else { // receiver
        cout << "Receiver:" << endl;

        NIC<Ethernet>::Address peer_mac = mac;
        peer_mac[5]++;
        ELP::Address peer(peer_mac, 0);

        Link<ELP> com(ELP::Address(mac, 0), peer);

        char data[ELP::MTU];

        for(int i = 0; i < ITERATIONS; i++) {
            ELP::Address peer;
            int received = com.receive(data, sizeof(data));
            if(received == sizeof(data))
                cout << "  Data from " << peer << ": " << data << endl;
            else
                cout << "  Data was not correctly received. It was " << sizeof(data) << " bytes long, but " << received << " bytes were received!"<< endl;
        }
    }

    Ethernet::Statistics stat = nic->statistics();
    cout << "Statistics\n"
         << "Tx Packets: " << stat.tx_packets << "\n"
         << "Tx Bytes:   " << stat.tx_bytes << "\n"
         << "Rx Packets: " << stat.rx_packets << "\n"
         << "Rx Bytes:   " << stat.rx_bytes << endl;

    return stat.tx_bytes + stat.rx_bytes;
}

int elp_port_test()
{
    cout << "Port<ELP> Test" << endl;

    NIC<Ethernet> * nic = Traits<Ethernet>::DEVICES::Get<0>::Result::get(0);
    NIC<Ethernet>::Address mac = nic->address();
    cout << "  MAC address: " << mac << endl;

    if(mac[5] % 2) { // sender
        Delay(Second(1)); // Receuver takes more time to reach receiving state in the second execution.
        cout << "Sender:" << endl;

        NIC<Ethernet>::Address peer_mac = mac;
        peer_mac[5]--;
        ELP::Address peer(peer_mac, 0);

        Port<ELP> com(ELP::Address(mac, 0));

        NIC<Ethernet>::Address self = nic->address();
        cout << "  MAC: " << self << endl;

        char data[ELP::MTU];

        for(int i = 0; i < ITERATIONS; i++) {
            memset(data, '0' + i, ELP::MTU - 1);
            data[ELP::MTU - 1] = '\0';
            int sent = com.send(peer, data, sizeof(data));
            if(sent == sizeof(data))
                cout << "  Data to " << peer << ": " << data << endl;
            else
                cout << "  Data was not correctly sent. It was " << sizeof(data) << " bytes long, but only " << sent << " bytes were sent!"<< endl;
        }
    } else { // receiver
        cout << "Receiver:" << endl;

        Port<ELP> com(ELP::Address(mac, 0));

        char data[ELP::MTU];

        for(int i = 0; i < ITERATIONS; i++) {
            ELP::Address peer;
            int received = com.receive(&peer, data, sizeof(data));
            if(received == sizeof(data))
                cout << "  Data from " << peer << ": " << data << endl;
            else
                cout << "  Data was not correctly received. It was " << sizeof(data) << " bytes long, but " << received << " bytes were received!"<< endl;
        }
    }

    Ethernet::Statistics stat = nic->statistics();
    cout << "Statistics\n"
         << "Tx Packets: " << stat.tx_packets << "\n"
         << "Tx Bytes:   " << stat.tx_bytes << "\n"
         << "Rx Packets: " << stat.rx_packets << "\n"
         << "Rx Bytes:   " << stat.rx_bytes << endl;

    return stat.tx_bytes + stat.rx_bytes;
}

int main()
{
    cout << "ELP Test\n" << endl;

    elp_link_test();
    Delay(Second(5));
    elp_port_test();
}
