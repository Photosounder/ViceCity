#pragma once

struct ActiveOccluderLine {
	CVector2D origin;
	CVector2D direction;
	float length; 
};

class CActiveOccluder {

public:
	ActiveOccluderLine lines[6];
	int32 linesCount;
	float radius; 

	bool IsPointWithinOcclusionArea(float x, float y, float area);
};

class COccluder
{
public:
	int16 length, width, height;
	int16 x, y, z;
	uint16 angle;
	int16 listIndex;

	bool NearCamera();
	bool ProcessOneOccluder(CActiveOccluder *occl);
	bool ProcessLineSegment(int corner1, int corner2, CActiveOccluder* occl);
	float GetAngle(void) { return angle*TWOPI/UINT16_MAX; }
};

//+ rouz edit (Claude)
// Formerly the static class COcclusion, now C globals and functions shared with C
#ifdef __cplusplus
extern "C" {
#endif
extern int32 Occlusion_NumOccludersOnMap;
extern int16 Occlusion_FarAwayList;
extern int16 Occlusion_NearbyList;
extern int16 Occlusion_ListWalkThroughFA;
extern int16 Occlusion_PreviousListWalkThroughFA;
extern int16 Occlusion_NumActiveOccluders;
void Occlusion_Init(void);
void Occlusion_AddOne(float x, float y, float z, float width, float length, float height, float angle);
void Occlusion_ProcessBeforeRendering(void);
bool Occlusion_OccluderHidesBehind(CActiveOccluder *occl1, CActiveOccluder *occl2);
bool Occlusion_IsAABoxOccluded(CVector pos, float width, float length, float height);
bool Occlusion_IsPositionOccluded(CVector pos, float side);
#ifndef MASTER
void Occlusion_Render();
#endif
#ifdef __cplusplus
}
// Members that C cannot declare yet (references, default arguments, overloads, C++ class values), with C++ linkage
extern COccluder Occlusion_aOccluders[NUMOCCLUSIONVOLUMES];
extern CActiveOccluder Occlusion_aActiveOccluders[NUMACTIVEOCCLUDERS];
#ifndef MASTER
#endif
#endif
//- rouz edit (Claude)

bool CalcScreenCoors(CVector const &in, CVector *out, float *outw, float *outh);
bool CalcScreenCoors(CVector const &in, CVector *out);

#ifndef MASTER
extern bool bDispayOccDebugStuff;
#endif