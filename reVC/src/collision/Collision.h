//+ rouz edit (ChatGPT)
#pragma once

#include "ColModel.h"
#include "Game.h"	// for eLevelName
#ifdef VU_COLLISION
#include "VuVector.h"
#endif

#include "StoredCollPoly.h"

// If you spawn many tanks at once, you will see that collisions of two entity exceeds 32.
#if defined(FIX_BUGS) && !defined(SQUEEZE_PERFORMANCE)
#define MAX_COLLISION_POINTS 64
#else
#define MAX_COLLISION_POINTS 32
#endif

//+ rouz edit (Claude)
// Formerly the static class CCollision, now C globals and functions shared with C
#ifdef __cplusplus
class CBox;
class CColBox;
class CColLine;
class CColSphere;
class CColTriangle;
class CColTrianglePlane;
class CSphere;
class CStoredCollPoly;
#else
typedef struct CBox CBox;
typedef struct CColBox CColBox;
typedef struct CColLine CColLine;
typedef struct CColSphere CColSphere;
typedef struct CColTriangle CColTriangle;
typedef struct CColTrianglePlane CColTrianglePlane;
typedef struct CSphere CSphere;
typedef struct CStoredCollPoly CStoredCollPoly;
#endif
#ifdef __cplusplus
extern "C" {
#endif
extern eLevelName Collision_ms_collisionInMemory;
void Collision_Init(void);
void Collision_Shutdown(void);
void Collision_Update(void);
void Collision_LoadCollisionWhenINeedIt(bool changeLevel);
void Collision_SortOutCollisionAfterLoad(void);
void Collision_LoadCollisionScreen(eLevelName level);
void Collision_CalculateTrianglePlanes(CColModel *model);
void Collision_RemoveTrianglePlanes(CColModel *model);
#ifdef __cplusplus
}
// Members that C cannot declare yet (references, default arguments, overloads, C++ class values), with C++ linkage
extern CLinkList Collision_ms_colModelCache;
void Collision_DrawColModel(const CMatrix &mat, const CColModel &colModel);
void Collision_DrawColModel_Coloured(const CMatrix &mat, const CColModel &colModel, int32 id);
bool Collision_TestSphereSphere(const CSphere &s1, const CSphere &s2);
bool Collision_TestSphereBox(const CSphere &sph, const CBox &box);
bool Collision_TestLineBox(const CColLine &line, const CBox &box);
bool Collision_TestVerticalLineBox(const CColLine &line, const CBox &box);
bool Collision_TestLineTriangle(const CColLine &line, const CompressedVector *verts, const CColTriangle &tri, const CColTrianglePlane &plane);
bool Collision_TestLineSphere(const CColLine &line, const CColSphere &sph);
bool Collision_TestSphereTriangle(const CColSphere &sphere, const CompressedVector *verts, const CColTriangle &tri, const CColTrianglePlane &plane);
bool Collision_TestLineOfSight(const CColLine &line, const CMatrix &matrix, CColModel &model, bool ignoreSeeThrough, bool ignoreShootThrough);
bool Collision_ProcessSphereSphere(const CColSphere &s1, const CColSphere &s2, CColPoint &point, float &mindistsq);
bool Collision_ProcessSphereBox(const CColSphere &sph, const CColBox &box, CColPoint &point, float &mindistsq);
bool Collision_ProcessLineBox(const CColLine &line, const CColBox &box, CColPoint &point, float &mindist);
bool Collision_ProcessVerticalLineTriangle(const CColLine &line, const CompressedVector *verts, const CColTriangle &tri, const CColTrianglePlane &plane, CColPoint &point, float &mindist, CStoredCollPoly *poly);
bool Collision_ProcessLineTriangle(const CColLine &line , const CompressedVector *verts, const CColTriangle &tri, const CColTrianglePlane &plane, CColPoint &point, float &mindist, CStoredCollPoly *poly = nil);
bool Collision_ProcessLineSphere(const CColLine &line, const CColSphere &sphere, CColPoint &point, float &mindist);
bool Collision_ProcessSphereTriangle(const CColSphere &sph, const CompressedVector *verts, const CColTriangle &tri, const CColTrianglePlane &plane, CColPoint &point, float &mindistsq);
bool Collision_ProcessLineOfSight(const CColLine &line, const CMatrix &matrix, CColModel &model, CColPoint &point, float &mindist, bool ignoreSeeThrough, bool ignoreShootThrough);
bool Collision_ProcessVerticalLine(const CColLine &line, const CMatrix &matrix, CColModel &model, CColPoint &point, float &mindist, bool ignoreSeeThrough, bool ignoreShootThrough, CStoredCollPoly *poly);
int32 Collision_ProcessColModels(const CMatrix &matrixA, CColModel &modelA, const CMatrix &matrixB, CColModel &modelB, CColPoint *spherepoints, CColPoint *linepoints, float *linedists);
float Collision_DistToLine(const CVector *l0, const CVector *l1, const CVector *point);
float Collision_DistToLine(const CVector *l0, const CVector *l1, const CVector *point, CVector &closest);
#endif
//- rouz edit (Claude)
//- rouz edit (ChatGPT)
