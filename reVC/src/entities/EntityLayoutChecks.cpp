//+ rouz edit (Claude)
#include <stddef.h>
#include "common.h"
#include "Entity.h"
#include "Physical.h"
#include "Vehicle.h"
#include "Ped.h"
#include "Object.h"
#include "Automobile.h"
#include "Boat.h"
#include "Bike.h"
#include "Heli.h"
#include "PathFind.h"
#include "Radar.h"
#include "HeliLayout.h"
#include "BikeLayout.h"
#include "BoatLayout.h"
#include "AutomobileLayout.h"
#include "EntityLayout.h"

// The C views must be exactly the C++ objects, this fails to compile if a layout drifts
#define LAYOUT_OFFSET(type, member) offsetof(type, member)

static_assert(sizeof(CEntityStruct) == sizeof(CEntity), "CEntityStruct size");
static_assert(LAYOUT_OFFSET(CEntity, m_matrix) == offsetof(CEntityStruct, placeable.m_matrix), "CEntity m_matrix");
static_assert(LAYOUT_OFFSET(CEntity, orig_pos_set) == offsetof(CEntityStruct, placeable.orig_pos_set), "CEntity orig_pos_set");
static_assert(LAYOUT_OFFSET(CEntity, m_rwObject) == offsetof(CEntityStruct, entity.m_rwObject), "CEntity m_rwObject");
static_assert(LAYOUT_OFFSET(CEntity, m_modelIndex) == offsetof(CEntityStruct, entity.m_modelIndex), "CEntity m_modelIndex");
static_assert(LAYOUT_OFFSET(CEntity, m_pFirstReference) == offsetof(CEntityStruct, entity.m_pFirstReference), "CEntity m_pFirstReference");

static_assert(sizeof(CPhysicalStruct) == sizeof(CPhysical), "CPhysicalStruct size");
static_assert(LAYOUT_OFFSET(CPhysical, m_audioEntityId) == offsetof(CPhysicalStruct, physical.m_audioEntityId), "CPhysical m_audioEntityId");
static_assert(LAYOUT_OFFSET(CPhysical, m_vecMoveSpeed) == offsetof(CPhysicalStruct, physical.m_vecMoveSpeed), "CPhysical m_vecMoveSpeed");
static_assert(LAYOUT_OFFSET(CPhysical, m_nZoneLevel) == offsetof(CPhysicalStruct, physical.m_nZoneLevel), "CPhysical m_nZoneLevel");

static_assert(sizeof(CVehicleStruct) == sizeof(CVehicle), "CVehicleStruct size");
static_assert(LAYOUT_OFFSET(CVehicle, pHandling) == offsetof(CVehicleStruct, vehicle.pHandling), "CVehicle pHandling");
static_assert(LAYOUT_OFFSET(CVehicle, pDriver) == offsetof(CVehicleStruct, vehicle.pDriver), "CVehicle pDriver");
static_assert(LAYOUT_OFFSET(CVehicle, m_fHealth) == offsetof(CVehicleStruct, vehicle.m_fHealth), "CVehicle m_fHealth");
static_assert(LAYOUT_OFFSET(CVehicle, m_vehType) == offsetof(CVehicleStruct, vehicle.m_vehType), "CVehicle m_vehType");

static_assert(sizeof(CPedStruct) == sizeof(CPed), "CPedStruct size");
static_assert(LAYOUT_OFFSET(CPed, m_collPoly) == offsetof(CPedStruct, ped.m_collPoly), "CPed m_collPoly");
static_assert(LAYOUT_OFFSET(CPed, m_nPedState) == offsetof(CPedStruct, ped.m_nPedState), "CPed m_nPedState");
static_assert(LAYOUT_OFFSET(CPed, m_weapons) == offsetof(CPedStruct, ped.m_weapons), "CPed m_weapons");
static_assert(LAYOUT_OFFSET(CPed, surf_rel_origin) == offsetof(CPedStruct, ped.surf_rel_origin), "CPed surf_rel_origin");

static_assert(sizeof(CObjectStruct) == sizeof(CObject), "CObjectStruct size");
static_assert(LAYOUT_OFFSET(CObject, m_objectMatrix) == offsetof(CObjectStruct, object.m_objectMatrix), "CObject m_objectMatrix");
static_assert(LAYOUT_OFFSET(CObject, ObjectCreatedBy) == offsetof(CObjectStruct, object.ObjectCreatedBy), "CObject ObjectCreatedBy");
static_assert(LAYOUT_OFFSET(CObject, m_nEndOfLifeTime) == offsetof(CObjectStruct, object.m_nEndOfLifeTime), "CObject m_nEndOfLifeTime");
static_assert(LAYOUT_OFFSET(CObject, m_colour2) == offsetof(CObjectStruct, object.m_colour2), "CObject m_colour2");

