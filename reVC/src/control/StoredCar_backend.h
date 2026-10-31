//+ rouz edit (ChatGPT)
#pragma once
#include "StoredCar.h"
#ifdef __cplusplus
class CVehicleModelInfo;class CPed;
extern "C" {
#else
typedef struct CVehicleModelInfo CVehicleModelInfo;
typedef struct CPed CPed;
#endif
int32_t StoredCar_BackendModelIndex(CVehicle *vehicle);
StoredCarVector StoredCar_BackendPosition(CVehicle *vehicle);
StoredCarVector StoredCar_BackendForward(CVehicle *vehicle);
uint8_t StoredCar_BackendPrimaryColor(CVehicle *vehicle);
uint8_t StoredCar_BackendSecondaryColor(CVehicle *vehicle);
uint8_t StoredCar_BackendRadioStation(CVehicle *vehicle);
int8_t StoredCar_BackendVariationA(CVehicle *vehicle);
int8_t StoredCar_BackendVariationB(CVehicle *vehicle);
bool StoredCar_BackendBulletProof(CVehicle *vehicle);
void StoredCar_BackendSetBulletProof(CVehicle *vehicle, bool value);
bool StoredCar_BackendFireProof(CVehicle *vehicle);
void StoredCar_BackendSetFireProof(CVehicle *vehicle, bool value);
bool StoredCar_BackendExplosionProof(CVehicle *vehicle);
void StoredCar_BackendSetExplosionProof(CVehicle *vehicle, bool value);
bool StoredCar_BackendCollisionProof(CVehicle *vehicle);
void StoredCar_BackendSetCollisionProof(CVehicle *vehicle, bool value);
bool StoredCar_BackendMeleeProof(CVehicle *vehicle);
void StoredCar_BackendSetMeleeProof(CVehicle *vehicle, bool value);
bool StoredCar_BackendIsCar(CVehicle *vehicle);
bool StoredCar_BackendIsBike(CVehicle *vehicle);
int8_t StoredCar_BackendBombType(CVehicle *vehicle);
void StoredCar_BackendRequestModel(int32_t model, int32_t flags);
bool StoredCar_BackendHasModelLoaded(int32_t model);
CVehicleModelInfo * StoredCar_BackendModelInfo(int32_t model);
int8_t StoredCar_BackendNumComponents(CVehicleModelInfo *info);
void StoredCar_BackendSetComponents(int8_t first, int8_t second);
bool StoredCar_BackendIsBoatModel(int32_t model);
bool StoredCar_BackendIsBikeModel(int32_t model);
CVehicle * StoredCar_BackendAllocateVehicle(void);
void StoredCar_BackendConstructBoat(CVehicle *vehicle, int32_t model);
void StoredCar_BackendConstructBike(CVehicle *vehicle, int32_t model);
void StoredCar_BackendConstructAutomobile(CVehicle *vehicle, int32_t model);
void StoredCar_BackendSetStanding(CVehicle *vehicle, bool value);
void StoredCar_BackendSetPosition(CVehicle *vehicle, StoredCarVector value);
void StoredCar_BackendSetForward(CVehicle *vehicle, StoredCarVector value);
void StoredCar_BackendSetRight(CVehicle *vehicle, StoredCarVector value);
void StoredCar_BackendSetUp(CVehicle *vehicle, StoredCarVector value);
void StoredCar_BackendSetStatus(CVehicle *vehicle, int32_t status);
void StoredCar_BackendClearDriver(CVehicle *vehicle);
void StoredCar_BackendSetPrimaryColor(CVehicle *vehicle, int8_t value);
void StoredCar_BackendSetSecondaryColor(CVehicle *vehicle, int8_t value);
void StoredCar_BackendSetRadioStation(CVehicle *vehicle, int8_t value);
void StoredCar_BackendSetFreebies(CVehicle *vehicle, bool value);
void StoredCar_BackendSetBombType(CVehicle *vehicle, int8_t value);
void StoredCar_BackendSetOwnedByPlayer(CVehicle *vehicle, bool value);
CPed * StoredCar_BackendPlayerPed(void);
void StoredCar_BackendSetBombRigger(CVehicle *vehicle, CPed *ped);
void StoredCar_BackendUnlock(CVehicle *vehicle);
void StoredCar_BackendAssertFailed(const char *expression, const char *file, unsigned line, const char *function);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
