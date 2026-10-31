//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "VehicleDamageParts.h"
// TODO: move some of this into Vehicle.h

enum eEngineStatus
{
	ENGINE_STATUS_STEAM1 = 100,
	ENGINE_STATUS_STEAM2 = 150,
	ENGINE_STATUS_SMOKE = 200,
	ENGINE_STATUS_ON_FIRE = 225
};

#include "DoorTypes.h"

enum ePanelStatus
{
	PANEL_STATUS_OK,
	PANEL_STATUS_SMASHED1,
	PANEL_STATUS_SMASHED2,
	PANEL_STATUS_MISSING,
};

#include "WheelStatus.h"

enum eLightStatus
{
	LIGHT_STATUS_OK,
	LIGHT_STATUS_BROKEN
};

typedef enum tComponent
{
	COMPONENT_DEFAULT,
	COMPONENT_WHEEL_FRONT_LEFT,
	COMPONENT_WHEEL_FRONT_RIGHT,
	COMPONENT_WHEEL_REAR_LEFT,
	COMPONENT_WHEEL_REAR_RIGHT,
	COMPONENT_DOOR_BONNET,
	COMPONENT_DOOR_BOOT,
	COMPONENT_DOOR_FRONT_LEFT,
	COMPONENT_DOOR_FRONT_RIGHT,
	COMPONENT_DOOR_REAR_LEFT,
	COMPONENT_DOOR_REAR_RIGHT,
	COMPONENT_PANEL_FRONT_LEFT,
	COMPONENT_PANEL_FRONT_RIGHT,
	COMPONENT_PANEL_REAR_LEFT,
	COMPONENT_PANEL_REAR_RIGHT,
	COMPONENT_PANEL_WINDSCREEN,
	COMPONENT_BUMPER_FRONT,
	COMPONENT_BUMPER_REAR,
} tComponent;

typedef enum tComponentGroup
{
	COMPGROUP_BUMPER,
	COMPGROUP_WHEEL,
	COMPGROUP_DOOR,
	COMPGROUP_BONNET,
	COMPGROUP_BOOT,
	COMPGROUP_PANEL,
	COMPGROUP_DEFAULT,
} tComponentGroup;


typedef struct CDamageManager {
	float m_fWheelDamageEffect;
	uint8_t m_engineStatus;
	uint8_t m_wheelStatus[4];
	uint8_t m_doorStatus[6];
	uint32_t m_lightStatus;
	uint32_t m_panelStatus;
	uint8_t field_18;

} CDamageManager;
#ifdef __cplusplus
extern "C" {
#endif
extern float G_aComponentDamage[7];
void Damage_Init(CDamageManager *self);
void Damage_ResetDamageStatus(CDamageManager *self);
void Damage_FuckCarCompletely(CDamageManager *self);
bool Damage_ApplyDamage(CDamageManager *self,int32_t component,float damage,float unused);
bool Damage_GetComponentGroup(CDamageManager *self,int32_t component,tComponentGroup *componentGroup,uint8_t *subComp);
void Damage_SetDoorStatus(CDamageManager *self,int32_t door,uint32_t status);
int32_t Damage_GetDoorStatus(CDamageManager *self,int32_t door);
bool Damage_ProgressDoorDamage(CDamageManager *self,uint8_t door);
void Damage_SetPanelStatus(CDamageManager *self,int32_t panel,uint32_t status);
int32_t Damage_GetPanelStatus(CDamageManager *self,int32_t panel);
bool Damage_ProgressPanelDamage(CDamageManager *self,uint8_t panel);
int32_t Damage_GetPanelStatusFromWord(uint32_t panelstatus,int32_t panel);
void Damage_SetLightStatus(CDamageManager *self,int32_t light,uint32_t status);
int32_t Damage_GetLightStatus(CDamageManager *self,int32_t light);
void Damage_SetWheelStatus(CDamageManager *self,int32_t wheel,uint32_t status);
int32_t Damage_GetWheelStatus(CDamageManager *self,int32_t wheel);
bool Damage_ProgressWheelDamage(CDamageManager *self,uint8_t wheel);
void Damage_SetEngineStatus(CDamageManager *self,uint32_t status);
int32_t Damage_GetEngineStatus(CDamageManager *self);
bool Damage_ProgressEngineDamage(CDamageManager *self);
#ifdef __cplusplus
}
#if defined(VALIDATE_SIZE)
VALIDATE_SIZE(CDamageManager,0x1C);
#endif
#endif
//- rouz edit (ChatGPT)
