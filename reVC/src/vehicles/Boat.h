#pragma once

#include "Vehicle.h"
#include "Door.h"

#include "BoatNodes.h"
#include "BoatData.h"

class CBoat : public CVehicle, public CBoatData
{
public:

	static float MAX_WAKE_LENGTH;
	static float MIN_WAKE_INTERVAL;
	static float WAKE_LIFETIME;

	CBoat(int, uint8);

	virtual void SetModelIndex(uint32 id);
	virtual void ProcessControl();
	virtual void Teleport(CVector v);
	virtual void PreRender(void);
	virtual void Render(void);
	virtual void ProcessControlInputs(uint8);
	virtual void GetComponentWorldPosition(int32 component, CVector &pos);
	virtual bool IsComponentPresent(int32 component) { return true; }
	virtual void BlowUpCar(CEntity *ent);

	void RenderWaterOutPolys(void);
	void ApplyWaterResistance(void);
	void SetupModelNodes();
	void PruneWakeTrail(void);
	void AddWakePoint(CVector point);
	void DoDriveByShootings(void);

	static CBoat *apFrameWakeGeneratingBoats[4];
	
	static bool IsSectorAffectedByWake(CVector2D sector, float fSize, CBoat **apBoats);
	static float IsVertexAffectedByWake(CVector vecVertex, CBoat *pBoat);
	static void FillBoatList(void);

#ifdef COMPATIBLE_SAVES
	virtual void Save(uint8*& buf);
	virtual void Load(uint8*& buf);
#endif
	static const uint32 nSaveStructSize;

};