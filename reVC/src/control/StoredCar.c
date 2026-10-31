//+ rouz edit (ChatGPT)
#include "StoredCar_backend.h"
#include "config.h"
#include "EntityStatus.h"
#include "VehicleBombTypes.h"
#include "StreamingInfo.h"
#include <stddef.h>
#ifdef MASTER
#define assert(expression) (expression)
#else
#define assert(expression) (void)((!!(expression))||(StoredCar_BackendAssertFailed(#expression,__FILE__,__LINE__,__func__),0))
#endif

void StoredCar_Init(CStoredCar *self)
{
    // Initialize only the original model sentinel
    self->m_nModelIndex = 0;
}

void StoredCar_Clear(CStoredCar *self)
{
    // Clear only the original model sentinel
    self->m_nModelIndex = 0;
}

bool StoredCar_HasCar(CStoredCar *self)
{
    // Test the stored model without modifying the record
    return self->m_nModelIndex != 0;
}

const CStoredCar *StoredCar_Assign(CStoredCar *self, const CStoredCar *other)
{
    // Copy original members in order while retaining destination tail padding
	self->m_nModelIndex = other->m_nModelIndex;
	self->m_vecPos = other->m_vecPos;
	self->m_vecAngle = other->m_vecAngle;
	self->m_nFlags = other->m_nFlags;
	self->m_nPrimaryColor = other->m_nPrimaryColor;
	self->m_nSecondaryColor = other->m_nSecondaryColor;
	self->m_nRadioStation = other->m_nRadioStation;
	self->m_nVariationA = other->m_nVariationA;
	self->m_nVariationB = other->m_nVariationB;
	self->m_nCarBombType = other->m_nCarBombType;
	return self;
}

void StoredCar_StoreCar(CStoredCar *self, CVehicle *pVehicle)
{
    // Capture the original fields and only store a bomb type for cars or bikes
	self->m_nModelIndex = StoredCar_BackendModelIndex(pVehicle);
	self->m_vecPos = StoredCar_BackendPosition(pVehicle);
	self->m_vecAngle = StoredCar_BackendForward(pVehicle);
	self->m_nPrimaryColor = StoredCar_BackendPrimaryColor(pVehicle);
	self->m_nSecondaryColor = StoredCar_BackendSecondaryColor(pVehicle);
	self->m_nRadioStation = StoredCar_BackendRadioStation(pVehicle);
	self->m_nVariationA = StoredCar_BackendVariationA(pVehicle);
	self->m_nVariationB = StoredCar_BackendVariationB(pVehicle);
	self->m_nFlags = 0;
	if (StoredCar_BackendBulletProof(pVehicle)) self->m_nFlags |= STORED_CAR_FLAG_BULLETPROOF;
	if (StoredCar_BackendFireProof(pVehicle)) self->m_nFlags |= STORED_CAR_FLAG_FIREPROOF;
	if (StoredCar_BackendExplosionProof(pVehicle)) self->m_nFlags |= STORED_CAR_FLAG_EXPLOSIONPROOF;
	if (StoredCar_BackendCollisionProof(pVehicle)) self->m_nFlags |= STORED_CAR_FLAG_COLLISIONPROOF;
	if (StoredCar_BackendMeleeProof(pVehicle)) self->m_nFlags |= STORED_CAR_FLAG_MELEEPROOF;
	if (StoredCar_BackendIsCar(pVehicle) || StoredCar_BackendIsBike(pVehicle))
		self->m_nCarBombType = StoredCar_BackendBombType(pVehicle); // NB: cast to CAutomobile is original behaviour
}

