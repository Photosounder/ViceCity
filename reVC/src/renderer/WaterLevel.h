#pragma once

#define WATER_X_OFFSET (400.0f)

#define WATER_Z_OFFSET (0.5f)

#define NO_WATER -128

#define MAX_SMALL_SECTORS      128
#define MAX_LARGE_SECTORS      64
#define MAX_HUGE_SECTORS       32
#define MAX_EXTRAHUGE_SECTORS  16

#define SMALL_SECTOR_SIZE      32
#define LARGE_SECTOR_SIZE      64
#define HUGE_SECTOR_SIZE       128
#define EXTRAHUGE_SECTOR_SIZE  256

#define WATER_START_X -2048.0f
#define WATER_END_X    2048.0f

#define WATER_START_Y -2048.0f
#define WATER_END_Y    2048.0f

#define WATER_WIDTH		((WATER_END_X - WATER_START_X))
#define WATER_HEIGHT	((WATER_END_Y - WATER_START_Y))

#define WATER_UNSIGN_X(x)                   ( (x) + (WATER_WIDTH /2) )
#define WATER_UNSIGN_Y(y)                   ( (y) + (WATER_HEIGHT/2) )
#define WATER_SIGN_X(x)                     ( (x) - (WATER_WIDTH /2) )
#define WATER_SIGN_Y(y)                     ( (y) - (WATER_HEIGHT/2) )

// 64x64 Large blocks 64x64 each
#define WATER_TO_BLOCK_X(x)             ( WATER_UNSIGN_X(x) / WATER_BLOCK_SECTORS )
#define WATER_TO_BLOCK_Y(x)             ( WATER_UNSIGN_Y(x) / WATER_BLOCK_SECTORS ) 

// 128x128 Small blocks 32x32 each
#define WATER_TO_FINEBLOCK_X(x)         ( WATER_UNSIGN_X(x) / WATER_FINEBLOCK_SECTORS )
#define WATER_TO_FINEBLOCK_Y(x)         ( WATER_UNSIGN_Y(x) / WATER_FINEBLOCK_SECTORS ) 

// 32
#define WATER_SMALL_X(x)                    ( WATER_UNSIGN_X(x)					/ MAX_SMALL_SECTORS     )
#define WATER_SMALL_Y(y)                    ( WATER_UNSIGN_Y(y)					/ MAX_SMALL_SECTORS     )
#define WATER_FROM_SMALL_SECTOR_X(x)        ( ((x) - (MAX_SMALL_SECTORS/2)    ) * SMALL_SECTOR_SIZE     )
#define WATER_FROM_SMALL_SECTOR_Y(y)        ( ((y) - (MAX_SMALL_SECTORS/2)    ) * SMALL_SECTOR_SIZE     )
#define WATER_TO_SMALL_SECTOR_X(x)          ( WATER_UNSIGN_X(x)					/ SMALL_SECTOR_SIZE     )
#define WATER_TO_SMALL_SECTOR_Y(y)          ( WATER_UNSIGN_Y(y)					/ SMALL_SECTOR_SIZE     )
			
// 64			
#define WATER_LARGE_X(x)                    ( WATER_UNSIGN_X(x)					/ MAX_LARGE_SECTORS     )
#define WATER_LARGE_Y(y)                    ( WATER_UNSIGN_Y(y)					/ MAX_LARGE_SECTORS     )
#define WATER_FROM_LARGE_SECTOR_X(x)        ( ((x) - (MAX_LARGE_SECTORS/2)    ) * LARGE_SECTOR_SIZE     )
#define WATER_FROM_LARGE_SECTOR_Y(y)        ( ((y) - (MAX_LARGE_SECTORS/2)    ) * LARGE_SECTOR_SIZE     )
#define WATER_TO_LARGE_SECTOR_X(x)          ( WATER_UNSIGN_X(x)					/ LARGE_SECTOR_SIZE     )
#define WATER_TO_LARGE_SECTOR_Y(y)          ( WATER_UNSIGN_Y(y)					/ LARGE_SECTOR_SIZE     )
				
