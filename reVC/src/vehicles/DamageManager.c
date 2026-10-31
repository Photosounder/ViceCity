//+ rouz edit (ChatGPT)
#include "DamageManager.h"
#include "config.h"
#define ARRAY_SIZE(array) (sizeof(array)/sizeof((array)[0]))
static inline uint32_t Damage_DepositBits(uint32_t bits,uint32_t position,uint32_t size,uint32_t word)
{
    // Preserve the original packed bit pattern with unsigned C shifts
    uint32_t mask=((UINT32_C(1)<<size)-UINT32_C(1))<<position;
    return (word & ~mask) | ((bits<<position) & mask);
}
static inline uint32_t Damage_LoadBits(uint32_t position,uint32_t size,uint32_t word)
{
    // Extract the same low-width field from the packed word
    return word>>position & ((UINT32_C(1)<<size)-UINT32_C(1));
}
float G_aComponentDamage[] = { 2.5f, 1.25f, 3.2f, 1.4f, 2.5f, 2.8f, 0.5f };

void
Damage_Init(CDamageManager *self)
{
    // Preserve the complete original damage operation and its ordered field accesses
	Damage_ResetDamageStatus(self);
	self->m_fWheelDamageEffect = 0.5f;
	self->field_18 = 1;
}

void
Damage_ResetDamageStatus(CDamageManager *self)
{
    // Preserve the complete original damage operation and its ordered field accesses
	int i;
	self->m_fWheelDamageEffect = 0.5f; // rouz edit (ChatGPT)
	self->m_engineStatus = 0;
	for(i = 0; i < ARRAY_SIZE(self->m_wheelStatus); i++) self->m_wheelStatus[i] = 0;
	for(i = 0; i < ARRAY_SIZE(self->m_doorStatus); i++) self->m_doorStatus[i] = 0;
	self->m_lightStatus = 0;
	self->m_panelStatus = 0;
}

void
Damage_FuckCarCompletely(CDamageManager *self)
{
    // Preserve the complete original damage operation and its ordered field accesses
	int i;

	self->m_wheelStatus[0] = WHEEL_STATUS_MISSING;
	// wheels 1-3 not reset?

	for(i = 0; i < ARRAY_SIZE(self->m_doorStatus); i++)
		self->m_doorStatus[i] = DOOR_STATUS_MISSING;

	for(i = 0; i < 3; i++){
#ifdef FIX_BUGS
		Damage_ProgressPanelDamage(self, VEHBUMPER_FRONT);
		Damage_ProgressPanelDamage(self, VEHBUMPER_REAR);
#else
		// this can't be right
		Damage_ProgressPanelDamage(self, COMPONENT_BUMPER_FRONT);
		Damage_ProgressPanelDamage(self, COMPONENT_BUMPER_REAR);
#endif
	}
	// Why set to no damage?
#ifndef FIX_BUGS
	self->m_lightStatus = 0;
	self->m_panelStatus = 0;
#endif
	Damage_SetEngineStatus(self, 250);
}

bool
Damage_ApplyDamage(CDamageManager *self,int32_t component, float damage, float unused)
{
    // Retain the unused scalar argument without changing the damage calculation
    (void)unused;
    // Preserve the complete original damage operation and its ordered field accesses
	tComponentGroup group;
	uint8_t subComp;

	Damage_GetComponentGroup(self, component, &group, &subComp);
	damage *= G_aComponentDamage[group];
	if(component == COMPONENT_PANEL_WINDSCREEN)
		damage *= 0.6f;
	if(damage > 150.0f){
		switch(group){
		case COMPGROUP_WHEEL:
			Damage_ProgressWheelDamage(self, subComp);
			break;
		case COMPGROUP_DOOR:
		case COMPGROUP_BOOT:
			Damage_ProgressDoorDamage(self, subComp);
			break;
		case COMPGROUP_BONNET:
			if(damage > 220.0f)
				Damage_ProgressEngineDamage(self);
			Damage_ProgressDoorDamage(self, subComp);
			break;
		case COMPGROUP_PANEL:
			// so windscreen is a light?
			Damage_SetLightStatus(self, (int32_t)subComp, 1);
			// fall through
		case COMPGROUP_BUMPER:
			if(damage > 220.0f &&
			   (component == COMPONENT_PANEL_FRONT_LEFT ||
			    component == COMPONENT_PANEL_FRONT_RIGHT ||
			    component == COMPONENT_PANEL_WINDSCREEN))
				Damage_ProgressEngineDamage(self);
			Damage_ProgressPanelDamage(self, subComp);
			break;
		default: break;
		}
		return true;
	}
	return false;
}

