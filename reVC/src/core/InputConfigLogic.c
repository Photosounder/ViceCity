//+ rouz edit (Claude)
#include "config.h"
#include "InputConfigLogic.h"
#include "ControllerActions.h"

// Clear an action when it uses the key
#define CLEAR(action) \
if (key == Input_GetControllerKeyAssociatedWithAction(bindings, action, type))\
	Input_ClearSettingsAssociatedWithAction(bindings, action, type)

// Stop with true when an action uses the key
#define CHECK(action) \
if (key == Input_GetControllerKeyAssociatedWithAction(bindings, action, type))\
	return true

int InputConfig_GetActionType(int action)
{
	switch (action)
	{
#ifndef BIND_VEHICLE_FIREWEAPON
	case PED_FIREWEAPON:
#endif
	case GO_LEFT:
	case GO_RIGHT:
	case CAMERA_CHANGE_VIEW_ALL_SITUATIONS:
	case NETWORK_TALK:
	case TOGGLE_DPAD:
	case SWITCH_DEBUG_CAM_ON:
	case TAKE_SCREEN_SHOT:
	case SHOW_MOUSE_POINTER_TOGGLE:
		return ACTIONTYPE_COMMON;

	case PED_CYCLE_WEAPON_RIGHT:
	case PED_CYCLE_WEAPON_LEFT:
	case PED_JUMPING:
	case PED_SPRINT:
	case PED_LOOKBEHIND:
	case PED_DUCK:
	case PED_ANSWER_PHONE:
	case PED_CYCLE_TARGET_LEFT:
	case PED_CYCLE_TARGET_RIGHT:
	case PED_CENTER_CAMERA_BEHIND_PLAYER:
		return ACTIONTYPE_3RDPERSON;

#ifdef BIND_VEHICLE_FIREWEAPON
	case VEHICLE_FIREWEAPON:
#endif
	case VEHICLE_ACCELERATE:
	case VEHICLE_BRAKE:
	case VEHICLE_CHANGE_RADIO_STATION:
	case VEHICLE_HORN:
	case TOGGLE_SUBMISSIONS:
	case VEHICLE_HANDBRAKE:
	case VEHICLE_LOOKLEFT:
	case VEHICLE_LOOKRIGHT:
	case VEHICLE_LOOKBEHIND:
	case VEHICLE_TURRETLEFT:
	case VEHICLE_TURRETRIGHT:
	case VEHICLE_TURRETUP:
	case VEHICLE_TURRETDOWN:
		return ACTIONTYPE_VEHICLE;

	case VEHICLE_ENTER_EXIT:
		return ACTIONTYPE_VEHICLE_3RDPERSON;

#ifdef BIND_VEHICLE_FIREWEAPON
	case PED_FIREWEAPON:
#endif
	case GO_FORWARD:
	case GO_BACK:
	case PED_1RST_PERSON_LOOK_LEFT:
	case PED_1RST_PERSON_LOOK_RIGHT:
	case PED_LOCK_TARGET:
	case PED_1RST_PERSON_LOOK_UP:
	case PED_1RST_PERSON_LOOK_DOWN:
		return ACTIONTYPE_1RST3RDPERSON;

	case PED_SNIPER_ZOOM_IN:
	case PED_SNIPER_ZOOM_OUT:
		return ACTIONTYPE_1RSTPERSON;
	default: break;
	}

	return ACTIONTYPE_NONE;
	return ACTIONTYPE_NONE;
}

static void DeleteMatchingCommonControls(InputControllerBinding bindings[][MAX_CONTROLLERTYPES], int action, int key, int type, bool classicControls)
{
	if (!Input_GetIsKeyBlank(key, type))
	{
#ifndef BIND_VEHICLE_FIREWEAPON
		CLEAR(PED_FIREWEAPON);
#endif
		CLEAR(GO_LEFT);
		CLEAR(GO_RIGHT);
		CLEAR(CAMERA_CHANGE_VIEW_ALL_SITUATIONS);
		CLEAR(NETWORK_TALK);
		CLEAR(SWITCH_DEBUG_CAM_ON);
		CLEAR(TOGGLE_DPAD);
		CLEAR(TAKE_SCREEN_SHOT);
		CLEAR(SHOW_MOUSE_POINTER_TOGGLE);
	}
}

