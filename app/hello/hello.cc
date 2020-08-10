#include <utility/ostream.h>
#include <time.h>

using namespace EPOS;

OStream cout;

int main()
{
    cout << "Hello world!" << endl;
    
    cout << "simulation ended" << endl;
    while (1)
    {
        Alarm::delay(1000000);
        cout << "simulation ended" << endl;
    }
    return 0;
}
