#include <stdint.h>
#include <string.h>
uint64_t TestAircraftHash(uint64_t hash,const void *data,uint32_t size)
{
    // Observe every initialized word with the fixture's original ordered FNV arithmetic
    const unsigned char *bytes=(const unsigned char *)data;
    for(uint32_t i=0;i<size;i+=4){uint32_t word;memcpy(&word,bytes+i,4);hash=(hash^word)*UINT64_C(1099511628211);}
    return hash;
}
