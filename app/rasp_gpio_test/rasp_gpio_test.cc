#include <utility/ostream.h>
#include <machine/cortex/raspberry_pi3/raspberry_pi3_gpio.h>

using namespace EPOS;
OStream cout;

int main() {
    cout << "Starting GPIO_Engine test..." << endl;
    /* The pin level was also confirmed measuring pin 17 from raspberry with a multimeter */
    GPIO_Engine * pin = new GPIO_Engine(GPIO_Common::B, 7, GPIO_Common::Direction::INOUT, GPIO_Common::Pull::DOWN, GPIO_Common::Edge::NONE);

    cout << "Pin state before set: " << pin->get() << endl;
    cout << "Setting..." << endl; pin->set();
    cout << "Pin state after set: " << pin->get() << endl;
    cout << "Clearing..." << endl; pin->set(); pin->clear();
    cout << "\t" << pin->get() << endl;
    cout << "Pin state after clearing: " << pin->get() << endl;

    cout << "Ending GPIO_Engine test..." << endl;
    return 0;
}