static_assert(sizeof(CAutomobileStruct) == sizeof(CAutomobile), "CAutomobileStruct size");
static_assert(LAYOUT_OFFSET(CAutomobile, Damage) == offsetof(CAutomobileStruct, automobile.Damage), "CAutomobile Damage");
static_assert(LAYOUT_OFFSET(CAutomobile, Doors) == offsetof(CAutomobileStruct, automobile.Doors), "CAutomobile Doors");
static_assert(LAYOUT_OFFSET(CAutomobile, m_aSuspensionSpringRatio) == offsetof(CAutomobileStruct, automobile.m_aSuspensionSpringRatio), "CAutomobile m_aSuspensionSpringRatio");
static_assert(LAYOUT_OFFSET(CAutomobile, m_aWheelState) == offsetof(CAutomobileStruct, automobile.m_aWheelState), "CAutomobile m_aWheelState");
static_assert(LAYOUT_OFFSET(CAutomobile, m_nDriveWheelsOnGround) == offsetof(CAutomobileStruct, automobile.m_nDriveWheelsOnGround), "CAutomobile m_nDriveWheelsOnGround");

static_assert(sizeof(CBoatStruct) == sizeof(CBoat), "CBoatStruct size");
static_assert(LAYOUT_OFFSET(CBoat, m_fMovingRotation) == offsetof(CBoatStruct, boat.m_fMovingRotation), "CBoat m_fMovingRotation");
static_assert(LAYOUT_OFFSET(CBoat, m_aBoatNodes) == offsetof(CBoatStruct, boat.m_aBoatNodes), "CBoat m_aBoatNodes");
static_assert(LAYOUT_OFFSET(CBoat, m_boom) == offsetof(CBoatStruct, boat.m_boom), "CBoat m_boom");
static_assert(LAYOUT_OFFSET(CBoat, m_nNumWakePoints) == offsetof(CBoatStruct, boat.m_nNumWakePoints), "CBoat m_nNumWakePoints");
static_assert(LAYOUT_OFFSET(CBoat, m_avec2dWakePoints) == offsetof(CBoatStruct, boat.m_avec2dWakePoints), "CBoat m_avec2dWakePoints");
static_assert(LAYOUT_OFFSET(CBoat, m_afWakePointLifeTime) == offsetof(CBoatStruct, boat.m_afWakePointLifeTime), "CBoat m_afWakePointLifeTime");

static_assert(sizeof(CBikeStruct) == sizeof(CBike), "CBikeStruct size");
static_assert(LAYOUT_OFFSET(CBike, m_aBikeNodes) == offsetof(CBikeStruct, bike.m_aBikeNodes), "CBike m_aBikeNodes");
static_assert(LAYOUT_OFFSET(CBike, m_leanMatrix) == offsetof(CBikeStruct, bike.m_leanMatrix), "CBike m_leanMatrix");
static_assert(LAYOUT_OFFSET(CBike, m_aWheelColPoints) == offsetof(CBikeStruct, bike.m_aWheelColPoints), "CBike m_aWheelColPoints");
static_assert(LAYOUT_OFFSET(CBike, m_aWheelRotation) == offsetof(CBikeStruct, bike.m_aWheelRotation), "CBike m_aWheelRotation");
static_assert(LAYOUT_OFFSET(CBike, m_aGroundPhysical) == offsetof(CBikeStruct, bike.m_aGroundPhysical), "CBike m_aGroundPhysical");
static_assert(LAYOUT_OFFSET(CBike, m_aWheelState) == offsetof(CBikeStruct, bike.m_aWheelState), "CBike m_aWheelState");

static_assert(sizeof(CHeliStruct) == sizeof(CHeli), "CHeliStruct size");
static_assert(LAYOUT_OFFSET(CHeli, m_aHeliNodes) == offsetof(CHeliStruct, heli.m_aHeliNodes), "CHeli m_aHeliNodes");
static_assert(LAYOUT_OFFSET(CHeli, m_nExplosionTimer) == offsetof(CHeliStruct, heli.m_nExplosionTimer), "CHeli m_nExplosionTimer");
static_assert(LAYOUT_OFFSET(CHeli, m_aSwatState) == offsetof(CHeliStruct, heli.m_aSwatState), "CHeli m_aSwatState");
static_assert(LAYOUT_OFFSET(CHeli, m_fHeliDustZ) == offsetof(CHeliStruct, heli.m_fHeliDustZ), "CHeli m_fHeliDustZ");
static_assert(LAYOUT_OFFSET(CHeli, m_bTestRight) == offsetof(CHeliStruct, heli.m_bTestRight), "CHeli m_bTestRight");

static_assert(sizeof(CCarPathLink) == 12, "CCarPathLink size");
static_assert(sizeof(CCarPathLinkData) == sizeof(CCarPathLink), "CCarPathLinkData size");
static_assert(sizeof(sRadarTrace) == 56, "sRadarTrace size");
static_assert(sizeof(sRadarTraceData) == sizeof(sRadarTrace), "sRadarTraceData size");
//- rouz edit (Claude)
