#include "aldlist.h"
#include <cassert>
extern "C" int AudioDevice_CTest(void);
int main(void)
{
    // Invoke the independently compiled C list through its public C++ declarations
    ALDeviceList list;
    ALDeviceList_Init(&list);
    assert(ALDeviceList_GetNumDevices(&list) == 0);
    ALDeviceList_Destroy(&list);
    return AudioDevice_CTest();
}
