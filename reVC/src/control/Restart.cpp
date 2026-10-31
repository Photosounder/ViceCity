//+ rouz edit (ChatGPT)
#include "common.h"
#include "Restart_backend.h"
#include "PathFind.h"
RestartPoint Restart_BackendPosition(const RestartPosition *position)
{
    // Read the original still-live native position without assuming its object layout
    RestartPoint result={position->x,position->y,position->z};return result;
}
void Restart_BackendSetPosition(RestartPosition *position,RestartPoint value)
{
    // Assign the original native output object with explicit coordinate transfers
    *position=CVector(value.x,value.y,value.z);
}
RestartPoint Restart_BackendClosestNode(const RestartPosition *position)
{
    // Preserve the actual native path search and selected node's original fixed-point conversion
    CVector result=ThePaths.m_pathNodes[ThePaths.FindNodeClosestToCoors(*position,PATH_PED,999999.9f)].GetPosition();
    RestartPoint point={result.x,result.y,result.z};return point;
}
//- rouz edit (ChatGPT)
