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

//- rouz edit (ChatGPT)
