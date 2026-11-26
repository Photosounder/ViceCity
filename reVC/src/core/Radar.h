//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
#pragma once
#include "Sprite2d.h"
#include "Draw.h"

#define CARBLIP_MARKER_COLOR_R 252
#define CARBLIP_MARKER_COLOR_G 138
#define CARBLIP_MARKER_COLOR_B 242
#define CARBLIP_MARKER_COLOR_A 255

#define CHARBLIP_MARKER_COLOR_R 252
#define CHARBLIP_MARKER_COLOR_G 138
#define CHARBLIP_MARKER_COLOR_B 242
#define CHARBLIP_MARKER_COLOR_A 255

#define OBJECTBLIP_MARKER_COLOR_R 252
#define OBJECTBLIP_MARKER_COLOR_G 138
#define OBJECTBLIP_MARKER_COLOR_B 242
#define OBJECTBLIP_MARKER_COLOR_A 255

#define COORDBLIP_MARKER_COLOR_R 252
#define COORDBLIP_MARKER_COLOR_G 138
#define COORDBLIP_MARKER_COLOR_B 242
#define COORDBLIP_MARKER_COLOR_A 228

#define NUM_MAP_LEGENDS 75

#define MENU_MAP_LENGTH_UNIT 1190.0f // in game unit
#define MENU_MAP_WIDTH_SCALE 1.112f // in game unit (originally 1.112494151260504f)
#define MENU_MAP_HEIGHT_SCALE 1.119f // in game unit (originally 1.118714268907563f)
#define MENU_MAP_TOP_OFFSET 0.28f // in length unit defined above - ~333 game unit
#define MENU_MAP_LEFT_OFFSET 0.185f // in length unit defined above - ~220 game unit
#define MENU_MAP_LENGTH (4000.f / MENU_MAP_LENGTH_UNIT)

enum eBlipType
{
	BLIP_NONE,
	BLIP_CAR,
	BLIP_CHAR,
	BLIP_OBJECT,
	BLIP_COORD,
	BLIP_CONTACT_POINT
};

enum eBlipDisplay
{
	BLIP_DISPLAY_NEITHER = 0,
	BLIP_DISPLAY_MARKER_ONLY = 1,
	BLIP_DISPLAY_BLIP_ONLY = 2,
	BLIP_DISPLAY_BOTH = 3,
};

enum eRadarSprite
{
	RADAR_SPRITE_ENTITY_BLIP = -2,
	RADAR_SPRITE_COORD_BLIP = -1,
	RADAR_SPRITE_NONE = 0,
	RADAR_SPRITE_CENTRE,
	RADAR_SPRITE_MAP_HERE,
	RADAR_SPRITE_NORTH,
	RADAR_SPRITE_AVERY,
	RADAR_SPRITE_BIKER,
	RADAR_SPRITE_CORTEZ,
	RADAR_SPRITE_DIAZ,
	RADAR_SPRITE_KENT,
	RADAR_SPRITE_LAWYER,
	RADAR_SPRITE_PHIL,
	RADAR_SPRITE_BIKERS,
	RADAR_SPRITE_BOATYARD,
	RADAR_SPRITE_MALIBU_CLUB,
	RADAR_SPRITE_CUBANS,
	RADAR_SPRITE_FILM,
	RADAR_SPRITE_GUN,
	RADAR_SPRITE_HAITIANS,
	RADAR_SPRITE_HARDWARE,
	RADAR_SPRITE_SAVE,
	RADAR_SPRITE_STRIP,
	RADAR_SPRITE_ICE,
	RADAR_SPRITE_KCABS,
	RADAR_SPRITE_LOVEFIST,
	RADAR_SPRITE_PRINTWORKS,
	RADAR_SPRITE_PROPERTY,
	RADAR_SPRITE_SUNYARD,
	RADAR_SPRITE_SPRAY,
	RADAR_SPRITE_TSHIRT,
	RADAR_SPRITE_TOMMY,
	RADAR_SPRITE_PHONE,
	RADAR_SPRITE_RADIO_WILDSTYLE,
	RADAR_SPRITE_RADIO_FLASH,
	RADAR_SPRITE_RADIO_KCHAT,
	RADAR_SPRITE_RADIO_FEVER,
	RADAR_SPRITE_RADIO_VROCK,
	RADAR_SPRITE_RADIO_VCPR,
	RADAR_SPRITE_RADIO_ESPANTOSO,
	RADAR_SPRITE_RADIO_EMOTION,
	RADAR_SPRITE_RADIO_WAVE,
#ifdef MAP_ENHANCEMENTS
	RADAR_SPRITE_WAYPOINT,
#endif