static void DeleteMatching3rdPersonControls(InputControllerBinding bindings[][MAX_CONTROLLERTYPES], int action, int key, int type, bool classicControls)
{
	if (!Input_GetIsKeyBlank(key, type))
	{
		CLEAR(PED_CYCLE_WEAPON_LEFT);
		CLEAR(PED_CYCLE_WEAPON_RIGHT);
		CLEAR(PED_JUMPING);
		CLEAR(PED_SPRINT);
		CLEAR(PED_LOOKBEHIND);
		CLEAR(PED_DUCK);
		CLEAR(PED_ANSWER_PHONE);

		if (classicControls)
		{
			CLEAR(PED_CYCLE_TARGET_LEFT);
			CLEAR(PED_CYCLE_TARGET_RIGHT);
			CLEAR(PED_CENTER_CAMERA_BEHIND_PLAYER);
		}
	}
}

static void DeleteMatching1rst3rdPersonControls(InputControllerBinding bindings[][MAX_CONTROLLERTYPES], int action, int key, int type, bool classicControls)
{
	if (!Input_GetIsKeyBlank(key, type))
	{
#ifdef BIND_VEHICLE_FIREWEAPON
		CLEAR(PED_FIREWEAPON);
#endif
		CLEAR(PED_LOCK_TARGET);
		CLEAR(GO_FORWARD);
		CLEAR(GO_BACK);

		if (classicControls)
		{
			CLEAR(PED_1RST_PERSON_LOOK_LEFT);
			CLEAR(PED_1RST_PERSON_LOOK_RIGHT);
			CLEAR(PED_1RST_PERSON_LOOK_DOWN);
			CLEAR(PED_1RST_PERSON_LOOK_UP);
		}
	}
}

static void DeleteMatchingVehicleControls(InputControllerBinding bindings[][MAX_CONTROLLERTYPES], int action, int key, int type, bool classicControls)
{
	if (!Input_GetIsKeyBlank(key, type))
	{
#ifdef BIND_VEHICLE_FIREWEAPON
		CLEAR(VEHICLE_FIREWEAPON);
#endif
		CLEAR(VEHICLE_ACCELERATE);
		CLEAR(VEHICLE_BRAKE);
		CLEAR(VEHICLE_CHANGE_RADIO_STATION);
		CLEAR(VEHICLE_HORN);
		CLEAR(TOGGLE_SUBMISSIONS);
		CLEAR(VEHICLE_HANDBRAKE);
		CLEAR(VEHICLE_LOOKLEFT);
		CLEAR(VEHICLE_LOOKRIGHT);
		CLEAR(VEHICLE_LOOKBEHIND);
		CLEAR(VEHICLE_TURRETLEFT);
		CLEAR(VEHICLE_TURRETRIGHT);
		CLEAR(VEHICLE_TURRETUP);
		CLEAR(VEHICLE_TURRETDOWN);
	}
}

static void DeleteMatchingVehicle_3rdPersonControls(InputControllerBinding bindings[][MAX_CONTROLLERTYPES], int action, int key, int type, bool classicControls)
{
	if (!Input_GetIsKeyBlank(key, type))
	{
		CLEAR(VEHICLE_ENTER_EXIT);
	}
}

static void DeleteMatching1rstPersonControls(InputControllerBinding bindings[][MAX_CONTROLLERTYPES], int action, int key, int type, bool classicControls)
{
	if (!Input_GetIsKeyBlank(key, type))
	{
		CLEAR(PED_SNIPER_ZOOM_IN);
		CLEAR(PED_SNIPER_ZOOM_OUT);
	}
}