// 128				
#define WATER_HUGE_X(x)                     ( WATER_UNSIGN_X(x)					/ MAX_HUGE_SECTORS      )
#define WATER_HUGE_Y(y)                     ( WATER_UNSIGN_Y(y)					/ MAX_HUGE_SECTORS      )
#define WATER_FROM_HUGE_SECTOR_X(x)         ( ((x) - (MAX_HUGE_SECTORS/2)     ) * HUGE_SECTOR_SIZE      )
#define WATER_FROM_HUGE_SECTOR_Y(y)         ( ((y) - (MAX_HUGE_SECTORS/2)     ) * HUGE_SECTOR_SIZE      )
#define WATER_TO_HUGE_SECTOR_X(x)           ( WATER_UNSIGN_X(x)					/ HUGE_SECTOR_SIZE      )
#define WATER_TO_HUGE_SECTOR_Y(y)           ( WATER_UNSIGN_Y(y)					/ HUGE_SECTOR_SIZE      )

// 256	
#define WATER_EXTRAHUGE_X(x)                ( WATER_UNSIGN_X(x)					/ MAX_EXTRAHUGE_SECTORS )
#define WATER_EXTRAHUGE_Y(y)                ( WATER_UNSIGN_Y(y)					/ MAX_EXTRAHUGE_SECTORS )
#define WATER_FROM_EXTRAHUGE_SECTOR_X(x)    ( ((x) - (MAX_EXTRAHUGE_SECTORS/2)) * EXTRAHUGE_SECTOR_SIZE )
#define WATER_FROM_EXTRAHUGE_SECTOR_Y(y)    ( ((y) - (MAX_EXTRAHUGE_SECTORS/2)) * EXTRAHUGE_SECTOR_SIZE )
#define WATER_TO_EXTRAHUGE_SECTOR_X(x)      ( WATER_UNSIGN_X(x)					/ EXTRAHUGE_SECTOR_SIZE )
#define WATER_TO_EXTRAHUGE_SECTOR_Y(y)      ( WATER_UNSIGN_Y(y)					/ EXTRAHUGE_SECTOR_SIZE )

struct ColData
{
	uint8 SurfaceType;
	uint8 PieceType;
};

enum eBeachToy
{
	BEACHTOY_0 = 0,
	BEACHTOY_BALL,
	BEACHTOY_LOUNGE_WOOD_UP,
	BEACHTOY_LOUNGE_TOWEL_UP,
	BEACHTOY_LOUNGE_WOOD_ON,
	BEACHTOY_ANY_LOUNGE,
	BEACHTOY_LOTION,
	BEACHTOY_TOWEL1,
	BEACHTOY_TOWEL2,
	BEACHTOY_TOWEL3,
	BEACHTOY_TOWEL4,
	BEACHTOY_ANY_TOWEL,
};

extern RwRaster* gpWaterRaster;
extern bool gbDontRenderWater;

class CEntity;