	RADAR_SPRITE_COUNT
};

#include "RadarTrace.h"
#include "RadarTraceData.h"

enum
{
	BLIP_MODE_TRIANGULAR_UP = 0,
	BLIP_MODE_TRIANGULAR_DOWN,
	BLIP_MODE_SQUARE,
};

struct sRadarTrace : public sRadarTraceData
{
};

// Either that was a thing while saving/loading blips, or they added sizes of each field one by one. I want to do the former.
#pragma pack(push,1)
struct sRadarTraceSave
{
	uint32 m_nColor;
	float m_Radius;
	uint32 m_eBlipType; // eBlipType
	int32 m_nEntityHandle;
	CVector2D m_vec2DPos;
	CVector m_vecPos;
	uint16 m_BlipIndex;
	bool m_bDim;
	bool m_bInUse;
	bool m_bShortRange;
	bool m_unused;
	int16 m_wScale;
	uint16 m_eBlipDisplay; // eBlipDisplay
	uint16 m_eRadarSprite; // eRadarSprite
};
#pragma pack(pop)

// Values for screen space
#define RADAR_LEFT (40.0f)
#define RADAR_BOTTOM (40.0f)

#ifdef FIX_RADAR
/*
	The values are from an early screenshot taken before R* broke radar
		#define RADAR_WIDTH (82.0f)
		#define RADAR_HEIGHT (82.0f)
*/
#define RADAR_WIDTH ((Draw_ms_bFixRadar)  ? (82.0f) : (94.0f))	
#define RADAR_HEIGHT ((Draw_ms_bFixRadar) ? (82.0f) : (76.0f))
#else
/*
	broken since forever, someone tried to fix size for 640x512(PAL)
		http://aap.rockstarvision.com/pics/gta3/ps2screens/gta3_interface.jpg
	but failed:
		http://aap.rockstarvision.com/pics/gta3/artwork/gta3_artwork_16.jpg
	most likely the guy used something like this:
		int y = 82 * (640.0/512.0)/(640.0/480.0);
		int x = y * (640.0/512.0);
*/
#define RADAR_WIDTH (94.0f)
#define RADAR_HEIGHT (76.0f)
#endif

