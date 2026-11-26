//+ rouz edit (ChatGPT)
#pragma once

class CEntity;
class CVehicle;
struct CPtrList;

class CFallingGlassPane : public CMatrix
{
public:
	CVector m_vecMoveSpeed;
	CVector m_vecTurn;
	uint32 m_nTimer;
	float m_fGroundZ;
	float m_fStep;
	uint8 m_nTriIndex;
	bool m_bActive;
	bool m_bShattered;
	bool m_bCarGlass;

	CFallingGlassPane()  { }
	~CFallingGlassPane() { }

	void Update(void);
	void Render(void);
};

VALIDATE_SIZE(CFallingGlassPane, 0x70);

enum
{
	NUM_GLASSTRIANGLES = 5,
};

//+ rouz edit (Claude)
// Formerly the static class CGlass, now C globals and functions shared with C
#ifdef __cplusplus
extern "C" {
#endif
extern uint32 Glass_NumGlassEntities;
extern CEntity * Glass_apEntitiesToBeRendered[NUM_GLASSENTITIES];
void Glass_Init(void);
void Glass_Update(void);
void Glass_Render(void);
CFallingGlassPane *Glass_FindFreePane(void);
void Glass_GeneratePanesForWindow(uint32 type, CVector pos, CVector up, CVector right, CVector speed, CVector center, float moveSpeed, bool cracked, bool explosion, int32 stepmul, bool carGlass);
void Glass_AskForObjectToBeRenderedInGlass(CEntity *entity);
void Glass_RenderEntityInGlass(CEntity *entity);
int32 Glass_CalcAlphaWithNormal(CVector *normal);
void Glass_RenderHiLightPolys(void);
void Glass_RenderShatteredPolys(void);
void Glass_RenderReflectionPolys(void);
void Glass_WindowRespondsToCollision(CEntity *entity, float amount, CVector speed, CVector point, bool explosion);
void Glass_WindowRespondsToSoftCollision(CEntity *entity, float amount);
void Glass_WasGlassHitByBullet(CEntity *entity, CVector point);
void Glass_WindowRespondsToExplosion(CEntity *entity, CVector point);
void Glass_CarWindscreenShatters(CVehicle *vehicle, bool unk);
bool Glass_HasGlassBeenShatteredAtCoors(float x, float y, float z);
void Glass_BreakGlassPhysically(CVector pos, float radius);
#ifdef __cplusplus
}
// Members that C cannot declare yet (references, default arguments, overloads, C++ class values), with C++ linkage
extern CFallingGlassPane Glass_aGlassPanes[NUM_GLASSPANES];
void Glass_FindWindowSectorList(CPtrList &list, float *dist, CEntity **entity, float x, float y, float z);
#endif
//- rouz edit (Claude)
//- rouz edit (ChatGPT)
