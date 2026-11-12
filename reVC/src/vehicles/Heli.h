#pragma once

#include "Vehicle.h"

class CObject;

#include "HeliNodes.h"
#include "HeliData.h"


enum
{
	HELI_TYPE_RANDOM,
	HELI_TYPE_SCRIPT,
	HELI_TYPE_CATALINA,
};


class CHeli : public CVehicle, public CHeliData
{
public:

	static CHeli *pHelis[NUM_HELIS];
	static int16 NumRandomHelis;
	static uint32 TestForNewRandomHelisTimer;
	static bool CatalinaHeliOn;
	static bool CatalinaHasBeenShotDown;
	static bool ScriptHeliOn;

	CHeli(int32 id, uint8 CreatedBy);

	// from CEntity
	void SetModelIndex(uint32 id);
	void ProcessControl(void);
	void PreRender(void);
	void Render(void);

	void PreRenderAlways(void);
	CObject *SpawnFlyingComponent(int32 component);
	CVector FindSwatPositionRelativeToHeli(int n);
	bool SendDownSwat(void);

	static void InitHelis(void);
	static CHeli *GenerateHeli(bool catalina); // out of class in III PC and later because of SecuROM
	static void UpdateHelis(void);
	static void SpecialHeliPreRender(void);
	static bool TestRocketCollision(CVector *coors);
	static bool TestBulletCollision(CVector *line0, CVector *line1, CVector *bulletPos, int32 damage);
	static bool TestSniperCollision(CVector *line0, CVector *line1);

	static void StartCatalinaFlyBy(void); // out of class in III PC and later because of SecuROM
	static void RemoveCatalinaHeli(void);
	static CHeli *FindPointerToCatalinasHeli(void);
	static void CatalinaTakeOff(void);
	static void MakeCatalinaHeliFlyAway(void);
	static bool HasCatalinaBeenShotDown(void);

	static void ActivateHeli(bool activate);
};