//+ rouz edit (Claude)
// Formerly the static class CWaterLevel, now C globals and functions shared with C
#ifdef __cplusplus
extern "C" {
#endif
extern int32 WaterLevel_ms_nNoOfWaterLevels;
extern float WaterLevel_ms_aWaterZs[48];
extern CRect WaterLevel_ms_aWaterRects[48];
extern int8 WaterLevel_aWaterBlockList[MAX_LARGE_SECTORS][MAX_LARGE_SECTORS];
extern int8 WaterLevel_aWaterFineBlockList[MAX_SMALL_SECTORS][MAX_SMALL_SECTORS];
extern bool WaterLevel_WavesCalculatedThisFrame;
extern bool WaterLevel_RequireWavySector;
extern bool WaterLevel_MaskCalculatedThisFrame;
extern CVector WaterLevel_PreCalculatedMaskPosn;
extern bool WaterLevel_m_bRenderSeaBed;
extern int32 WaterLevel_m_nRenderWaterLayers;
extern RpAtomic    * WaterLevel_ms_pWavyAtomic;
extern RpAtomic    * WaterLevel_ms_pMaskAtomic;
void    WaterLevel_Initialise(Const char *pWaterDat); // out of class in III PC and later because of SecuROM
void    WaterLevel_Shutdown();
void    WaterLevel_CreateWavyAtomic();
void    WaterLevel_DestroyWavyAtomic();
void    WaterLevel_AddWaterLevel(float fXLeft, float fYBottom, float fXRight, float fYTop, float fLevel);
void    WaterLevel_RemoveIsolatedWater();
bool    WaterLevel_GetWaterLevelNoWaves(float fX, float fY, float fZ, float *pfOutLevel);
float   WaterLevel_GetWaterWavesOnly(short x, short y);	// unused
CVector WaterLevel_GetWaterNormal(float fX, float fY);
void    WaterLevel_RenderWater();
void    WaterLevel_RenderTransparentWater(void);
#ifdef PC_WATER
#else
#endif
#ifdef PC_WATER
void WaterLevel_PreCalcWaterGeometry(void);
#endif
void WaterLevel_RenderBoatWakes(void);
float WaterLevel_CalcDistanceToWater(float fX, float fY);
void WaterLevel_RenderAndEmptyRenderBuffer();
void WaterLevel_RenderSeaBirds();
void WaterLevel_RenderShipsOnHorizon();
void WaterLevel_HandleSeaLifeForms();
void WaterLevel_HandleBeachToysStuff(void);
#ifdef __cplusplus
}
// Members that C cannot declare yet (references, default arguments, overloads, C++ class values), with C++ linkage
bool    WaterLevel_WaterLevelAccordingToRectangles(float fX, float fY, float *pfOutLevel = nil);
bool    WaterLevel_TestVisibilityForFineWaterBlocks(const CVector &worldPos);
bool    WaterLevel_GetWaterLevel(float fX, float fY, float fZ, float *pfOutLevel, bool bDontCheckZ);
static inline bool    WaterLevel_GetWaterLevel(CVector coors, float *pfOutLevel, bool bDontCheckZ);
void WaterLevel_RenderOneFlatSmallWaterPoly(float fX, float fY, float fZ, RwRGBA const &color);
void WaterLevel_RenderOneFlatLargeWaterPoly(float fX, float fY, float fZ, RwRGBA const &color);
void WaterLevel_RenderOneFlatHugeWaterPoly(float fX, float fY, float fZ, RwRGBA const &color);
void WaterLevel_RenderOneFlatExtraHugeWaterPoly(float fX, float fY, float fZ, RwRGBA const &color);
void WaterLevel_RenderOneWavySector(float fX, float fY, float fZ, RwRGBA const &color, bool bDontRender = false);
#ifdef PC_WATER
void WaterLevel_RenderWavyMask(float fX, float fY, float fZ, float fSectorX, float fSectorY, float fCamPosX, float fCamPosY, float fCamDirX, float fCamDirY, RwRGBA const&color);
#else
void WaterLevel_RenderWavyMask(float fX, float fY, float fZ, float fSectorX, float fSectorY, int32 nCamDirX, int32 nCamDirY, RwRGBA const&color);
#endif
#ifdef PC_WATER
bool WaterLevel_PreCalcWavySector(RwRGBA const &color);	//fucked up
bool WaterLevel_PreCalcWavyMask(float fX, float fY, float fZ, float fSectorX, float fSectorY, float fCamPosX, float fCamPosY, float fCamDirX, float fCamDirY, RwRGBA const&color);
#endif
void WaterLevel_RenderWakeSegment(CVector2D &vecA, CVector2D &vecB, CVector2D &vecC, CVector2D &vecD, float &fSizeA, float &fSizeB, float &fAlphaA, float &fAlphaB, float &fWakeZ);
void WaterLevel_RenderOneSlopedUnderWaterPoly(float fX, float fY, float fZ, RwRGBA const&color); // UNUSED
void WaterLevel_RenderOneFlatSmallWaterPolyBlended(float fX, float fY, float fZ, float fCamX, float fCamY, RwRGBA const &color, RwRGBA const &colorTrans, float fDrawDist);
bool WaterLevel_GetGroundLevel(CVector const &vecPosn, float *pfOutLevel, ColData *pData, float fDistance);
bool WaterLevel_IsLocationOutOfWorldBounds_WS(CVector const &vecPosn, int nOffset);
bool WaterLevel_GetGroundLevel_WS(CVector const & vecPosn, float *pfOutLevel, ColData *pData, float fDistance);
bool WaterLevel_GetWaterDepth(CVector const &vecPosn, float *pfDepth, float *pfLevelNoWaves, float *pfGroundLevel);
CEntity *WaterLevel_CreateBeachToy(CVector const &vec, eBeachToy beachtoy);
static inline bool    WaterLevel_GetWaterLevel(CVector coors, float *pfOutLevel, bool bDontCheckZ) { return WaterLevel_GetWaterLevel(coors.x, coors.y, coors.z, pfOutLevel, bDontCheckZ); }
#endif
//- rouz edit (Claude)
