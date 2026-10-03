#include "config.h"
#include "stream.h"
extern "C" int AudioStream_CTest(void);
int main() {
    // Exercise the actual C stream API through its C++ linkage declarations
    ALuint sources[2] = {1, 2};
    ALuint buffers[NUM_STREAMBUFFERS] = {};
    CStream stream;
    CStream_Init(&stream, sources, buffers);
    CStream_Destroy(&stream);
    return AudioStream_CTest();
}