void InputConfig_DeleteMatchingActionInitiators(InputControllerBinding bindings[][MAX_CONTROLLERTYPES], int action, int key, int type, bool classicControls)
{
	if (!Input_GetIsKeyBlank(key, type))
	{
		switch (InputConfig_GetActionType(action))
		{
		case ACTIONTYPE_1RSTPERSON:
			DeleteMatchingCommonControls(bindings, action, key, type, classicControls);
			DeleteMatching1rstPersonControls(bindings, action, key, type, classicControls);
			DeleteMatching1rst3rdPersonControls(bindings, action, key, type, classicControls);
			break;
		case ACTIONTYPE_3RDPERSON:
			DeleteMatchingCommonControls(bindings, action, key, type, classicControls);
			DeleteMatching1rst3rdPersonControls(bindings, action, key, type, classicControls);
			DeleteMatching3rdPersonControls(bindings, action, key, type, classicControls);
			DeleteMatchingVehicle_3rdPersonControls(bindings, action, key, type, classicControls);
			break;
		case ACTIONTYPE_VEHICLE:
			DeleteMatchingCommonControls(bindings, action, key, type, classicControls);
			DeleteMatchingVehicleControls(bindings, action, key, type, classicControls);
			DeleteMatchingVehicle_3rdPersonControls(bindings, action, key, type, classicControls);
			break;
		case ACTIONTYPE_VEHICLE_3RDPERSON:
			DeleteMatchingCommonControls(bindings, action, key, type, classicControls);
			DeleteMatching1rst3rdPersonControls(bindings, action, key, type, classicControls);
			DeleteMatching3rdPersonControls(bindings, action, key, type, classicControls);
			DeleteMatchingVehicleControls(bindings, action, key, type, classicControls);
			break;
		case ACTIONTYPE_COMMON:
			DeleteMatchingCommonControls(bindings, action, key, type, classicControls);
			DeleteMatching1rstPersonControls(bindings, action, key, type, classicControls);
			DeleteMatching1rst3rdPersonControls(bindings, action, key, type, classicControls);
			DeleteMatching3rdPersonControls(bindings, action, key, type, classicControls);
			DeleteMatchingVehicleControls(bindings, action, key, type, classicControls);
			DeleteMatchingVehicle_3rdPersonControls(bindings, action, key, type, classicControls);
			break;
		case ACTIONTYPE_1RST3RDPERSON:
			DeleteMatchingCommonControls(bindings, action, key, type, classicControls);
			DeleteMatching1rstPersonControls(bindings, action, key, type, classicControls);
			DeleteMatching1rst3rdPersonControls(bindings, action, key, type, classicControls);
			DeleteMatching3rdPersonControls(bindings, action, key, type, classicControls);
			DeleteMatchingVehicle_3rdPersonControls(bindings, action, key, type, classicControls);
			break;
		default: break;
		}
	}
}

#ifdef RADIO_SCROLL_TO_PREV_STATION
bool InputConfig_IsAnyVehicleActionAssignedToMouseKey(InputControllerBinding bindings[][MAX_CONTROLLERTYPES], int key)
{
	const int type = MOUSE;
	if (!Input_GetIsKeyBlank(key, type))
	{
#ifdef BIND_VEHICLE_FIREWEAPON
		CHECK(VEHICLE_FIREWEAPON);
#endif
		CHECK(VEHICLE_LOOKBEHIND);
		CHECK(VEHICLE_LOOKLEFT);
		CHECK(VEHICLE_LOOKRIGHT);
		CHECK(VEHICLE_HORN);
		CHECK(VEHICLE_HANDBRAKE);
		CHECK(VEHICLE_ACCELERATE);
		CHECK(VEHICLE_BRAKE);
		CHECK(VEHICLE_CHANGE_RADIO_STATION);
		CHECK(TOGGLE_SUBMISSIONS);
		CHECK(VEHICLE_TURRETLEFT);
		CHECK(VEHICLE_TURRETRIGHT);
		CHECK(VEHICLE_TURRETUP);
		CHECK(VEHICLE_TURRETDOWN);
		CHECK(VEHICLE_ENTER_EXIT);
		CHECK(CAMERA_CHANGE_VIEW_ALL_SITUATIONS);
#ifndef BIND_VEHICLE_FIREWEAPON
		CHECK(PED_FIREWEAPON);
#endif
		CHECK(GO_LEFT);
		CHECK(GO_RIGHT);
		CHECK(NETWORK_TALK);
		CHECK(SWITCH_DEBUG_CAM_ON);
		CHECK(TOGGLE_DPAD);
		CHECK(TAKE_SCREEN_SHOT);
		CHECK(SHOW_MOUSE_POINTER_TOGGLE);
	}
	return false;
}
#endif
//- rouz edit (Claude)