CVehicle *StoredCar_RestoreCar(CStoredCar *self)
{
    // Request the stored model before checking whether restoration can proceed
    StoredCar_BackendRequestModel(self->m_nModelIndex, STREAMFLAGS_DEPENDENCY);
    if (!StoredCar_BackendHasModelLoaded(self->m_nModelIndex))
        return NULL;
#ifdef FIX_BUGS
    // Preserve the fixed model assertion and component-count gate
    CVehicleModelInfo *pModelInfo = StoredCar_BackendModelInfo(self->m_nModelIndex);
    assert(pModelInfo);
    if (StoredCar_BackendNumComponents(pModelInfo) != 0)
#endif
    {
        // Set the original component selections before vehicle construction
        StoredCar_BackendSetComponents(self->m_nVariationA, self->m_nVariationB);
    }

    // Preserve boat-first selection and each allocation, assertion and construction boundary
    CVehicle *pVehicle;
    if (StoredCar_BackendIsBoatModel(self->m_nModelIndex)) {
        pVehicle = StoredCar_BackendAllocateVehicle();
        assert(pVehicle);
        StoredCar_BackendConstructBoat(pVehicle, self->m_nModelIndex);
    } else if (StoredCar_BackendIsBikeModel(self->m_nModelIndex)) {
        CVehicle *pBike = StoredCar_BackendAllocateVehicle();
        assert(pBike);
        StoredCar_BackendConstructBike(pBike, self->m_nModelIndex);
        StoredCar_BackendSetStanding(pBike, true);
        pVehicle = pBike;
    } else {
        pVehicle = StoredCar_BackendAllocateVehicle();
        assert(pVehicle);
        StoredCar_BackendConstructAutomobile(pVehicle, self->m_nModelIndex);
    }

    // Restore the transform and identity fields in their original order
    StoredCar_BackendSetPosition(pVehicle, self->m_vecPos);
    StoredCar_BackendSetStatus(pVehicle, STATUS_ABANDONED);
    StoredCar_BackendSetForward(pVehicle, self->m_vecAngle);
    const StoredCarVector right = { self->m_vecAngle.y, -self->m_vecAngle.x, 0.0f };
    StoredCar_BackendSetRight(pVehicle, right);
    const StoredCarVector up = { 0.0f, 0.0f, 1.0f };
    StoredCar_BackendSetUp(pVehicle, up);
    StoredCar_BackendClearDriver(pVehicle);
    StoredCar_BackendSetPrimaryColor(pVehicle, self->m_nPrimaryColor);
    StoredCar_BackendSetSecondaryColor(pVehicle, self->m_nSecondaryColor);
    StoredCar_BackendSetRadioStation(pVehicle, self->m_nRadioStation);
    StoredCar_BackendSetFreebies(pVehicle, false);
    if (StoredCar_BackendIsCar(pVehicle)) {
        // Restore only car bombs and retain the fixed nonempty-bomb ownership rule
        StoredCar_BackendSetBombType(pVehicle, self->m_nCarBombType);
#ifdef FIX_BUGS
        if (self->m_nCarBombType != CARBOMB_NONE)
            StoredCar_BackendSetBombRigger(pVehicle, StoredCar_BackendPlayerPed());
#endif
    }

    // Preserve ownership, unlock and one-way proof flag restoration
    StoredCar_BackendSetOwnedByPlayer(pVehicle, true);
    StoredCar_BackendUnlock(pVehicle);
    if (self->m_nFlags & STORED_CAR_FLAG_BULLETPROOF) StoredCar_BackendSetBulletProof(pVehicle, true);
    if (self->m_nFlags & STORED_CAR_FLAG_FIREPROOF) StoredCar_BackendSetFireProof(pVehicle, true);
    if (self->m_nFlags & STORED_CAR_FLAG_EXPLOSIONPROOF) StoredCar_BackendSetExplosionProof(pVehicle, true);
    if (self->m_nFlags & STORED_CAR_FLAG_COLLISIONPROOF) StoredCar_BackendSetCollisionProof(pVehicle, true);
    if (self->m_nFlags & STORED_CAR_FLAG_MELEEPROOF) StoredCar_BackendSetMeleeProof(pVehicle, true);
    return pVehicle;
}
//- rouz edit (ChatGPT)
