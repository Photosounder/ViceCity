#pragma once

#include "Physical.h"
#include "ObjectState.h"

enum CollisionDamageEffect
{
	DAMAGE_EFFECT_NONE,
	DAMAGE_EFFECT_CHANGE_MODEL,
	DAMAGE_EFFECT_SPLIT_MODEL,
	DAMAGE_EFFECT_SMASH_AND_DAMAGE_TRAFFICLIGHTS,

	DAMAGE_EFFECT_SMASH_COMPLETELY = 20,
	DAMAGE_EFFECT_CHANGE_THEN_SMASH,

	DAMAGE_EFFECT_SMASH_CARDBOARD_COMPLETELY = 50,
	DAMAGE_EFFECT_SMASH_YELLOW_TARGET_COMPLETELY = 51,

	DAMAGE_EFFECT_SMASH_WOODENBOX_COMPLETELY = 60,
	DAMAGE_EFFECT_SMASH_TRAFFICCONE_COMPLETELY = 70,
	DAMAGE_EFFECT_SMASH_BARPOST_COMPLETELY = 80,

	DAMAGE_EFFECT_SMASH_NEWSTANDNEW1 = 91,
	DAMAGE_EFFECT_SMASH_NEWSTANDNEW2 = 92,
	DAMAGE_EFFECT_SMASH_NEWSTANDNEW3 = 93,
	DAMAGE_EFFECT_SMASH_NEWSTANDNEW4 = 94,
	DAMAGE_EFFECT_SMASH_NEWSTANDNEW5 = 95,

	DAMAGE_EFFECT_SMASH_BLACKBAG = 100,
	DAMAGE_EFFECT_SMASH_VEGPALM = 110,
	DAMAGE_EFFECT_BURST_BEACHBALL = 120,
	DAMAGE_EFFECT_SMASH_BEACHLOUNGE_WOOD = 131,
	DAMAGE_EFFECT_SMASH_BEACHLOUNGE_TOWEL = 132,
};

class CVehicle;
class CDummyObject;

class CObject : public CPhysical, public CObjectState
{
public:

	static int16 nNoTempObjects;
	static float fDistToNearestTree;

	CObject(void);
	CObject(int32, bool);
	CObject(CDummyObject*);
	~CObject(void);

	void ProcessControl(void);
	void Teleport(CVector vecPos);
	void Render(void);
	bool SetupLighting(void);
	void RemoveLighting(bool reset);

	void ObjectDamage(float amount);
	void RefModelInfo(int32 modelId);
	void Init(void);
	bool CanBeDeleted(void);

	static void DeleteAllMissionObjects();
	static void DeleteAllTempObjects();
	static void DeleteAllTempObjectsInArea(CVector point, float fRadius);
};

bool IsObjectPointerValid(CObject* pObject);
