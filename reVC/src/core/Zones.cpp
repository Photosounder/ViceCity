//+ rouz edit (ChatGPT)
#include "common.h"
#include "Zones.h"
#include "Text.h"
#include "World.h"

extern "C" ZonesPosition
ZonesHost_Player(void)
{
    // Copy the original player coordinates at the zone update's query point
    return ZonesPosition_FromVector(FindPlayerCoors());
}

extern "C" uint16_t *
ZonesHost_Text(const char *name)
{
    // Look up the original eight-byte zone label through the existing text owner
    return Text_Get(&TheText,name);
}
//- rouz edit (ChatGPT)