//+ rouz edit (Claude)
// Formerly the static class CRadar, now C globals and functions shared with C
#ifdef __cplusplus
extern "C" {
#endif
extern float Radar_m_radarRange;
extern sRadarTrace Radar_ms_RadarTrace[NUMRADARBLIPS];
extern Sprite2d Radar_CentreSprite;
extern Sprite2d Radar_MapHereSprite;
extern Sprite2d Radar_NorthSprite;
extern Sprite2d Radar_AverySprite;
extern Sprite2d Radar_BikerSprite;
extern Sprite2d Radar_CortezSprite;
extern Sprite2d Radar_DiazSprite;
extern Sprite2d Radar_KentSprite;
extern Sprite2d Radar_LawyerSprite;
extern Sprite2d Radar_PhilSprite;
extern Sprite2d Radar_BikersSprite;
extern Sprite2d Radar_BoatyardSprite;
extern Sprite2d Radar_MalibuClubSprite;
extern Sprite2d Radar_CubansSprite;
extern Sprite2d Radar_FilmSprite;
extern Sprite2d Radar_GunSprite;
extern Sprite2d Radar_HaitiansSprite;
extern Sprite2d Radar_HardwareSprite;
extern Sprite2d Radar_SaveHouseSprite;
extern Sprite2d Radar_StripSprite;
extern Sprite2d Radar_IceSprite;
extern Sprite2d Radar_KCabsSprite;
extern Sprite2d Radar_LovefistSprite;
extern Sprite2d Radar_PrintworksSprite;
extern Sprite2d Radar_PropertySprite;
extern Sprite2d Radar_SunYardSprite;
extern Sprite2d Radar_SpraySprite;
extern Sprite2d Radar_TShirtSprite;
extern Sprite2d Radar_TommySprite;
extern Sprite2d Radar_PhoneSprite;
extern Sprite2d Radar_RadioWildstyleSprite;
extern Sprite2d Radar_RadioFlashSprite;
extern Sprite2d Radar_RadioKChatSprite;
extern Sprite2d Radar_RadioFeverSprite;
extern Sprite2d Radar_RadioVRockSprite;
extern Sprite2d Radar_RadioVCPRSprite;
extern Sprite2d Radar_RadioEspantosoSprite;
extern Sprite2d Radar_RadioEmotionSprite;
extern Sprite2d Radar_RadioWaveSprite;
extern Sprite2d * Radar_RadarSprites[RADAR_SPRITE_COUNT];
extern float Radar_cachedCos;
extern float Radar_cachedSin;
extern CRGBA Radar_ArrowBlipColour1;
extern CRGBA Radar_ArrowBlipColour2;
extern int16 Radar_MapLegendList[NUM_MAP_LEGENDS];
extern int16 Radar_MapLegendCounter;
#ifdef MAP_ENHANCEMENTS
extern Sprite2d Radar_WaypointSprite;
extern int Radar_TargetMarkerId;
extern CVector Radar_TargetMarkerPos;
#endif
void Radar_InitFrontEndMap();
void Radar_DrawYouAreHereSprite(float, float);
#ifdef MAP_ENHANCEMENTS
void Radar_ToggleTargetMarker(float, float);
#endif
uint8 Radar_CalculateBlipAlpha(float dist);
void Radar_ChangeBlipBrightness(int32 i, int32 bright);
void Radar_ChangeBlipColour(int32 i, int32);
void Radar_ChangeBlipDisplay(int32 i, eBlipDisplay display);
void Radar_ChangeBlipScale(int32 i, int32 scale);
void Radar_ClearBlip(int32 i);
void Radar_ClearBlipForEntity(eBlipType type, int32 id);
int Radar_ClipRadarPoly(CVector2D *out, const CVector2D *in);
bool Radar_DisplayThisBlip(int32 i);
void Radar_Draw3dMarkers();
void Radar_DrawBlips();
void Radar_DrawMap();
void Radar_DrawRadarMap();
void Radar_DrawRadarMask();
void Radar_DrawRadarSection(int32 x, int32 y);
void Radar_DrawRadarSprite(uint16 sprite, float x, float y, uint8 alpha);
void Radar_DrawRotatingRadarSprite(Sprite2d* sprite, float x, float y, float angle, int32 alpha);
int32 Radar_GetActualBlipArrayIndex(int32 i);
int32 Radar_GetNewUniqueBlipIndex(int32 i);
uint32 Radar_GetRadarTraceColour(uint32 color, bool bright);
void Radar_Initialise();
void Radar_LoadAllRadarBlips(uint8 *buf, uint32 size);
void Radar_LoadTextures();
void Radar_RemoveRadarSections();
void Radar_SaveAllRadarBlips(uint8*, uint32*);
void Radar_SetBlipSprite(int32 i, int32 icon);
int32 Radar_SetCoordBlip(eBlipType type, CVector pos, uint32, eBlipDisplay);
int32 Radar_SetEntityBlip(eBlipType type, int32, uint32, eBlipDisplay);
int32 Radar_SetShortRangeCoordBlip(eBlipType type, CVector pos, uint32, eBlipDisplay);
void Radar_SetRadarMarkerState(int32 i, bool flag);
void Radar_ShowRadarMarker(CVector pos, uint32 color, float radius);
void Radar_ShowRadarTrace(float x, float y, uint32 size, uint8 red, uint8 green, uint8 blue, uint8 alpha);
void Radar_ShowRadarTraceWithHeight(float x, float y, uint32 size, uint8 red, uint8 green, uint8 blue, uint8 alpha, uint8 mode);
void Radar_Shutdown();
void Radar_CalculateCachedSinCos();
void Radar_DrawEntityBlip(int32 blipId);
void Radar_DrawCoordBlip(int32 blipId);
void Radar_DrawLegend(int32, int32, int32);
#ifdef __cplusplus
}
// Members that C cannot declare yet (references, default arguments, overloads, C++ class values), with C++ linkage
#ifdef MAP_ENHANCEMENTS
#endif
#ifdef MAP_ENHANCEMENTS
#endif
float Radar_LimitRadarPoint(CVector2D &point);
void Radar_StreamRadarSections(const CVector &posn);
void Radar_StreamRadarSections(int32 x, int32 y);
void Radar_TransformRealWorldToTexCoordSpace(CVector2D &out, const CVector2D &in, int32 x, int32 y);
void Radar_TransformRadarPointToRealWorldSpace(CVector2D &out, const CVector2D &in);
void Radar_TransformRadarPointToScreenSpace(CVector2D &out, const CVector2D &in);
void Radar_TransformRealWorldPointToRadarSpace(CVector2D &out, const CVector2D &in);
#endif
//- rouz edit (Claude)
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
