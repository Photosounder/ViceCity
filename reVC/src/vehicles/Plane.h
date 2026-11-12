#pragma once

#include "Vehicle.h"

#include "PlaneNodes.h"

class CPlane : public CVehicle
{
public:
#ifdef CPLANE_ROTORS
	RwFrame *m_aPlaneNodes[NUM_PLANE_NODES];
	float m_fRotorRotation;
#endif
	int16 m_nPlaneId;
	int16 m_isFarAway;
	int16 m_nCurPathNode;
	float m_fSpeed;
	uint32 m_nFrameWhenHit;
	bool m_bHasBeenHit;
	bool m_bIsDrugRunCesna;
	bool m_bIsDropOffCesna;
	bool m_bTempPlane;
	bool m_bHasCollisionVelocity; // rouz edit (ChatGPT)

	CPlane(int32 id, uint8 CreatedBy);
	~CPlane(void);

	// from CEntity
	void SetModelIndex(uint32 id);
	void DeleteRwObject(void);
	void ProcessControl(void);
	void ProcessCollision(void); // rouz edit (ChatGPT)
	void PreRender(void);
	void Render(void);
	void FlagToDestroyWhenNextProcessed() { bRemoveFromWorld = true; }

	static void InitPlanes(void);
	static void Shutdown(void);
	static CPlaneNode *LoadPath(char const *filename, int32 &numNodes, float &totalLength, bool loop);
	static void RemoveTemporaryPlanes(void);
	static void UpdatePlanes(void);
	static bool TestRocketCollision(CVector *rocketPos);
	static void CreateIncomingCesna(void);
	static void CreateDropOffCesna(void);
	static const CVector FindDrugPlaneCoordinates(void);
	static const CVector FindDropOffCesnaCoordinates(void);
	static bool HasCesnaLanded(void);
	static bool HasCesnaBeenDestroyed(void);
	static bool HasDropOffCesnaBeenShotDown(void);
	static void Load(void);
	static void Save(void);
};

extern float LandingPoint;
extern float TakeOffPoint;
extern float PlanePathPosition[3];
