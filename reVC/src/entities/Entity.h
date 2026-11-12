//+ rouz edit (ChatGPT)
#pragma once

#include "ModelInfo.h"
#include "Placeable.h"
#include "EntityData.h"
#include "EntityLogic.h"

struct CReference;
struct CPtrList;

#include "EntityTypes.h"

#include "EntityStatus.h"

class CEntity : public CPlaceable, public CEntityData
{
public:
	uint8 GetType() const { return m_type; }
	void SetType(uint8 type) { m_type = type; }
	uint8 GetStatus() const { return m_status; }
	void SetStatus(uint8 status) { m_status = status; }
	CColModel *GetColModel(void) { return ((CBaseModelInfo*)ModelRegistry_Get(m_modelIndex))->GetColModel(); } // rouz edit (ChatGPT)
	bool GetIsStatic(void) const { return Entity_GetIsStatic(this); }
	void SetIsStatic(bool state) { bIsStatic = state; }
#ifdef COMPATIBLE_SAVES
	void SaveEntityFlags(uint8*& buf);
	void LoadEntityFlags(uint8*& buf);
#else
	uint32* GetAddressOfEntityProperties() { /* AWFUL */ return (uint32*)((char*)&m_rwObject + sizeof(m_rwObject)); }
#endif

	CEntity(void);
	virtual ~CEntity(void);

	virtual void Add(void);
	virtual void Remove(void);
	virtual void SetModelIndex(uint32 id);
	virtual void SetModelIndexNoCreate(uint32 id);
	virtual void CreateRwObject(void);
	virtual void DeleteRwObject(void);
	virtual CRect GetBoundRect(void);
	virtual void ProcessControl(void) {}
	virtual void ProcessCollision(void) {}
	virtual void ProcessShift(void) {}
	virtual void Teleport(CVector v) {}
	virtual void PreRender(void);
	virtual void Render(void);
	virtual bool SetupLighting(void);
	virtual void RemoveLighting(bool);
	virtual void FlagToDestroyWhenNextProcessed(void) {}

	bool IsBuilding(void) { return Entity_IsBuilding(this); }
	bool IsVehicle(void) { return Entity_IsVehicle(this); }
	bool IsPed(void) { return Entity_IsPed(this); }
	bool IsObject(void) { return Entity_IsObject(this); }
	bool IsDummy(void) { return Entity_IsDummy(this); }

	RpAtomic *GetAtomic(void) {
		assert(RwObjectGetType(m_rwObject) == rpATOMIC);
		return (RpAtomic*)m_rwObject;
	}
	RpClump *GetClump(void) {
		assert(RwObjectGetType(m_rwObject) == rpCLUMP);
		return (RpClump*)m_rwObject;
	}

	void GetBoundCentre(CVUVECTOR &out);
	CVector GetBoundCentre(void);
	float GetBoundRadius(void);
	float GetDistanceFromCentreOfMassToBaseOfModel(void);
	bool GetIsTouching(CVUVECTOR const &center, float r);
	bool GetIsOnScreen(void);
	bool GetIsOnScreenComplex(void);
	bool IsVisible(void);
	bool IsVisibleComplex(void);
	bool IsEntityOccluded(void);
	int16 GetModelIndex(void) const { return m_modelIndex; }
	void UpdateRwFrame(void);
	void SetupBigBuilding(void);
	bool HasPreRenderEffects(void);

	void AttachToRwObject(RwObject *obj);
	void DetachFromRwObject(void);

	void RegisterReference(CEntity **pent);
	void ResolveReferences(void);
	void PruneReferences(void);
	void CleanUpOldReference(CEntity **pent);

	void UpdateRpHAnim(void);

	void PreRenderForGlassWindow(void);
	void AddSteamsFromGround(CVector *unused);
	void ModifyMatrixForTreeInWind(void);
	void ModifyMatrixForBannerInWind(void);
	void ProcessLightsForEntity(void);
	void SetRwObjectAlpha(int32 alpha);
};

bool IsEntityPointerValid(CEntity*);

//- rouz edit (ChatGPT)
