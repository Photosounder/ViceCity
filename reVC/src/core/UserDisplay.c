//+ rouz edit (ChatGPT)
#include "ReplayState.h"
#include "User.h"
#include "User_backend.h"
#include "Zones.h"
#include "Text.h"
#include <stddef.h>
CPlaceName PlaceName_state;
CCurrentVehicle CurrentVehicle_state;

void
PlaceName_Init(CPlaceName *self)
{
    // Preserve the complete original Init operation and its ordered live accesses
	self->m_pZone = NULL;
	self->m_pZone2 = NULL;
	self->m_nAdditionalTimer = 0;
}

void
PlaceName_Process(CPlaceName *self)
{
    // Preserve the complete original Process operation and its ordered live accesses
	ZonesPosition pos = UserDisplay_BackendPlayerPosition();
	CZone *navigZone = Zones_FindSmallestNavigationZoneForPosition(pos, false, true);
	CZone *defaultZone = Zones_FindSmallestNavigationZoneForPosition(pos, true, false);

	if (navigZone == NULL) self->m_pZone = NULL;
	if (defaultZone == NULL) self->m_pZone2 = NULL;

	if (navigZone == self->m_pZone) {
		if (defaultZone == self->m_pZone2 || self->m_pZone != NULL) {
			if (navigZone != NULL || defaultZone != NULL) {
				if (self->m_nAdditionalTimer != 0)
					self->m_nAdditionalTimer--;
			} else {
				self->m_nAdditionalTimer = 0;
				self->m_pZone = NULL;
				self->m_pZone2 = NULL;
			}
		} else {
			self->m_pZone2 = defaultZone;
			self->m_nAdditionalTimer = 250;
		}
	} else {
		self->m_pZone = navigZone;
		self->m_nAdditionalTimer = 250;
	}
	PlaceName_Display(self);
}

void
PlaceName_Display(CPlaceName *self)
{
    // Preserve the complete original Display operation and its ordered live accesses
	uint16_t *text;
	if (self->m_pZone != NULL)
		text = Zone_GetTranslatedName(self->m_pZone);
	else if (self->m_pZone2 != NULL)
		text = Zone_GetTranslatedName(self->m_pZone2);
#ifdef FIX_BUGS
	else
		text = NULL;
#endif
	UserDisplay_BackendSetZoneName(text);
}

void
PlaceName_ProcessAfterFrontEndShutDown(CPlaceName *self)
{
    // Preserve the complete original ProcessAfterFrontEndShutDown operation and its ordered live accesses
	(*UserDisplay_BackendLastZoneName()) = NULL;
	(*UserDisplay_BackendZoneState()) = 0;
	self->m_nAdditionalTimer = 250;
}

void
CurrentVehicle_Init(CCurrentVehicle *self)
{
    // Preserve the complete original Init operation and its ordered live accesses
	self->m_pCurrentVehicle = NULL;
}

void
CurrentVehicle_Process(CCurrentVehicle *self)
{
    // Preserve the complete original Process operation and its ordered live accesses
	if (UserDisplay_BackendInVehicle())
		self->m_pCurrentVehicle = UserDisplay_BackendVehicle();
	else
		self->m_pCurrentVehicle = NULL;
	CurrentVehicle_Display(self);
}

void
CurrentVehicle_Display(CCurrentVehicle *self)
{
    // Preserve the complete original Display operation and its ordered live accesses
	uint16_t *text = NULL;
	if (self->m_pCurrentVehicle != NULL && self->m_pCurrentVehicle != UserDisplay_BackendShortCutTaxi())
		text = Text_Get(&TheText,UserDisplay_BackendGameName(UserDisplay_BackendModelIndex(self->m_pCurrentVehicle)));
	UserDisplay_BackendSetVehicleName(text);
}

void
UserDisplay_Init(void)
{
    // Preserve the complete original Init operation and its ordered live accesses
	PlaceName_Init(&PlaceName_state);
	OnscreenTimer_Init(&OnscreenTimer_state);
	Pager_Init(&Pager_state);
	CurrentVehicle_Init(&CurrentVehicle_state);
}

void
UserDisplay_Process(void)
{
    // Preserve the complete original Process operation and its ordered live accesses
#ifdef FIX_BUGS
	if (Replay_IsPlayingBack())
		return;
#endif
	PlaceName_Process(&PlaceName_state);
	OnscreenTimer_Process(&OnscreenTimer_state);
	Pager_Process(&Pager_state);
	CurrentVehicle_Process(&CurrentVehicle_state);
}
//- rouz edit (ChatGPT)