bool
Damage_GetComponentGroup(CDamageManager *self,int32_t component, tComponentGroup *componentGroup, uint8_t *subComp)
{
    // Keep this stateless operation independent of the record
    (void)self;
    // Preserve the complete original damage operation and its ordered field accesses
	*subComp = -2;	// ??

	// This is done very strangely in the game, maybe an optimized switch?
	if(component >= COMPONENT_PANEL_FRONT_LEFT){
		if(component >= COMPONENT_BUMPER_FRONT)
			*componentGroup = COMPGROUP_BUMPER;
		else
			*componentGroup = COMPGROUP_PANEL;
		*subComp = component - COMPONENT_PANEL_FRONT_LEFT;
		return true;
	}else if(component >= COMPONENT_DOOR_BONNET){
		if(component == COMPONENT_DOOR_BONNET)
			*componentGroup = COMPGROUP_BONNET;
		else if(component == COMPONENT_DOOR_BOOT)
			*componentGroup = COMPGROUP_BOOT;
		else
			*componentGroup = COMPGROUP_DOOR;
		*subComp = component - COMPONENT_DOOR_BONNET;
		return true;
	}else if(component >= COMPONENT_WHEEL_FRONT_LEFT){
		*componentGroup = COMPGROUP_WHEEL;
		*subComp = component - COMPONENT_WHEEL_FRONT_LEFT;
		return true;
	}else if(component >= COMPONENT_DEFAULT){
		*componentGroup = COMPGROUP_DEFAULT;
		*subComp = COMPONENT_DEFAULT;
		return true;
	}else
		return false;
}

void
Damage_SetDoorStatus(CDamageManager *self,int32_t door, uint32_t status)
{
    // Preserve the complete original damage operation and its ordered field accesses
	self->m_doorStatus[door] = status;
}

int32_t
Damage_GetDoorStatus(CDamageManager *self,int32_t door)
{
    // Preserve the complete original damage operation and its ordered field accesses
	return self->m_doorStatus[door];
}

bool
Damage_ProgressDoorDamage(CDamageManager *self,uint8_t door)
{
    // Preserve the complete original damage operation and its ordered field accesses
	int status = Damage_GetDoorStatus(self, door);
	if(status == PANEL_STATUS_MISSING)
		return false;
	Damage_SetDoorStatus(self, door, status+1);
	return true;
}

void
Damage_SetPanelStatus(CDamageManager *self,int32_t panel, uint32_t status)
{
    // Preserve the complete original damage operation and its ordered field accesses
	self->m_panelStatus = Damage_DepositBits(status, panel*4, 4, self->m_panelStatus);
}

int32_t
Damage_GetPanelStatus(CDamageManager *self,int32_t panel)
{
    // Preserve the complete original damage operation and its ordered field accesses
	return Damage_LoadBits(panel*4, 4, self->m_panelStatus);
}

bool
Damage_ProgressPanelDamage(CDamageManager *self,uint8_t panel)
{
    // Preserve the complete original damage operation and its ordered field accesses
	int status = Damage_GetPanelStatus(self, panel);
	if(status == DOOR_STATUS_MISSING)
		return false;
	Damage_SetPanelStatus(self, panel, status+1);
	return true;
}

void
Damage_SetLightStatus(CDamageManager *self,int32_t light, uint32_t status)
{
    // Preserve the complete original damage operation and its ordered field accesses
	self->m_lightStatus = Damage_DepositBits(status, light*2, 2, self->m_lightStatus);
}

int32_t
Damage_GetLightStatus(CDamageManager *self,int32_t light)
{
    // Preserve the complete original damage operation and its ordered field accesses
	return Damage_LoadBits(light*2, 2, self->m_lightStatus);
}

void
Damage_SetWheelStatus(CDamageManager *self,int32_t wheel, uint32_t status)
{
    // Preserve the complete original damage operation and its ordered field accesses
	self->m_wheelStatus[wheel] = status;
}

int32_t
Damage_GetWheelStatus(CDamageManager *self,int32_t wheel)
{
    // Preserve the complete original damage operation and its ordered field accesses
	return self->m_wheelStatus[wheel];
}

bool
Damage_ProgressWheelDamage(CDamageManager *self,uint8_t wheel)
{
    // Preserve the complete original damage operation and its ordered field accesses
	int status = Damage_GetWheelStatus(self, wheel);
	if(status == WHEEL_STATUS_MISSING)
		return false;
	Damage_SetWheelStatus(self, wheel, status+1);
	return true;
}

void
Damage_SetEngineStatus(CDamageManager *self,uint32_t status)
{
    // Preserve the complete original damage operation and its ordered field accesses
	if(status > 250)
		self->m_engineStatus = 250;
	else
		self->m_engineStatus = status;
}

int32_t
Damage_GetEngineStatus(CDamageManager *self)
{
    // Preserve the complete original damage operation and its ordered field accesses
	return self->m_engineStatus;
}

bool
Damage_ProgressEngineDamage(CDamageManager *self)
{
    // Preserve the original cut-feature no-op without reading the record
    (void)self;
    // Preserve the complete original damage operation and its ordered field accesses
	// gone in VC
	return false;
}

int32_t Damage_GetPanelStatusFromWord(uint32_t panelstatus,int32_t panel)
{
    // Preserve the original replay accessor without requiring a damage record
    return Damage_LoadBits(panel*4,4,panelstatus);
}
//- rouz edit (ChatGPT)
