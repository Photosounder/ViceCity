//+ rouz edit (ChatGPT)
#include "common.h"
#include "AutoPilot_backend.h"
#include "PathFind.h"
AutoPilotVector AutoPilot_FromVector(CVector value)
{
    // Copy the original native destination components into the plain C record
    AutoPilotVector result={value.x,value.y,value.z};return result;
}
CVector AutoPilot_ToVector(AutoPilotVector value)
{
    // Copy the plain destination components at the remaining native argument boundary
    return CVector(value.x,value.y,value.z);
}
CCarPathLink * AutoPilot_BackendLink(uint32_t index)
{
    // Read the original typed path-link primitive at its original C policy access point
    return &ThePaths.m_carPathLinks[index];
}

float AutoPilot_BackendX(CCarPathLink *link)
{
    // Read the original typed path-link primitive at its original C policy access point
    return link->GetX();
}

float AutoPilot_BackendY(CCarPathLink *link)
{
    // Read the original typed path-link primitive at its original C policy access point
    return link->GetY();
}

float AutoPilot_BackendDirX(CCarPathLink *link)
{
    // Read the original typed path-link primitive at its original C policy access point
    return link->GetDirX();
}

float AutoPilot_BackendDirY(CCarPathLink *link)
{
    // Read the original typed path-link primitive at its original C policy access point
    return link->GetDirY();
}
//- rouz edit (ChatGPT)
