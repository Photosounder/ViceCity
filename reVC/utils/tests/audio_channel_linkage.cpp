#include "channel.h"
#include <cassert>
extern "C" int AudioChannel_CTest(void);
int main(void)
{
    // Exercise independently compiled C channel state through its C++ API declarations
    CChannel channel;
    CChannel_InitializeState(&channel);
    assert(channel.Data == nullptr && channel.DataSize == 0 && !channel.bIs2D);
    assert(channel.Pitch == 1.0f && channel.Gain == 1.0f && channel.LoopCount == 1);
    return AudioChannel_CTest();
}
