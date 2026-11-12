//+ rouz edit (Claude)
#pragma once
#include "FloaterMath.h"

class CPhysical;

enum tWaterLevel
{
	FLOATER_ABOVE_WATER,
	FLOATER_IN_WATER,
	FLOATER_UNDER_WATER,
};

class cBuoyancy : public FloaterState
{
public:
	bool ProcessBuoyancy(CPhysical *phys, float buoyancy, CVector *point, CVector *impulse);
	bool ProcessBuoyancyBoat(CVehicle *phys, float buoyancy, CVector *point, CVector *impulse, bool bNoTurnForce);
	void PreCalcSetup(CPhysical *phys, float buoyancy);
	void SimpleCalcBuoyancy(void);
	float SimpleSumBuoyancyData(CVector &waterLevel, tWaterLevel waterPosition);
	void FindWaterLevel(const CVector &zpos, CVector *waterLevel, tWaterLevel *waterPosition);
	void FindWaterLevelNorm(const CVector &zpos, CVector *waterLevel, tWaterLevel *waterPosition, CVector *normal);
	bool CalcBuoyancyForce(CPhysical *phys, CVector *impulse, CVector *point);
};
extern cBuoyancy mod_Buoyancy;
//- rouz edit (Claude)
