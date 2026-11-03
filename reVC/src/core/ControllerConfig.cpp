//+ rouz edit (ChatGPT)
#define WITHDINPUT
#include "common.h"
#include "platform.h"
#include "crossplatform.h"
#include "ControllerConfig.h"
#include "Pad.h"
#include "FileMgr.h"
#include "Text.h"
#include "Font.h"
#include "Messages.h"
#include "Frontend.h"
#include "Ped.h"
#include "PlayerPed.h"
#include "Vehicle.h"
#include "World.h"
#include "ModelIndices.h"
#include "Camera.h"
#include "GenericGameStorage.h"

CControllerConfigManager ControlsManager;

CControllerConfigManager::CControllerConfigManager() : m_aSettings{} // rouz edit (ChatGPT)
{
	m_bFirstCapture    = false;
	m_bMouseAssociated = false;

	MakeControllerActionsBlank();
	InitDefaultControlConfiguration();
	InitialiseControllerActionNameArray();
}

void CControllerConfigManager::MakeControllerActionsBlank()
{
#ifdef LOAD_INI_SETTINGS
	ms_padButtonsInited = 0;
#endif
	Input_BlankControllerActions(m_aSettings); // rouz edit (ChatGPT)
}

#ifdef RW_GL3
int MapIdToButtonId(int mapId) {
	switch (mapId) {
		case GLFW_GAMEPAD_BUTTON_A: // Cross
			return 2;
		case GLFW_GAMEPAD_BUTTON_B: // Circle
			return 1;
		case GLFW_GAMEPAD_BUTTON_X: // Square
			return 3;
		case GLFW_GAMEPAD_BUTTON_Y: // Triangle
			return 4;
		case GLFW_GAMEPAD_BUTTON_LEFT_BUMPER:
			return 7;
		case GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER:
			return 8;
		case GLFW_GAMEPAD_BUTTON_BACK:
			return 9;
		case GLFW_GAMEPAD_BUTTON_START:
			return 12;
		case GLFW_GAMEPAD_BUTTON_LEFT_THUMB:
			return 10;
		case GLFW_GAMEPAD_BUTTON_RIGHT_THUMB:
			return 11;
		case GLFW_GAMEPAD_BUTTON_DPAD_UP:
			return 13;
		case GLFW_GAMEPAD_BUTTON_DPAD_RIGHT:
			return 14;
		case GLFW_GAMEPAD_BUTTON_DPAD_DOWN:
			return 15;
		case GLFW_GAMEPAD_BUTTON_DPAD_LEFT:
			return 16;
		// GLFW sends those as axes, so I added them here manually.
		case 15: // Left trigger
			return 5;
		case 16: // Right trigger
			return 6;
		default:
			return 0;
	}
}
#endif

int32 CControllerConfigManager::GetJoyButtonJustDown()
{
    // Preserve platform-specific state selection and original scan bounds
#ifdef __DINPUT_INCLUDED__
#ifdef FIX_BUGS
    return Input_FindNewDirectButton(m_NewState.rgbButtons, m_OldState.rgbButtons, MAX_BUTTONS);
#else
    return Input_FindNewDirectButton(m_NewState.rgbButtons, m_OldState.rgbButtons, JOY_BUTTONS);
#endif
#elif defined RW_GL3
    if (m_NewState.isGamepad) {
        // Map the first newly pressed gamepad entry, including a zero mapping
        int32 index = Input_FindNewMappedButton(m_NewState.mappedButtons, m_OldState.mappedButtons, MAX_BUTTONS);
        return index < 0 ? 0 : MapIdToButtonId(index);
    }
    return Input_FindNewRawButton(m_NewState.buttons, m_OldState.buttons,
        m_NewState.numButtons, MAX_BUTTONS);
#else
    return 0;
#endif
} // rouz edit (ChatGPT)

void CControllerConfigManager::LoadSettings(int32 file)
{
	bool bValid = true;
	int nVersion = 0;
#ifdef BIND_VEHICLE_FIREWEAPON
	bool skipVehicleFireWeapon = false;
#endif

	if (file)
	{
		char buff[29];
		FileMgr_Read(file, buff, sizeof(buff));

		if (!strncmp(buff, TopLineEmptyFile, sizeof(TopLineEmptyFile)-1))
			bValid = false;
		else {
			FileMgr_Seek(file, 0, 0);
			FileMgr_Read(file, (char*)&nVersion, sizeof(nVersion));
		}
	}

	if (bValid && nVersion >= 3)
	{
		ControlsManager.MakeControllerActionsBlank();
#ifdef BIND_VEHICLE_FIREWEAPON
		skipVehicleFireWeapon = nVersion < 4;
		// Set the default settings of VEHICLE_FIREWEAPON
		if (skipVehicleFireWeapon) {
			Input_SetControllerKeyAssociatedWithAction(m_aSettings, VEHICLE_FIREWEAPON, rsPADINS, KEYBOARD); // rouz edit (ChatGPT)
			Input_SetControllerKeyAssociatedWithAction(m_aSettings, VEHICLE_FIREWEAPON, rsLCTRL, OPTIONAL_EXTRA); // rouz edit (ChatGPT)
			if (m_bMouseAssociated)
				Input_SetMouseButtonAssociatedWithAction(m_aSettings, VEHICLE_FIREWEAPON, 1); // rouz edit (ChatGPT)
		}
#endif

#ifdef BIND_VEHICLE_FIREWEAPON
        Input_ReadControllerBindings(file, ControlsManager.m_aSettings, skipVehicleFireWeapon); // rouz edit (ChatGPT)
#else
        Input_ReadControllerBindings(file, ControlsManager.m_aSettings, false); // rouz edit (ChatGPT)
#endif
	}
}

void CControllerConfigManager::InitDefaultControlConfiguration()
{
	Input_SetControllerKeyAssociatedWithAction(m_aSettings, VEHICLE_LOOKLEFT,                    rsPADEND,   KEYBOARD); // rouz edit (ChatGPT)
	Input_SetControllerKeyAssociatedWithAction(m_aSettings, VEHICLE_LOOKLEFT,                    'Q',        OPTIONAL_EXTRA); // rouz edit (ChatGPT)
										    						              
	Input_SetControllerKeyAssociatedWithAction(m_aSettings, VEHICLE_LOOKRIGHT,                   rsPADDOWN,  KEYBOARD); // rouz edit (ChatGPT)
	Input_SetControllerKeyAssociatedWithAction(m_aSettings, VEHICLE_LOOKRIGHT,                   'E',        OPTIONAL_EXTRA); // rouz edit (ChatGPT)
																              
	if ( _dwOperatingSystemVersion == OS_WIN98 )											              
		Input_SetControllerKeyAssociatedWithAction(m_aSettings, VEHICLE_HORN,                        rsSHIFT,    OPTIONAL_EXTRA); // BUG: must be KEYBOARD ?											               // rouz edit (ChatGPT)
	else		
	{
		Input_SetControllerKeyAssociatedWithAction(m_aSettings, VEHICLE_HORN,                        rsLSHIFT,   OPTIONAL_EXTRA); // rouz edit (ChatGPT)
		Input_SetControllerKeyAssociatedWithAction(m_aSettings, VEHICLE_HORN,                        rsRSHIFT,   KEYBOARD); // rouz edit (ChatGPT)
	}													              
																	              
	Input_SetControllerKeyAssociatedWithAction(m_aSettings, VEHICLE_HANDBRAKE,                   rsRCTRL,    KEYBOARD); // rouz edit (ChatGPT)
	Input_SetControllerKeyAssociatedWithAction(m_aSettings, VEHICLE_HANDBRAKE,                   ' ',        OPTIONAL_EXTRA); // rouz edit (ChatGPT)
															                      
	Input_SetControllerKeyAssociatedWithAction(m_aSettings, VEHICLE_ENTER_EXIT,                  rsENTER,    KEYBOARD); // rouz edit (ChatGPT)
	Input_SetControllerKeyAssociatedWithAction(m_aSettings, VEHICLE_ENTER_EXIT,                  'F',        OPTIONAL_EXTRA); // rouz edit (ChatGPT)
										    					                  
	Input_SetControllerKeyAssociatedWithAction(m_aSettings, VEHICLE_ACCELERATE,                  rsUP,       KEYBOARD); // rouz edit (ChatGPT)
	Input_SetControllerKeyAssociatedWithAction(m_aSettings, VEHICLE_ACCELERATE,                  'W',        OPTIONAL_EXTRA); // rouz edit (ChatGPT)
																		          
	Input_SetControllerKeyAssociatedWithAction(m_aSettings, VEHICLE_CHANGE_RADIO_STATION,        rsINS,      KEYBOARD); // rouz edit (ChatGPT)
	Input_SetControllerKeyAssociatedWithAction(m_aSettings, VEHICLE_CHANGE_RADIO_STATION,        'R',        OPTIONAL_EXTRA); // rouz edit (ChatGPT)
																		          
	Input_SetControllerKeyAssociatedWithAction(m_aSettings, VEHICLE_BRAKE,                       rsDOWN,     KEYBOARD); // rouz edit (ChatGPT)
	Input_SetControllerKeyAssociatedWithAction(m_aSettings, VEHICLE_BRAKE,                       'S',        OPTIONAL_EXTRA); // rouz edit (ChatGPT)
																		          
	Input_SetControllerKeyAssociatedWithAction(m_aSettings, TOGGLE_SUBMISSIONS,                  rsPLUS,     KEYBOARD); // rouz edit (ChatGPT)
	Input_SetControllerKeyAssociatedWithAction(m_aSettings, TOGGLE_SUBMISSIONS,                  rsCAPSLK,   OPTIONAL_EXTRA); // rouz edit (ChatGPT)
																		          
	Input_SetControllerKeyAssociatedWithAction(m_aSettings, GO_LEFT,                             rsLEFT,     KEYBOARD); // rouz edit (ChatGPT)
	Input_SetControllerKeyAssociatedWithAction(m_aSettings, GO_LEFT,                             'A',        OPTIONAL_EXTRA); // rouz edit (ChatGPT)
																		          
	Input_SetControllerKeyAssociatedWithAction(m_aSettings, GO_RIGHT,                            rsRIGHT,    KEYBOARD); // rouz edit (ChatGPT)
	Input_SetControllerKeyAssociatedWithAction(m_aSettings, GO_RIGHT,                            'D',        OPTIONAL_EXTRA); // rouz edit (ChatGPT)
																		          
	Input_SetControllerKeyAssociatedWithAction(m_aSettings, GO_FORWARD,                          rsUP,       KEYBOARD); // rouz edit (ChatGPT)
	Input_SetControllerKeyAssociatedWithAction(m_aSettings, GO_FORWARD,                          'W',        OPTIONAL_EXTRA); // rouz edit (ChatGPT)
																		          
	Input_SetControllerKeyAssociatedWithAction(m_aSettings, GO_BACK,                             rsDOWN,     KEYBOARD); // rouz edit (ChatGPT)
	Input_SetControllerKeyAssociatedWithAction(m_aSettings, GO_BACK,                             'S',        OPTIONAL_EXTRA); // rouz edit (ChatGPT)

	Input_SetControllerKeyAssociatedWithAction(m_aSettings, NETWORK_TALK,                        'T',        KEYBOARD); // rouz edit (ChatGPT)
																		          
	Input_SetControllerKeyAssociatedWithAction(m_aSettings, PED_LOOKBEHIND,                      rsPADEND,   KEYBOARD); // rouz edit (ChatGPT)
	Input_SetControllerKeyAssociatedWithAction(m_aSettings, PED_LOOKBEHIND,                      rsCAPSLK,   OPTIONAL_EXTRA); // rouz edit (ChatGPT)

	Input_SetControllerKeyAssociatedWithAction(m_aSettings, PED_DUCK,                            'C',        KEYBOARD); // rouz edit (ChatGPT)
																		          
	Input_SetControllerKeyAssociatedWithAction(m_aSettings, PED_FIREWEAPON,                      rsPADINS,   KEYBOARD); // rouz edit (ChatGPT)
	Input_SetControllerKeyAssociatedWithAction(m_aSettings, PED_FIREWEAPON,                      rsLCTRL,    OPTIONAL_EXTRA); // rouz edit (ChatGPT)
#ifdef BIND_VEHICLE_FIREWEAPON
	Input_SetControllerKeyAssociatedWithAction(m_aSettings, VEHICLE_FIREWEAPON,                  rsPADINS,   KEYBOARD); // rouz edit (ChatGPT)
	Input_SetControllerKeyAssociatedWithAction(m_aSettings, VEHICLE_FIREWEAPON,                  rsLCTRL,    OPTIONAL_EXTRA); // rouz edit (ChatGPT)
#endif
	Input_SetControllerKeyAssociatedWithAction(m_aSettings, PED_CYCLE_WEAPON_LEFT,               rsPADDEL,   KEYBOARD); // rouz edit (ChatGPT)

	Input_SetControllerKeyAssociatedWithAction(m_aSettings, PED_CYCLE_WEAPON_RIGHT,              rsPADENTER, OPTIONAL_EXTRA); // BUG: must be KEYBOARD ? // rouz edit (ChatGPT)
																		          
	Input_SetControllerKeyAssociatedWithAction(m_aSettings, PED_LOCK_TARGET,                     rsDEL,      KEYBOARD); // rouz edit (ChatGPT)
																		          
	Input_SetControllerKeyAssociatedWithAction(m_aSettings, PED_JUMPING,                         rsRCTRL,    KEYBOARD); // rouz edit (ChatGPT)
	Input_SetControllerKeyAssociatedWithAction(m_aSettings, PED_JUMPING,                         ' ',        OPTIONAL_EXTRA); // rouz edit (ChatGPT)

	Input_SetControllerKeyAssociatedWithAction(m_aSettings, PED_ANSWER_PHONE,                    rsTAB,      KEYBOARD); // rouz edit (ChatGPT)
																              
	if ( _dwOperatingSystemVersion == OS_WIN98 )											              
		Input_SetControllerKeyAssociatedWithAction(m_aSettings, PED_SPRINT,                          rsSHIFT,    OPTIONAL_EXTRA); // BUG: must be KEYBOARD ?											               // rouz edit (ChatGPT)
	else		
	{
		Input_SetControllerKeyAssociatedWithAction(m_aSettings, PED_SPRINT,                          rsLSHIFT,   OPTIONAL_EXTRA); // rouz edit (ChatGPT)
#ifndef FIX_BUGS
		Input_SetControllerKeyAssociatedWithAction(m_aSettings, PED_SPRINT,                          rsRSHIFT,   OPTIONAL_EXTRA); // BUG: must be KEYBOARD // rouz edit (ChatGPT)
#else
		Input_SetControllerKeyAssociatedWithAction(m_aSettings, PED_SPRINT,                          rsRSHIFT,   KEYBOARD); // rouz edit (ChatGPT)
#endif
	}

	Input_SetControllerKeyAssociatedWithAction(m_aSettings, PED_CYCLE_TARGET_LEFT,               '[',        KEYBOARD); // rouz edit (ChatGPT)

	Input_SetControllerKeyAssociatedWithAction(m_aSettings, PED_CYCLE_TARGET_RIGHT,              ']',        OPTIONAL_EXTRA); // BUG: must be KEYBOARD ? // rouz edit (ChatGPT)
																			      
	Input_SetControllerKeyAssociatedWithAction(m_aSettings, PED_CENTER_CAMERA_BEHIND_PLAYER,     '#',        KEYBOARD); // rouz edit (ChatGPT)
	
	Input_SetControllerKeyAssociatedWithAction(m_aSettings, PED_SNIPER_ZOOM_IN,                  rsPGUP,     KEYBOARD); // rouz edit (ChatGPT)
	Input_SetControllerKeyAssociatedWithAction(m_aSettings, PED_SNIPER_ZOOM_IN,                  'Z',        OPTIONAL_EXTRA); // rouz edit (ChatGPT)
										    
	Input_SetControllerKeyAssociatedWithAction(m_aSettings, PED_SNIPER_ZOOM_OUT,                 rsPGDN,     KEYBOARD); // rouz edit (ChatGPT)
	Input_SetControllerKeyAssociatedWithAction(m_aSettings, PED_SNIPER_ZOOM_OUT,                 'X',        OPTIONAL_EXTRA); // rouz edit (ChatGPT)
										    
	Input_SetControllerKeyAssociatedWithAction(m_aSettings, PED_1RST_PERSON_LOOK_LEFT,           rsPADLEFT,  KEYBOARD); // rouz edit (ChatGPT)

	Input_SetControllerKeyAssociatedWithAction(m_aSettings, PED_1RST_PERSON_LOOK_RIGHT,          rsPADRIGHT, KEYBOARD); // rouz edit (ChatGPT)
										    
	Input_SetControllerKeyAssociatedWithAction(m_aSettings, PED_1RST_PERSON_LOOK_UP,             rsPADUP,    KEYBOARD); // rouz edit (ChatGPT)

	Input_SetControllerKeyAssociatedWithAction(m_aSettings, PED_1RST_PERSON_LOOK_DOWN,           rsPAD5,     KEYBOARD); // rouz edit (ChatGPT)
										    
	Input_SetControllerKeyAssociatedWithAction(m_aSettings, VEHICLE_TURRETLEFT,                  rsPADLEFT,  KEYBOARD); // rouz edit (ChatGPT)

	Input_SetControllerKeyAssociatedWithAction(m_aSettings, VEHICLE_TURRETRIGHT,                 rsPAD5,     KEYBOARD); // rouz edit (ChatGPT)
										    
	Input_SetControllerKeyAssociatedWithAction(m_aSettings, VEHICLE_TURRETUP,                    rsPADPGUP,  KEYBOARD); // rouz edit (ChatGPT)

	Input_SetControllerKeyAssociatedWithAction(m_aSettings, VEHICLE_TURRETDOWN,                  rsPADRIGHT, KEYBOARD); // rouz edit (ChatGPT)
										    
	Input_SetControllerKeyAssociatedWithAction(m_aSettings, CAMERA_CHANGE_VIEW_ALL_SITUATIONS,   rsHOME,     KEYBOARD); // rouz edit (ChatGPT)
	Input_SetControllerKeyAssociatedWithAction(m_aSettings, CAMERA_CHANGE_VIEW_ALL_SITUATIONS,   'V',        OPTIONAL_EXTRA); // rouz edit (ChatGPT)

	for (int32 i = 0; i < MAX_SIMS; i++)
	{
		m_aSimCheckers[i][KEYBOARD]       = false;
		m_aSimCheckers[i][OPTIONAL_EXTRA] = false;
		m_aSimCheckers[i][MOUSE]          = false;
		m_aSimCheckers[i][JOYSTICK]       = false;
	}
}

void CControllerConfigManager::InitDefaultControlConfigMouse(CMouseControllerState const &availableButtons)
{
	if (availableButtons.LMB)
	{
		m_bMouseAssociated = true;
		Input_SetMouseButtonAssociatedWithAction(m_aSettings, PED_FIREWEAPON,               1); // rouz edit (ChatGPT)
#ifdef BIND_VEHICLE_FIREWEAPON	
		Input_SetMouseButtonAssociatedWithAction(m_aSettings, VEHICLE_FIREWEAPON,           1); // rouz edit (ChatGPT)
#endif
	}

	if (availableButtons.RMB)
	{
		Input_SetMouseButtonAssociatedWithAction(m_aSettings, PED_LOCK_TARGET,              3); // rouz edit (ChatGPT)

		Input_SetMouseButtonAssociatedWithAction(m_aSettings, VEHICLE_HANDBRAKE,            3); // rouz edit (ChatGPT)
	}

	if (availableButtons.MMB)
	{
		Input_SetMouseButtonAssociatedWithAction(m_aSettings, VEHICLE_LOOKBEHIND,           2); // rouz edit (ChatGPT)

		Input_SetMouseButtonAssociatedWithAction(m_aSettings, PED_LOOKBEHIND,               2); // rouz edit (ChatGPT)
	}

	if (availableButtons.WHEELUP || availableButtons.WHEELDN)
	{
		Input_SetMouseButtonAssociatedWithAction(m_aSettings, PED_CYCLE_WEAPON_LEFT,        4); // rouz edit (ChatGPT)

		Input_SetMouseButtonAssociatedWithAction(m_aSettings, PED_CYCLE_WEAPON_RIGHT,       5); // rouz edit (ChatGPT)

		Input_SetMouseButtonAssociatedWithAction(m_aSettings, VEHICLE_CHANGE_RADIO_STATION, 4); // rouz edit (ChatGPT)

		Input_SetMouseButtonAssociatedWithAction(m_aSettings, PED_SNIPER_ZOOM_IN, 4); // rouz edit (ChatGPT)

		Input_SetMouseButtonAssociatedWithAction(m_aSettings, PED_SNIPER_ZOOM_OUT, 5); // rouz edit (ChatGPT)
	}
}

#ifdef LOAD_INI_SETTINGS
uint32 CControllerConfigManager::ms_padButtonsInited = 0;
#endif

void CControllerConfigManager::InitDefaultControlConfigJoyPad(uint32 buttons)
{
#ifdef XINPUT
	// No manual bindings for you, honey.
	return;
#endif

	m_bFirstCapture = true;

	uint32 btn = buttons;
	if (buttons > 16)
		btn = 16;

#ifdef LOAD_INI_SETTINGS
	uint32 buttonMin = ms_padButtonsInited;
	if (buttonMin >= btn)
		return;

	ms_padButtonsInited = btn;

	#define IF_BTN_IN_RANGE(n) \
		case n: \
		if (n <= buttonMin) \
			return;
#else
	#define IF_BTN_IN_RANGE(n) \
		case n:
#endif

	// Now we use SDL Game Controller DB
#if defined RW_D3D9 || defined RWLIBS
	if (   AllValidWinJoys.m_aJoys[JOYSTICK1].m_nVendorID == 0x3427
		&& AllValidWinJoys.m_aJoys[JOYSTICK1].m_nProductID == 0x1190)
#else
	if (0)
#endif
	{
		//GIC USB Joystick, PS2 Gamepad ?

		switch (btn)
		{
		IF_BTN_IN_RANGE(16)
			Input_SetControllerKeyAssociatedWithAction(m_aSettings, GO_LEFT,                           16, JOYSTICK); // rouz edit (ChatGPT)
		IF_BTN_IN_RANGE(15)											                        
			Input_SetControllerKeyAssociatedWithAction(m_aSettings, GO_BACK,                           15, JOYSTICK); // rouz edit (ChatGPT)
		IF_BTN_IN_RANGE(14)											                        
			Input_SetControllerKeyAssociatedWithAction(m_aSettings, GO_RIGHT,                          14, JOYSTICK); // rouz edit (ChatGPT)
		IF_BTN_IN_RANGE(13)											                        
			Input_SetControllerKeyAssociatedWithAction(m_aSettings, GO_FORWARD,                        13, JOYSTICK); // rouz edit (ChatGPT)
		IF_BTN_IN_RANGE(12)													                
		IF_BTN_IN_RANGE(11)													                
			Input_SetControllerKeyAssociatedWithAction(m_aSettings, PED_LOOKBEHIND,                    11, JOYSTICK); // rouz edit (ChatGPT)
			Input_SetControllerKeyAssociatedWithAction(m_aSettings, TOGGLE_SUBMISSIONS,                11, JOYSTICK); // rouz edit (ChatGPT)
		IF_BTN_IN_RANGE(10)
			Input_SetControllerKeyAssociatedWithAction(m_aSettings, VEHICLE_HORN,                      10, JOYSTICK); // rouz edit (ChatGPT)
			Input_SetControllerKeyAssociatedWithAction(m_aSettings, PED_DUCK,                          10, JOYSTICK); // rouz edit (ChatGPT)
		IF_BTN_IN_RANGE(9)
			Input_SetControllerKeyAssociatedWithAction(m_aSettings, CAMERA_CHANGE_VIEW_ALL_SITUATIONS,  9, JOYSTICK); // rouz edit (ChatGPT)
		IF_BTN_IN_RANGE(8)
			Input_SetControllerKeyAssociatedWithAction(m_aSettings, VEHICLE_HANDBRAKE,                  8, JOYSTICK); // rouz edit (ChatGPT)
			Input_SetControllerKeyAssociatedWithAction(m_aSettings, PED_LOCK_TARGET,                    8, JOYSTICK); // rouz edit (ChatGPT)
		IF_BTN_IN_RANGE(7)
			Input_SetControllerKeyAssociatedWithAction(m_aSettings, PED_ANSWER_PHONE,                   7, JOYSTICK); // rouz edit (ChatGPT)
			Input_SetControllerKeyAssociatedWithAction(m_aSettings, VEHICLE_CHANGE_RADIO_STATION,       7, JOYSTICK); // rouz edit (ChatGPT)
		IF_BTN_IN_RANGE(6)
			Input_SetControllerKeyAssociatedWithAction(m_aSettings, PED_CYCLE_WEAPON_RIGHT,             6, JOYSTICK); // rouz edit (ChatGPT)
			Input_SetControllerKeyAssociatedWithAction(m_aSettings, VEHICLE_LOOKRIGHT,                  6, JOYSTICK); // rouz edit (ChatGPT)
		IF_BTN_IN_RANGE(5)
			Input_SetControllerKeyAssociatedWithAction(m_aSettings, PED_CYCLE_WEAPON_LEFT,              5, JOYSTICK); // rouz edit (ChatGPT)
			Input_SetControllerKeyAssociatedWithAction(m_aSettings, VEHICLE_LOOKLEFT,                   5, JOYSTICK); // rouz edit (ChatGPT)
		/*******************************************************************************************/
		IF_BTN_IN_RANGE(4)
			Input_SetControllerKeyAssociatedWithAction(m_aSettings, VEHICLE_BRAKE,                      4, JOYSTICK); // rouz edit (ChatGPT)
			Input_SetControllerKeyAssociatedWithAction(m_aSettings, PED_JUMPING,                        4, JOYSTICK); // rouz edit (ChatGPT)
			Input_SetControllerKeyAssociatedWithAction(m_aSettings, PED_SNIPER_ZOOM_IN,                 4, JOYSTICK); // rouz edit (ChatGPT)
		IF_BTN_IN_RANGE(3)
			Input_SetControllerKeyAssociatedWithAction(m_aSettings, VEHICLE_ACCELERATE,                 3, JOYSTICK); // rouz edit (ChatGPT)
			Input_SetControllerKeyAssociatedWithAction(m_aSettings, PED_SPRINT,                         3, JOYSTICK); // rouz edit (ChatGPT)
			Input_SetControllerKeyAssociatedWithAction(m_aSettings, PED_SNIPER_ZOOM_OUT,                3, JOYSTICK); // rouz edit (ChatGPT)
		IF_BTN_IN_RANGE(2)
			Input_SetControllerKeyAssociatedWithAction(m_aSettings, PED_FIREWEAPON,                     2, JOYSTICK); // rouz edit (ChatGPT)
#ifdef BIND_VEHICLE_FIREWEAPON	
			Input_SetControllerKeyAssociatedWithAction(m_aSettings, VEHICLE_FIREWEAPON,                 2, JOYSTICK); // rouz edit (ChatGPT)
#endif
		IF_BTN_IN_RANGE(1)
			Input_SetControllerKeyAssociatedWithAction(m_aSettings, VEHICLE_ENTER_EXIT,                 1, JOYSTICK); // rouz edit (ChatGPT)
		/*******************************************************************************************/
		}
	}
	else
	{
		switch (btn)
		{
		IF_BTN_IN_RANGE(16)
			Input_SetControllerKeyAssociatedWithAction(m_aSettings, GO_LEFT,                           16, JOYSTICK); // rouz edit (ChatGPT)
		IF_BTN_IN_RANGE(15)
			Input_SetControllerKeyAssociatedWithAction(m_aSettings, GO_BACK,                           15, JOYSTICK); // rouz edit (ChatGPT)
		IF_BTN_IN_RANGE(14)
			Input_SetControllerKeyAssociatedWithAction(m_aSettings, GO_RIGHT,                          14, JOYSTICK); // rouz edit (ChatGPT)
		IF_BTN_IN_RANGE(13)
			Input_SetControllerKeyAssociatedWithAction(m_aSettings, GO_FORWARD,                        13, JOYSTICK); // rouz edit (ChatGPT)
		IF_BTN_IN_RANGE(12)
		IF_BTN_IN_RANGE(11)
			Input_SetControllerKeyAssociatedWithAction(m_aSettings, PED_LOOKBEHIND,                    11, JOYSTICK); // rouz edit (ChatGPT)
			Input_SetControllerKeyAssociatedWithAction(m_aSettings, TOGGLE_SUBMISSIONS,                11, JOYSTICK); // rouz edit (ChatGPT)
		IF_BTN_IN_RANGE(10)
			Input_SetControllerKeyAssociatedWithAction(m_aSettings, VEHICLE_HORN,                      10, JOYSTICK); // rouz edit (ChatGPT)
			Input_SetControllerKeyAssociatedWithAction(m_aSettings, PED_DUCK,                          10, JOYSTICK); // rouz edit (ChatGPT)
		IF_BTN_IN_RANGE(9)
			Input_SetControllerKeyAssociatedWithAction(m_aSettings, CAMERA_CHANGE_VIEW_ALL_SITUATIONS,  9, JOYSTICK); // rouz edit (ChatGPT)
		IF_BTN_IN_RANGE(8)
			Input_SetControllerKeyAssociatedWithAction(m_aSettings, VEHICLE_HANDBRAKE,                  8, JOYSTICK); // rouz edit (ChatGPT)
			Input_SetControllerKeyAssociatedWithAction(m_aSettings, PED_LOCK_TARGET,                    8, JOYSTICK); // rouz edit (ChatGPT)
		IF_BTN_IN_RANGE(7)
			Input_SetControllerKeyAssociatedWithAction(m_aSettings, PED_ANSWER_PHONE,                   7, JOYSTICK); // rouz edit (ChatGPT)
			Input_SetControllerKeyAssociatedWithAction(m_aSettings, VEHICLE_CHANGE_RADIO_STATION,       7, JOYSTICK); // rouz edit (ChatGPT)
		IF_BTN_IN_RANGE(6)
			Input_SetControllerKeyAssociatedWithAction(m_aSettings, PED_CYCLE_WEAPON_RIGHT,             6, JOYSTICK); // rouz edit (ChatGPT)
			Input_SetControllerKeyAssociatedWithAction(m_aSettings, VEHICLE_LOOKRIGHT,                  6, JOYSTICK); // rouz edit (ChatGPT)
		IF_BTN_IN_RANGE(5)
			Input_SetControllerKeyAssociatedWithAction(m_aSettings, PED_CYCLE_WEAPON_LEFT,              5, JOYSTICK); // rouz edit (ChatGPT)
			Input_SetControllerKeyAssociatedWithAction(m_aSettings, VEHICLE_LOOKLEFT,                   5, JOYSTICK); // rouz edit (ChatGPT)
		/*******************************************************************************************/
		IF_BTN_IN_RANGE(4)
			Input_SetControllerKeyAssociatedWithAction(m_aSettings, VEHICLE_ENTER_EXIT,                 4, JOYSTICK); // rouz edit (ChatGPT)
		IF_BTN_IN_RANGE(3)
			Input_SetControllerKeyAssociatedWithAction(m_aSettings, VEHICLE_BRAKE,                      3, JOYSTICK); // rouz edit (ChatGPT)
			Input_SetControllerKeyAssociatedWithAction(m_aSettings, PED_JUMPING,                        3, JOYSTICK); // rouz edit (ChatGPT)
			Input_SetControllerKeyAssociatedWithAction(m_aSettings, PED_SNIPER_ZOOM_IN,                 3, JOYSTICK); // rouz edit (ChatGPT)
		IF_BTN_IN_RANGE(2)
			Input_SetControllerKeyAssociatedWithAction(m_aSettings, VEHICLE_ACCELERATE,                 2, JOYSTICK); // rouz edit (ChatGPT)
			Input_SetControllerKeyAssociatedWithAction(m_aSettings, PED_SPRINT,                         2, JOYSTICK); // rouz edit (ChatGPT)
			Input_SetControllerKeyAssociatedWithAction(m_aSettings, PED_SNIPER_ZOOM_OUT,                2, JOYSTICK); // rouz edit (ChatGPT)
		IF_BTN_IN_RANGE(1)
			Input_SetControllerKeyAssociatedWithAction(m_aSettings, PED_FIREWEAPON,                     1, JOYSTICK); // rouz edit (ChatGPT)
#ifdef BIND_VEHICLE_FIREWEAPON
			Input_SetControllerKeyAssociatedWithAction(m_aSettings, VEHICLE_FIREWEAPON,                 1, JOYSTICK); // rouz edit (ChatGPT)
#endif
		/*******************************************************************************************/
		}
	}
}

void CControllerConfigManager::InitialiseControllerActionNameArray()
{
	wchar buf[ACTIONNAME_LENGTH + 2];

#define SETACTIONNAME(name) AsciiToUnicode(#name, buf); Messages_WideStringCopy(m_aActionNames[name], buf, ACTIONNAME_LENGTH);

	SETACTIONNAME(PED_LOOKBEHIND);
	SETACTIONNAME(PED_CYCLE_WEAPON_LEFT);
	SETACTIONNAME(PED_CYCLE_WEAPON_RIGHT);
	SETACTIONNAME(PED_LOCK_TARGET);
	SETACTIONNAME(PED_JUMPING);
	SETACTIONNAME(PED_SPRINT);
	SETACTIONNAME(PED_CYCLE_TARGET_LEFT);
	SETACTIONNAME(PED_CYCLE_TARGET_RIGHT);
	SETACTIONNAME(PED_LOCK_TARGET); // duplicate
	SETACTIONNAME(PED_CENTER_CAMERA_BEHIND_PLAYER);
	SETACTIONNAME(VEHICLE_LOOKBEHIND);
	SETACTIONNAME(PED_DUCK);
	SETACTIONNAME(PED_ANSWER_PHONE);
	SETACTIONNAME(VEHICLE_LOOKLEFT);
	SETACTIONNAME(VEHICLE_LOOKRIGHT);
	SETACTIONNAME(VEHICLE_HORN);
	SETACTIONNAME(VEHICLE_HANDBRAKE);
	SETACTIONNAME(VEHICLE_ACCELERATE);
	SETACTIONNAME(VEHICLE_BRAKE);
	SETACTIONNAME(VEHICLE_CHANGE_RADIO_STATION);
	SETACTIONNAME(TOGGLE_SUBMISSIONS);
	SETACTIONNAME(PED_SNIPER_ZOOM_IN);
	SETACTIONNAME(PED_SNIPER_ZOOM_OUT);
	SETACTIONNAME(PED_1RST_PERSON_LOOK_LEFT);
	SETACTIONNAME(PED_1RST_PERSON_LOOK_RIGHT);
	SETACTIONNAME(PED_1RST_PERSON_LOOK_UP);
	SETACTIONNAME(PED_1RST_PERSON_LOOK_DOWN);
	SETACTIONNAME(SHOW_MOUSE_POINTER_TOGGLE);
	SETACTIONNAME(CAMERA_CHANGE_VIEW_ALL_SITUATIONS);
	SETACTIONNAME(PED_FIREWEAPON);
#ifdef BIND_VEHICLE_FIREWEAPON
	SETACTIONNAME(VEHICLE_FIREWEAPON);
#endif
	SETACTIONNAME(VEHICLE_ENTER_EXIT);
	SETACTIONNAME(GO_LEFT);
	SETACTIONNAME(GO_RIGHT);
	SETACTIONNAME(GO_FORWARD);
	SETACTIONNAME(GO_BACK);
	SETACTIONNAME(VEHICLE_TURRETLEFT);
	SETACTIONNAME(VEHICLE_TURRETRIGHT);
	SETACTIONNAME(VEHICLE_TURRETUP);
	SETACTIONNAME(VEHICLE_TURRETDOWN);
	SETACTIONNAME(NETWORK_TALK);
	SETACTIONNAME(TOGGLE_DPAD);
	SETACTIONNAME(SWITCH_DEBUG_CAM_ON);
	SETACTIONNAME(TAKE_SCREEN_SHOT);

#undef SETACTIONNAME
}

void CControllerConfigManager::UpdateJoyInConfigMenus_ButtonDown(int32 button, int32 padnumber)
{
	if (button != 0)
	{
		CPad *pad = CPad::GetPad(padnumber);
		if (pad != NULL)
		{
			switch (button)
			{
			case 16:
				pad->PCTempJoyState.DPadLeft = 255;
				break;
			case 15:
				pad->PCTempJoyState.DPadDown = 255;
				break;
			case 14:
				pad->PCTempJoyState.DPadRight = 255;
				break;
			case 13:
				pad->PCTempJoyState.DPadUp = 255;
				break;
			case 12:
#ifndef REGISTER_START_BUTTON
				if (padnumber == 1)
#endif
					pad->PCTempJoyState.Start = 255;
				break;
			case 11:
				pad->PCTempJoyState.RightShock = 255;
				break;
			case 10:
				pad->PCTempJoyState.LeftShock = 255;
				break;
			case 9:
				pad->PCTempJoyState.Select = 255;
				break;
			case 8:
				pad->PCTempJoyState.RightShoulder1 = 255;
				break;
			case 7:
				pad->PCTempJoyState.LeftShoulder1 = 255;
				break;
			case 6:
				pad->PCTempJoyState.RightShoulder2 = 255;
				break;
			case 5:
				pad->PCTempJoyState.LeftShoulder2 = 255;
				break;
			}

			// Now we use SDL Game Controller DB
#if defined RW_D3D9 || defined RWLIBS
			if (AllValidWinJoys.m_aJoys[JOYSTICK1].m_nVendorID == 0x3427
				&& AllValidWinJoys.m_aJoys[JOYSTICK1].m_nProductID == 0x1190)
#else
			if (0)
#endif
			{
				//GIC USB Joystick, PS2 Gamepad ?

				switch (button)
				{
				case 4:
					pad->PCTempJoyState.Square = 255;
					break;
				case 3:
					pad->PCTempJoyState.Cross = 255;
					break;
				case 2:
					pad->PCTempJoyState.Circle = 255;
					break;
				case 1:
					pad->PCTempJoyState.Triangle = 255;
					break;
				}
			}
			else
			{
				switch (button)
				{
				case 4:
					pad->PCTempJoyState.Triangle = 255;
					break;
				case 3:
					pad->PCTempJoyState.Square = 255;
					break;
				case 2:
					pad->PCTempJoyState.Cross = 255;
					break;
				case 1:
					pad->PCTempJoyState.Circle = 255;
					break;
				}
			}
		}
	}
}

void CControllerConfigManager::AffectControllerStateOn_ButtonDown(int32 button, eControllerType type)
{
	bool process = true;
	
	if ((type == KEYBOARD || type == OPTIONAL_EXTRA) && button == rsNULL)
		process = false;
	if (type == JOYSTICK && button == 0)
		process = false;
	if (type == MOUSE && button == 0)
		process = false;

	if (process)
	{
		CPad *pad = CPad::GetPad(PAD1);

		bool firstPerson = false;
		bool playerDriving = false;

		if (FindPlayerVehicle() != NULL)
		{
			CPlayerPed *plr = FindPlayerPed();
			if (plr != NULL)
			{
				if (plr->m_nPedState == PED_DRIVING)
					playerDriving = true;
			}
		}

		int16 mode = TheCamera.Cams[TheCamera.ActiveCam].Mode;
		if (   mode == CCam::MODE_1STPERSON
			|| mode == CCam::MODE_SNIPER
			|| mode == CCam::MODE_ROCKETLAUNCHER
			|| mode == CCam::MODE_CAMERA
			|| mode == CCam::MODE_M16_1STPERSON)
		{
			firstPerson = true;
		}

		CControllerState *state;
		
		switch (type)
		{
		case KEYBOARD:
		case OPTIONAL_EXTRA:
			state = &CPad::GetPad(PAD1)->PCTempKeyState;
			break;
		case JOYSTICK:
			state = &CPad::GetPad(PAD1)->PCTempJoyState;
			break;
		case MOUSE:
			state = &CPad::GetPad(PAD1)->PCTempMouseState;
			break;
		default: break;
		}

		if (pad != NULL)
		{
			if (playerDriving)
			{
				AffectControllerStateOn_ButtonDown_Driving(button, type, *state);
				AffectControllerStateOn_ButtonDown_VehicleAndThirdPersonOnly(button, type, *state);
			}
			else
			{
				AffectControllerStateOn_ButtonDown_FirstAndThirdPersonOnly(button, type, *state);
				if (firstPerson)
					AffectControllerStateOn_ButtonDown_FirstPersonOnly(button, type, *state);
				else
				{
					AffectControllerStateOn_ButtonDown_ThirdPersonOnly(button, type, *state);
					AffectControllerStateOn_ButtonDown_VehicleAndThirdPersonOnly(button, type, *state);
				}
			}

			AffectControllerStateOn_ButtonDown_AllStates(button, type, *state);

#ifdef REGISTER_START_BUTTON
			if (button == 12)
				state->Start = 255;
#endif
		}
	}
}

void CControllerConfigManager::AffectControllerStateOn_ButtonDown_Driving(int32 button, eControllerType type, CControllerState &state)
{
#ifdef BIND_VEHICLE_FIREWEAPON
	if (button == Input_GetControllerKeyAssociatedWithAction(m_aSettings, VEHICLE_FIREWEAPON, type)) // rouz edit (ChatGPT)
		state.Circle = 255;
#endif
	if (button == Input_GetControllerKeyAssociatedWithAction(m_aSettings, VEHICLE_LOOKBEHIND, type)) // rouz edit (ChatGPT)
	{
		state.LeftShoulder2 = 255;
		state.RightShoulder2 = 255;
	}

	if (button == Input_GetControllerKeyAssociatedWithAction(m_aSettings, VEHICLE_LOOKLEFT, type)) // rouz edit (ChatGPT)
		state.LeftShoulder2 = 255;
	if (button == Input_GetControllerKeyAssociatedWithAction(m_aSettings, VEHICLE_LOOKRIGHT, type)) // rouz edit (ChatGPT)
		state.RightShoulder2 = 255;
	if (button == Input_GetControllerKeyAssociatedWithAction(m_aSettings, VEHICLE_HORN, type)) // rouz edit (ChatGPT)
		state.LeftShock = 255;
	if (button == Input_GetControllerKeyAssociatedWithAction(m_aSettings, VEHICLE_HANDBRAKE, type)) // rouz edit (ChatGPT)
		state.RightShoulder1 = 255;
	if (button == Input_GetControllerKeyAssociatedWithAction(m_aSettings, VEHICLE_ACCELERATE, type)) // rouz edit (ChatGPT)
		state.Cross = 255;
	if (button == Input_GetControllerKeyAssociatedWithAction(m_aSettings, VEHICLE_CHANGE_RADIO_STATION, type)) // rouz edit (ChatGPT)
		state.LeftShoulder1 = 255;
	if (button == Input_GetControllerKeyAssociatedWithAction(m_aSettings, VEHICLE_BRAKE, type)) // rouz edit (ChatGPT)
		state.Square = 255;
	if (button == Input_GetControllerKeyAssociatedWithAction(m_aSettings, TOGGLE_SUBMISSIONS, type)) // rouz edit (ChatGPT)
		state.RightShock = 255;
	
	if (button == Input_GetControllerKeyAssociatedWithAction(m_aSettings, VEHICLE_TURRETLEFT, type)) // rouz edit (ChatGPT)
	{
		if (state.RightStickX == 128 || m_aSimCheckers[SIM_X2][type])
		{
			state.RightStickX = 0;
			m_aSimCheckers[SIM_X2][type] = true;
		}
		else
		{
			state.RightStickX = -128;
		}
	}

	if (button == Input_GetControllerKeyAssociatedWithAction(m_aSettings, VEHICLE_TURRETRIGHT, type)) // rouz edit (ChatGPT)
	{
		if (state.RightStickX == -128 || m_aSimCheckers[SIM_X2][type])
		{
			state.RightStickX = 0;
			m_aSimCheckers[SIM_X2][type] = true;
		}
		else
			state.RightStickX = 128;
	}
	
	bool isDodo = false;
	if (FindPlayerVehicle() && (FindPlayerVehicle()->IsVehicle() && (
		FindPlayerVehicle()->GetModelIndex() == MI_DODO
#ifdef FIX_BUGS
		|| (CVehicle::bAllDodosCheat && !FindPlayerVehicle()->IsRealHeli())
#endif
		)))
	{
		isDodo = true;
	}

	
	if (button == Input_GetControllerKeyAssociatedWithAction(m_aSettings, VEHICLE_TURRETUP, type)) // rouz edit (ChatGPT)
	{
		if (isDodo == true)
		{
			if (state.LeftStickY == -128 || m_aSimCheckers[SIM_Y1][type]) // BUG: should be SIM_Y2. SIM_Y1 it's DPAD
			{
				state.LeftStickY = 0;
				m_aSimCheckers[SIM_Y2][type] = true;
			}
			else
				state.LeftStickY = 128;
		}

		else if (state.RightStickY == -128 || m_aSimCheckers[SIM_Y2][type])
		{
			state.RightStickY = 0;
			m_aSimCheckers[SIM_Y2][type] = true;
		}
		else
		{
			state.RightStickY = 128;
		}
	}
	
	if (button == Input_GetControllerKeyAssociatedWithAction(m_aSettings, VEHICLE_TURRETDOWN, type)) // rouz edit (ChatGPT)
	{
		if (isDodo == true)
		{
			if (state.LeftStickY == 128 || m_aSimCheckers[SIM_Y1][type]) // BUG: should be SIM_Y2. SIM_Y1 it's DPAD
			{
				state.LeftStickY = 0;
				m_aSimCheckers[SIM_Y2][type] = true;
			}
			else
				state.LeftStickY = -128;
		}

		else if (state.RightStickY == 128 || m_aSimCheckers[SIM_Y2][type])
		{
			state.RightStickY = 0;
			m_aSimCheckers[SIM_Y2][type] = true;
		}
		else
			state.RightStickY = -128;
	}
}

void CControllerConfigManager::AffectControllerStateOn_ButtonDown_FirstPersonOnly(int32 button, eControllerType type, CControllerState &state)
{
	if (button == Input_GetControllerKeyAssociatedWithAction(m_aSettings, PED_SNIPER_ZOOM_IN, type)) // rouz edit (ChatGPT)
		state.Square = 255;
	if (button == Input_GetControllerKeyAssociatedWithAction(m_aSettings, PED_SNIPER_ZOOM_OUT, type)) // rouz edit (ChatGPT)
		state.Cross = 255;
	if (button == Input_GetControllerKeyAssociatedWithAction(m_aSettings, PED_DUCK, type)) // rouz edit (ChatGPT)
		state.LeftShock = 255;
}

void CControllerConfigManager::AffectControllerStateOn_ButtonDown_ThirdPersonOnly(int32 button, eControllerType type, CControllerState &state)
{
	if (button == Input_GetControllerKeyAssociatedWithAction(m_aSettings, PED_LOOKBEHIND, type)) // rouz edit (ChatGPT)
		state.RightShock = 255;
	if (button == Input_GetControllerKeyAssociatedWithAction(m_aSettings, PED_JUMPING, type)) // rouz edit (ChatGPT)
		state.Square = 255;
	if (button == Input_GetControllerKeyAssociatedWithAction(m_aSettings, PED_ANSWER_PHONE, type)) // rouz edit (ChatGPT)
		state.LeftShoulder1 = 255;
	if (button == Input_GetControllerKeyAssociatedWithAction(m_aSettings, PED_CYCLE_WEAPON_LEFT, type)) // rouz edit (ChatGPT)
		state.LeftShoulder2 = 255;
	if (button == Input_GetControllerKeyAssociatedWithAction(m_aSettings, PED_CYCLE_WEAPON_RIGHT, type)) // rouz edit (ChatGPT)
		state.RightShoulder2 = 255;
	if (button == Input_GetControllerKeyAssociatedWithAction(m_aSettings, PED_SPRINT, type)) // rouz edit (ChatGPT)
		state.Cross = 255;
	if (button == Input_GetControllerKeyAssociatedWithAction(m_aSettings, PED_DUCK, type)) // rouz edit (ChatGPT)
		state.LeftShock = 255;
	
	if (FrontEndMenuManager.m_ControlMethod == CONTROL_CLASSIC)
	{
		if (button == Input_GetControllerKeyAssociatedWithAction(m_aSettings, PED_CYCLE_TARGET_LEFT, type)) // rouz edit (ChatGPT)
			state.LeftShoulder2 = 255;
		if (button == Input_GetControllerKeyAssociatedWithAction(m_aSettings, PED_CYCLE_TARGET_RIGHT, type)) // rouz edit (ChatGPT)
			state.RightShoulder2 = 255;
		if (button == Input_GetControllerKeyAssociatedWithAction(m_aSettings, PED_CENTER_CAMERA_BEHIND_PLAYER, type)) // rouz edit (ChatGPT)
			state.LeftShoulder1 = 255;
	}
}

void CControllerConfigManager::AffectControllerStateOn_ButtonDown_FirstAndThirdPersonOnly(int32 button, eControllerType type, CControllerState &state)
{
	CPad *pad = CPad::GetPad(PAD1);

#ifdef BIND_VEHICLE_FIREWEAPON
	if (button == Input_GetControllerKeyAssociatedWithAction(m_aSettings, PED_FIREWEAPON, type)) // rouz edit (ChatGPT)
		state.Circle = 255;
#endif
	if (button == Input_GetControllerKeyAssociatedWithAction(m_aSettings, PED_LOCK_TARGET, type)) // rouz edit (ChatGPT)
		state.RightShoulder1 = 255;

	if (button == Input_GetControllerKeyAssociatedWithAction(m_aSettings, GO_FORWARD, type)) // rouz edit (ChatGPT)
	{
		if (state.DPadDown || m_aSimCheckers[SIM_Y1][type])
		{
			m_aSimCheckers[SIM_Y1][type] = true;
			state.DPadDown = 0;
			state.DPadUp = 0;
		}
		else
			state.DPadUp = 255;
	}

	if (button == Input_GetControllerKeyAssociatedWithAction(m_aSettings, GO_BACK, type)) // rouz edit (ChatGPT)
	{
		if (state.DPadUp || m_aSimCheckers[SIM_Y1][type])
		{
			m_aSimCheckers[SIM_Y1][type] = true;
			state.DPadDown = 0;
			state.DPadUp = 0;
		}
		else
			state.DPadDown = 255;
	}

	if (button == Input_GetControllerKeyAssociatedWithAction(m_aSettings, PED_1RST_PERSON_LOOK_LEFT, type)) // rouz edit (ChatGPT)
	{
		if (state.RightStickX == 128 || m_aSimCheckers[SIM_X2][type])
		{
			state.RightStickX = 0;
			m_aSimCheckers[SIM_X2][type] = true;
		}
		else
		{
			state.RightStickX = -128;
		}
	}

	if (button == Input_GetControllerKeyAssociatedWithAction(m_aSettings, PED_1RST_PERSON_LOOK_RIGHT, type)) // rouz edit (ChatGPT)
	{
		if (state.RightStickX == -128 || m_aSimCheckers[SIM_X2][type])
		{
			state.RightStickX = 0;
			m_aSimCheckers[SIM_X2][type] = true;
		}
		else
			state.RightStickX = 128;
	}

	if (FrontEndMenuManager.m_ControlMethod == CONTROL_CLASSIC)
	{
		if (button == Input_GetControllerKeyAssociatedWithAction(m_aSettings, PED_1RST_PERSON_LOOK_UP, type)) // rouz edit (ChatGPT)
		{
			if (state.RightStickY == -128 || m_aSimCheckers[SIM_Y2][type])
			{
				state.RightStickY = 0;
				m_aSimCheckers[SIM_Y2][type] = true;
			}
			else
				state.RightStickY = 128;
		}

		if (button == Input_GetControllerKeyAssociatedWithAction(m_aSettings, PED_1RST_PERSON_LOOK_DOWN, type)) // rouz edit (ChatGPT)
		{
			if (state.RightStickY == 128 || m_aSimCheckers[SIM_Y2][type])
			{
				state.RightStickY = 0;
				m_aSimCheckers[SIM_Y2][type] = true;
			}
			else
				state.RightStickY = -128;
		}
	}
}

void CControllerConfigManager::AffectControllerStateOn_ButtonDown_AllStates(int32 button, eControllerType type, CControllerState &state)
{
	if (button == Input_GetControllerKeyAssociatedWithAction(m_aSettings, CAMERA_CHANGE_VIEW_ALL_SITUATIONS, type)) // rouz edit (ChatGPT)
		state.Select = 255;

#ifndef BIND_VEHICLE_FIREWEAPON
	if (button == Input_GetControllerKeyAssociatedWithAction(m_aSettings, PED_FIREWEAPON, type)) // rouz edit (ChatGPT)
		state.Circle = 255;
#endif

	if (button == Input_GetControllerKeyAssociatedWithAction(m_aSettings, GO_LEFT, type)) // rouz edit (ChatGPT)
	{
		if (state.DPadRight || m_aSimCheckers[SIM_X1][type])
		{
			m_aSimCheckers[SIM_X1][type] = true;
			state.DPadLeft = 0;
			state.DPadRight = 0;
		}
		else
			state.DPadLeft = 255;
	}

	if (button == Input_GetControllerKeyAssociatedWithAction(m_aSettings, GO_RIGHT, type)) // rouz edit (ChatGPT)
	{
		if (state.DPadLeft || m_aSimCheckers[SIM_X1][type])
		{
			m_aSimCheckers[SIM_X1][type] = true;
			state.DPadLeft = 0;
			state.DPadRight = 0;
		}
		else
			state.DPadRight = 255;
	}

	if (button == Input_GetControllerKeyAssociatedWithAction(m_aSettings, NETWORK_TALK, type)) // rouz edit (ChatGPT)
		state.NetworkTalk = 255;
}

void CControllerConfigManager::AffectControllerStateOn_ButtonDown_VehicleAndThirdPersonOnly(int32 button, eControllerType type, CControllerState &state)
{
	if (button == Input_GetControllerKeyAssociatedWithAction(m_aSettings, VEHICLE_ENTER_EXIT, type)) // rouz edit (ChatGPT)
		state.Triangle = 255;
}

void CControllerConfigManager::UpdateJoyInConfigMenus_ButtonUp(int32 button, int32 padnumber)
{
	if (button!=0)
	{
		CPad *pad = CPad::GetPad(padnumber);

		if (pad != NULL)
		{
			switch (button)
			{
			case 16:
				pad->PCTempJoyState.DPadLeft = 0;
				break;
			case 15:
				pad->PCTempJoyState.DPadDown = 0;
				break;
			case 14:
				pad->PCTempJoyState.DPadRight = 0;
				break;
			case 13:
				pad->PCTempJoyState.DPadUp = 0;
				break;
#ifdef REGISTER_START_BUTTON
			case 12:
				pad->PCTempJoyState.Start = 0;
				break;
#endif
			case 11:
				pad->PCTempJoyState.RightShock = 0;
				break;
			case 10:
				pad->PCTempJoyState.LeftShock = 0;
				break;
			case 9:
				pad->PCTempJoyState.Select = 0;
				break;
			case 8:
				pad->PCTempJoyState.RightShoulder1 = 0;
				break;
			case 7:
				pad->PCTempJoyState.LeftShoulder1 = 0;
				break;
			case 6:
				pad->PCTempJoyState.RightShoulder2 = 0;
				break;
			case 5:
				pad->PCTempJoyState.LeftShoulder2 = 0;
				break;
			}

			// Now we use SDL Game Controller DB
#if defined RW_D3D9 || defined RWLIBS
			if (AllValidWinJoys.m_aJoys[JOYSTICK1].m_nVendorID == 0x3427
				&& AllValidWinJoys.m_aJoys[JOYSTICK1].m_nProductID == 0x1190)
#else
			if (0)
#endif
			{
				//GIC USB Joystick, PS2 Gamepad ?

				switch (button)
				{
				case 4:
					pad->PCTempJoyState.Square = 0;
					break;
				case 3:
					pad->PCTempJoyState.Cross = 0;
					break;
				case 2:
					pad->PCTempJoyState.Circle = 0;
					break;
				case 1:
					pad->PCTempJoyState.Triangle = 0;
					break;
				}
			}
			else
			{
				switch (button)
				{
				case 4:
					pad->PCTempJoyState.Triangle = 0;
					break;
				case 3:
					pad->PCTempJoyState.Square = 0;
					break;
				case 2:
					pad->PCTempJoyState.Cross = 0;
					break;
				case 1:
					pad->PCTempJoyState.Circle = 0;
					break;
				}
			}
		}
	}
}

void CControllerConfigManager::AffectControllerStateOn_ButtonUp(int32 button, eControllerType type)
{
	bool process = true;

	if ((type == KEYBOARD || type == OPTIONAL_EXTRA) && button == rsNULL)
		process = false;
	if (type == JOYSTICK && button == 0)
		process = false;
	if (type == MOUSE && button == 0)
		process = false;

	CControllerState *state;

	switch (type)
	{
	case KEYBOARD:
	case OPTIONAL_EXTRA:
		state = &CPad::GetPad(PAD1)->PCTempKeyState;
		break;
	case MOUSE:
		state = &CPad::GetPad(PAD1)->PCTempMouseState;
		break;
	case JOYSTICK:
		state = &CPad::GetPad(PAD1)->PCTempJoyState;
		break;
	default: break;
	}

	if (process)
	{
		CPad *pad = CPad::GetPad(PAD1);
		
		if (pad != NULL)
		{
			if (FrontEndMenuManager.GetIsMenuActive())
				AffectControllerStateOn_ButtonUp_All_Player_States(button, type, *state);

#ifdef REGISTER_START_BUTTON
			if (button == 12)
				state->Start = 0;
#endif
		}
	}
}

void CControllerConfigManager::AffectControllerStateOn_ButtonUp_All_Player_States(int32 button, eControllerType type, CControllerState &state)
{
	if (button == Input_GetControllerKeyAssociatedWithAction(m_aSettings, NETWORK_TALK, type)) // rouz edit (ChatGPT)
		state.NetworkTalk = 0;
}

//+ rouz edit (ChatGPT)
void InputConfig_TranslateShift(void)
{
    // Preserve the original shift translation call
    RsKeyCodes code;
    _InputTranslateShiftKeyUpDown(&code);
}
bool InputConfig_MenuActive(void)
{
    // Query the original menu state at the same decision point
    return FrontEndMenuManager.GetIsMenuActive();
}
void InputConfig_ButtonDown(int32_t key, int32_t type)
{
    // Dispatch the original controller button-down operation
    ControlsManager.AffectControllerStateOn_ButtonDown(key, (eControllerType)type);
}
void InputConfig_ButtonUp(int32_t key, int32_t type)
{
    // Dispatch the original controller button-up operation
    ControlsManager.AffectControllerStateOn_ButtonUp(key, (eControllerType)type);
}
//- rouz edit (ChatGPT)











#define CLEAR_ACTION_IF_NEEDED(action) \
if (key == Input_GetControllerKeyAssociatedWithAction(m_aSettings, action, type))\
	Input_ClearSettingsAssociatedWithAction(m_aSettings, action, type); // rouz edit (ChatGPT)

void CControllerConfigManager::DeleteMatchingCommonControls(e_ControllerAction action, int32 key, eControllerType type)
{
	if (!Input_GetIsKeyBlank(key, type)) // rouz edit (ChatGPT)
	{
#ifndef BIND_VEHICLE_FIREWEAPON
		CLEAR_ACTION_IF_NEEDED(PED_FIREWEAPON);
#endif
		CLEAR_ACTION_IF_NEEDED(GO_LEFT);
		CLEAR_ACTION_IF_NEEDED(GO_RIGHT);
		CLEAR_ACTION_IF_NEEDED(CAMERA_CHANGE_VIEW_ALL_SITUATIONS);
		CLEAR_ACTION_IF_NEEDED(NETWORK_TALK);
		CLEAR_ACTION_IF_NEEDED(SWITCH_DEBUG_CAM_ON);
		CLEAR_ACTION_IF_NEEDED(TOGGLE_DPAD);
		CLEAR_ACTION_IF_NEEDED(TAKE_SCREEN_SHOT);
		CLEAR_ACTION_IF_NEEDED(SHOW_MOUSE_POINTER_TOGGLE);
	}
}

void CControllerConfigManager::DeleteMatching3rdPersonControls(e_ControllerAction action, int32 key, eControllerType type)
{
	if (!Input_GetIsKeyBlank(key, type)) // rouz edit (ChatGPT)
	{
		CLEAR_ACTION_IF_NEEDED(PED_CYCLE_WEAPON_LEFT);
		CLEAR_ACTION_IF_NEEDED(PED_CYCLE_WEAPON_RIGHT);
		CLEAR_ACTION_IF_NEEDED(PED_JUMPING);
		CLEAR_ACTION_IF_NEEDED(PED_SPRINT);
		CLEAR_ACTION_IF_NEEDED(PED_LOOKBEHIND);
		CLEAR_ACTION_IF_NEEDED(PED_DUCK);
		CLEAR_ACTION_IF_NEEDED(PED_ANSWER_PHONE);

		if (FrontEndMenuManager.m_ControlMethod == CONTROL_CLASSIC)
		{
			CLEAR_ACTION_IF_NEEDED(PED_CYCLE_TARGET_LEFT);
			CLEAR_ACTION_IF_NEEDED(PED_CYCLE_TARGET_RIGHT);
			CLEAR_ACTION_IF_NEEDED(PED_CENTER_CAMERA_BEHIND_PLAYER);
		}
	}
}

void CControllerConfigManager::DeleteMatching1rst3rdPersonControls(e_ControllerAction action, int32 key, eControllerType type)
{
	if (!Input_GetIsKeyBlank(key, type)) // rouz edit (ChatGPT)
	{
#ifdef BIND_VEHICLE_FIREWEAPON
		CLEAR_ACTION_IF_NEEDED(PED_FIREWEAPON);
#endif
		CLEAR_ACTION_IF_NEEDED(PED_LOCK_TARGET);
		CLEAR_ACTION_IF_NEEDED(GO_FORWARD);
		CLEAR_ACTION_IF_NEEDED(GO_BACK);

		if (FrontEndMenuManager.m_ControlMethod == CONTROL_CLASSIC)
		{
			CLEAR_ACTION_IF_NEEDED(PED_1RST_PERSON_LOOK_LEFT);
			CLEAR_ACTION_IF_NEEDED(PED_1RST_PERSON_LOOK_RIGHT);
			CLEAR_ACTION_IF_NEEDED(PED_1RST_PERSON_LOOK_DOWN);
			CLEAR_ACTION_IF_NEEDED(PED_1RST_PERSON_LOOK_UP);
		}
	}
}

void CControllerConfigManager::DeleteMatchingVehicleControls(e_ControllerAction action, int32 key, eControllerType type)
{
	if (!Input_GetIsKeyBlank(key, type)) // rouz edit (ChatGPT)
	{
#ifdef BIND_VEHICLE_FIREWEAPON
		CLEAR_ACTION_IF_NEEDED(VEHICLE_FIREWEAPON);
#endif
		CLEAR_ACTION_IF_NEEDED(VEHICLE_ACCELERATE);
		CLEAR_ACTION_IF_NEEDED(VEHICLE_BRAKE);
		CLEAR_ACTION_IF_NEEDED(VEHICLE_CHANGE_RADIO_STATION);
		CLEAR_ACTION_IF_NEEDED(VEHICLE_HORN);
		CLEAR_ACTION_IF_NEEDED(TOGGLE_SUBMISSIONS);
		CLEAR_ACTION_IF_NEEDED(VEHICLE_HANDBRAKE);
		CLEAR_ACTION_IF_NEEDED(VEHICLE_LOOKLEFT);
		CLEAR_ACTION_IF_NEEDED(VEHICLE_LOOKRIGHT);
		CLEAR_ACTION_IF_NEEDED(VEHICLE_LOOKBEHIND);
		CLEAR_ACTION_IF_NEEDED(VEHICLE_TURRETLEFT);
		CLEAR_ACTION_IF_NEEDED(VEHICLE_TURRETRIGHT);
		CLEAR_ACTION_IF_NEEDED(VEHICLE_TURRETUP);
		CLEAR_ACTION_IF_NEEDED(VEHICLE_TURRETDOWN);
	}
}

void CControllerConfigManager::DeleteMatchingVehicle_3rdPersonControls(e_ControllerAction action, int32 key, eControllerType type)
{
	if (!Input_GetIsKeyBlank(key, type)) // rouz edit (ChatGPT)
	{
		CLEAR_ACTION_IF_NEEDED(VEHICLE_ENTER_EXIT);
	}
}

void CControllerConfigManager::DeleteMatching1rstPersonControls(e_ControllerAction action, int32 key, eControllerType type)
{
	if (!Input_GetIsKeyBlank(key, type)) // rouz edit (ChatGPT)
	{
		CLEAR_ACTION_IF_NEEDED(PED_SNIPER_ZOOM_IN);
		CLEAR_ACTION_IF_NEEDED(PED_SNIPER_ZOOM_OUT);
	}
}

#undef CLEAR_ACTION_IF_NEEDED

#ifdef RADIO_SCROLL_TO_PREV_STATION
#define CHECK_ACTION(action) \
if (key == Input_GetControllerKeyAssociatedWithAction(m_aSettings, action, type))\
	return true;

bool CControllerConfigManager::IsAnyVehicleActionAssignedToMouseKey(int32 key)
{
	const eControllerType type = MOUSE;
	if (!Input_GetIsKeyBlank(key, type)) // rouz edit (ChatGPT)
	{
#ifdef BIND_VEHICLE_FIREWEAPON
		CHECK_ACTION(VEHICLE_FIREWEAPON);
#endif
		CHECK_ACTION(VEHICLE_LOOKBEHIND);
		CHECK_ACTION(VEHICLE_LOOKLEFT);
		CHECK_ACTION(VEHICLE_LOOKRIGHT);
		CHECK_ACTION(VEHICLE_HORN);
		CHECK_ACTION(VEHICLE_HANDBRAKE);
		CHECK_ACTION(VEHICLE_ACCELERATE);
		CHECK_ACTION(VEHICLE_BRAKE);
		CHECK_ACTION(VEHICLE_CHANGE_RADIO_STATION);
		CHECK_ACTION(TOGGLE_SUBMISSIONS);
		CHECK_ACTION(VEHICLE_TURRETLEFT);
		CHECK_ACTION(VEHICLE_TURRETRIGHT);
		CHECK_ACTION(VEHICLE_TURRETUP);
		CHECK_ACTION(VEHICLE_TURRETDOWN);
		CHECK_ACTION(VEHICLE_ENTER_EXIT);
		CHECK_ACTION(CAMERA_CHANGE_VIEW_ALL_SITUATIONS);
#ifndef BIND_VEHICLE_FIREWEAPON
		CHECK_ACTION(PED_FIREWEAPON);
#endif
		CHECK_ACTION(GO_LEFT);
		CHECK_ACTION(GO_RIGHT);
		CHECK_ACTION(NETWORK_TALK);
		CHECK_ACTION(SWITCH_DEBUG_CAM_ON);
		CHECK_ACTION(TOGGLE_DPAD);
		CHECK_ACTION(TAKE_SCREEN_SHOT);
		CHECK_ACTION(SHOW_MOUSE_POINTER_TOGGLE);
	}
	return false;
}

#undef CHECK_ACTION
#endif

void CControllerConfigManager::DeleteMatchingActionInitiators(e_ControllerAction action, int32 key, eControllerType type)
{
	if (!Input_GetIsKeyBlank(key, type)) // rouz edit (ChatGPT)
	{
		switch (GetActionType(action))
		{
		case ACTIONTYPE_1RSTPERSON:
			DeleteMatchingCommonControls           (action, key, type);
			DeleteMatching1rstPersonControls       (action, key, type);
			DeleteMatching1rst3rdPersonControls    (action, key, type);
			break;
		case ACTIONTYPE_3RDPERSON:
			DeleteMatchingCommonControls           (action, key, type);
			DeleteMatching1rst3rdPersonControls    (action, key, type);
			DeleteMatching3rdPersonControls        (action, key, type);
			DeleteMatchingVehicle_3rdPersonControls(action, key, type);
			break;
		case ACTIONTYPE_VEHICLE:
			DeleteMatchingCommonControls           (action, key, type);
			DeleteMatchingVehicleControls          (action, key, type);
			DeleteMatchingVehicle_3rdPersonControls(action, key, type);
			break;
		case ACTIONTYPE_VEHICLE_3RDPERSON:
			DeleteMatchingCommonControls           (action, key, type);
			DeleteMatching1rst3rdPersonControls    (action, key, type);
			DeleteMatching3rdPersonControls        (action, key, type);
			DeleteMatchingVehicleControls          (action, key, type);
			break;
		case ACTIONTYPE_COMMON:
			DeleteMatchingCommonControls           (action, key, type);
			DeleteMatching1rstPersonControls       (action, key, type);
			DeleteMatching1rst3rdPersonControls    (action, key, type);
			DeleteMatching3rdPersonControls        (action, key, type);
			DeleteMatchingVehicleControls          (action, key, type);
			DeleteMatchingVehicle_3rdPersonControls(action, key, type);
			break;
		case ACTIONTYPE_1RST3RDPERSON:
			DeleteMatchingCommonControls           (action, key, type);
			DeleteMatching1rstPersonControls       (action, key, type);
			DeleteMatching1rst3rdPersonControls    (action, key, type);
			DeleteMatching3rdPersonControls        (action, key, type);
			DeleteMatchingVehicle_3rdPersonControls(action, key, type);
			break;
		default: break;
		}
	}
}



e_ControllerActionType CControllerConfigManager::GetActionType(e_ControllerAction action)
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
		break;

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
		break;

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
		break;

	case VEHICLE_ENTER_EXIT:
		return ACTIONTYPE_VEHICLE_3RDPERSON;
		break;

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
		break;

	case PED_SNIPER_ZOOM_IN:
	case PED_SNIPER_ZOOM_OUT:
		return ACTIONTYPE_1RSTPERSON;
		break;
	default: break;
	}

	return ACTIONTYPE_NONE;
}



wchar *CControllerConfigManager::GetControllerSettingTextWithOrderNumber(e_ControllerAction action, eContSetOrder setorder)
{
	for (int i = 0; i < MAX_CONTROLLERTYPES; i++)
	{
		if (m_aSettings[action][i].m_ContSetOrder == setorder)
		{
			switch (i)
			{
			case KEYBOARD:
			case OPTIONAL_EXTRA:
				return GetControllerSettingTextKeyBoard(action, (eControllerType)i);
			case MOUSE:
				return GetControllerSettingTextMouse   (action);
			case JOYSTICK:
				return GetControllerSettingTextJoystick(action);
			default: break;
			}
		}
	}
	
	return NULL;
}

wchar *CControllerConfigManager::GetControllerSettingTextKeyBoard(e_ControllerAction action, eControllerType type)
{
	static wchar ActionText[50];
	static wchar NewStringWithNumber[30];

	for (int32 i = 0; i < ARRAY_SIZE(ActionText); i++)
		ActionText[i] = '\0';

	if (Input_GetControllerKeyAssociatedWithAction(m_aSettings, action, type) != rsNULL) // rouz edit (ChatGPT)
	{
		if (   Input_GetControllerKeyAssociatedWithAction(m_aSettings, action, type) >= 0 // rouz edit (ChatGPT)
			&& Input_GetControllerKeyAssociatedWithAction(m_aSettings, action, type) <= 255) // rouz edit (ChatGPT)
		{
			char c = Input_GetControllerKeyAssociatedWithAction(m_aSettings, action, type); // rouz edit (ChatGPT)
			if (c == ' ')
				return Text_Get(&TheText,"FEC_SPC"); // "SPC"
			else
			{
				ActionText[0] = CFont::character_code(c);
				if (ActionText[0] == '\0')
					ActionText[0] = CFont::character_code('#');
				ActionText[1] = '\0';
				return ActionText;
			}
		}
		else
		{
			switch (Input_GetControllerKeyAssociatedWithAction(m_aSettings, action, type)) // rouz edit (ChatGPT)
			{
				case rsF1:
				case rsF2:
				case rsF3:
				case rsF4:
				case rsF5:
				case rsF6:
				case rsF7:
				case rsF8:
				case rsF9:
				case rsF10:
				case rsF11:
				case rsF12:
				{
					Messages_InsertNumberInString(Text_Get(&TheText,"FEC_FNC"), // "F~1~"
						Input_GetControllerKeyAssociatedWithAction(m_aSettings, action, type) - rsESC, // rouz edit (ChatGPT)
						-1, -1, -1, -1, -1,
						NewStringWithNumber);
					return NewStringWithNumber;
					break;
				}

				case rsINS:
				{
					return Text_Get(&TheText,"FEC_IRT"); // "INS"
					break;
				}

				case rsDEL:
				{
					return Text_Get(&TheText,"FEC_DLL"); // "DEL"
					break;
				}

				case rsHOME:
				{
					return Text_Get(&TheText,"FEC_HME"); // "HOME"
					break;
				}

				case rsEND:
				{
					return Text_Get(&TheText,"FEC_END"); // "END"
					break;
				}

				case rsPGUP:
				{
					return Text_Get(&TheText,"FEC_PGU"); // "PGUP"
					break;
				}

				case rsPGDN:
				{
					return Text_Get(&TheText,"FEC_PGD"); // "PGDN"
					break;
				}

				case rsUP:
				{
					return Text_Get(&TheText,"FEC_UPA"); // "UP"
					break;
				}

				case rsDOWN:
				{
					return Text_Get(&TheText,"FEC_DWA"); // "DOWN"
					break;
				}

				case rsLEFT:
				{
					return Text_Get(&TheText,"FEC_LFA"); // "LEFT"
					break;
				}

				case rsRIGHT:
				{
					return Text_Get(&TheText,"FEC_RFA"); // "RIGHT"
					break;
				}

				case rsDIVIDE:
				{
					return Text_Get(&TheText,"FEC_FWS"); // "NUM /"
					break;
				}

				case rsTIMES:
				{
					return Text_Get(&TheText,"FEC_STR"); // "NUM STAR"
					break;
				}

				case rsPLUS:
				{
					return Text_Get(&TheText,"FEC_PLS"); // "NUM +"
					break;
				}

				case rsMINUS:
				{
					return Text_Get(&TheText,"FEC_MIN"); // "NUM -"
					break;
				}

				case rsPADDEL:
				{
					return Text_Get(&TheText,"FEC_DOT"); // "NUM ."
					break;
				}

				case rsPADEND:
				{
					Messages_InsertNumberInString(Text_Get(&TheText,"FEC_NMN"), // "NUM~1~"
						1, -1, -1, -1, -1, -1, NewStringWithNumber);
					return NewStringWithNumber;
					break;
				}

				case rsPADDOWN:
				{
					Messages_InsertNumberInString(Text_Get(&TheText,"FEC_NMN"), // "NUM~1~"
						2, -1, -1, -1, -1, -1,
						NewStringWithNumber);
					return NewStringWithNumber;
					break;
				}

				case rsPADPGDN:
				{
					Messages_InsertNumberInString(Text_Get(&TheText,"FEC_NMN"), // "NUM~1~"
						3, -1, -1, -1, -1, -1,
						NewStringWithNumber);
					return NewStringWithNumber;
					break;
				}

				case rsPADLEFT:
				{
					Messages_InsertNumberInString(Text_Get(&TheText,"FEC_NMN"), // "NUM~1~"
						4, -1, -1, -1, -1, -1,
						NewStringWithNumber);
					return NewStringWithNumber;
					break;
				}

				case rsPAD5:
				{
					Messages_InsertNumberInString(Text_Get(&TheText,"FEC_NMN"), // "NUM~1~"
						5, -1, -1, -1, -1, -1,
						NewStringWithNumber);
					return NewStringWithNumber;
					break;
				}

				case rsNUMLOCK:
				{
					return Text_Get(&TheText,"FEC_NLK"); // "NUMLOCK"
					break;
				}

				case rsPADRIGHT:
				{
					Messages_InsertNumberInString(Text_Get(&TheText,"FEC_NMN"), // "NUM~1~"
						6, -1, -1, -1, -1, -1,
						NewStringWithNumber);
					return NewStringWithNumber;
					break;
				}

				case rsPADHOME:
				{
					Messages_InsertNumberInString(Text_Get(&TheText,"FEC_NMN"), // "NUM~1~"
						7, -1, -1, -1, -1, -1,
						NewStringWithNumber);
					return NewStringWithNumber;
					break;
				}

				case rsPADUP:
				{
					Messages_InsertNumberInString(Text_Get(&TheText,"FEC_NMN"), // "NUM~1~"
						8, -1, -1, -1, -1, -1,
						NewStringWithNumber);
					return NewStringWithNumber;
					break;
				}

				case rsPADPGUP:
				{
					Messages_InsertNumberInString(Text_Get(&TheText,"FEC_NMN"), // "NUM~1~"
						9, -1, -1, -1, -1, -1,
						NewStringWithNumber);
					return NewStringWithNumber;
					break;
				}

				case rsPADINS:
				{
					Messages_InsertNumberInString(Text_Get(&TheText,"FEC_NMN"), // "NUM~1~"
						0, -1, -1, -1, -1, -1,
						NewStringWithNumber);
					return NewStringWithNumber;
					break;
				}

				case rsPADENTER:
				{
					return Text_Get(&TheText,"FEC_ETR"); // "ENT"
					break;
				}

				case rsSCROLL:
				{
					return Text_Get(&TheText,"FEC_SLK"); // "SCROLL LOCK"
					break;
				}

				case rsPAUSE:
				{
					return Text_Get(&TheText,"FEC_PSB"); // "BREAK"
					break;
				}

				case rsBACKSP:
				{
					return Text_Get(&TheText,"FEC_BSP"); // "BSPACE"
					break;
				}

				case rsTAB:
				{
					return Text_Get(&TheText,"FEC_TAB"); // "TAB"
					break;
				}

				case rsCAPSLK:
				{
					return Text_Get(&TheText,"FEC_CLK"); // "CAPSLOCK"
					break;
				}

				case rsENTER:
				{
					return Text_Get(&TheText,"FEC_RTN"); // "RET"
					break;
				}

				case rsLSHIFT:
				{
					return Text_Get(&TheText,"FEC_LSF"); // "LSHIFT"
					break;
				}

				case rsRSHIFT:
				{
					return Text_Get(&TheText,"FEC_RSF"); // "RSHIFT"
					break;
				}

				case rsLCTRL:
				{
					return Text_Get(&TheText,"FEC_LCT"); // "LCTRL"
					break;
				}

				case rsRCTRL:
				{
					return Text_Get(&TheText,"FEC_RCT"); // "RCTRL"
					break;
				}

				case rsLALT:
				{
					return Text_Get(&TheText,"FEC_LAL"); // "LALT"
					break;
				}

				case rsRALT:
				{
					return Text_Get(&TheText,"FEC_RAL"); // "RALT"
					break;
				}

				case rsLWIN:
				{
					return Text_Get(&TheText,"FEC_LWD"); // "LWIN"
					break;
				}

				case rsRWIN:
				{
					return Text_Get(&TheText,"FEC_RWD"); // "RWIN"
					break;
				}

				case rsAPPS:
				{
					return Text_Get(&TheText,"FEC_WRC"); // "WINCLICK"
					break;
				}

				case rsSHIFT:
				{
					return Text_Get(&TheText,"FEC_SFT"); // "SHIFT"
					break;
				}
				default: break;
			}
		}
	}

	return NULL;
}

wchar *CControllerConfigManager::GetControllerSettingTextMouse(e_ControllerAction action)
{
	switch (m_aSettings[action][MOUSE].m_Key)
	{
	case 1:
		return Text_Get(&TheText,"FEC_MSL");	// LMB
		break;
	case 2:
		return Text_Get(&TheText,"FEC_MSM");	// MMB
		break;
	case 3:
		return Text_Get(&TheText,"FEC_MSR");	// RMB
		break;
	case 4:
		return Text_Get(&TheText,"FEC_MWF");	// WHEEL UP
		break;
	case 5:
		return Text_Get(&TheText,"FEC_MWB");	// WHEEL DN
		break;
	case 6:
		return Text_Get(&TheText,"FEC_MXO");	// MXB1
		break;
	case 7:
		return Text_Get(&TheText,"FEC_MXT");	// MXB2
		break;
	default: break;
	}

	return NULL;
}

wchar *CControllerConfigManager::GetControllerSettingTextJoystick(e_ControllerAction action)
{
	if (m_aSettings[action][JOYSTICK].m_Key == 0) 
		return NULL;

	static wchar NewStringWithNumber[30];

	Messages_InsertNumberInString(Text_Get(&TheText,"FEC_JBO"),		// JOY ~1~
		m_aSettings[action][JOYSTICK].m_Key, -1, -1, -1, -1, -1,
		NewStringWithNumber);

	return NewStringWithNumber;
}



#ifdef BIND_VEHICLE_FIREWEAPON
#define VFB(b) b,
#else
#define VFB(b)
#endif

#define CONTROLLER_BUTTONS(T, O, X, Q, L1, L2, L3, R1, R2, R3, SELECT, RSU, RSD, RSL, RSR)                                                                                         \
	{{                                                                                                                                                         \
	     O,      /* PED_FIREWEAPON */                                                                                                                          \
	     R2,     /* PED_CYCLE_WEAPON_RIGHT */                                                                                                                  \
	     L2,     /* PED_CYCLE_WEAPON_LEFT */                                                                                                                   \
	     nil,    /* GO_FORWARD */                                                                                                                              \
	     nil,    /* GO_BACK */                                                                                                                                 \
	     LEFT,    /* GO_LEFT */                                                                                                                                 \
	     RIGHT,    /* GO_RIGHT */                                                                                                                                \
	     Q,      /* PED_SNIPER_ZOOM_IN */                                                                                                                      \
	     X,      /* PED_SNIPER_ZOOM_OUT */                                                                                                                     \
	     T,      /* VEHICLE_ENTER_EXIT */                                                                                                                      \
	     SELECT, /* CAMERA_CHANGE_VIEW_ALL_SITUATIONS */                                                                                                       \
	     Q,      /* PED_JUMPING */                                                                                                                             \
	     X,      /* PED_SPRINT */                                                                                                                              \
	     R3,     /* PED_LOOKBEHIND */                                                                                                                          \
	     L3,     /* PED_DUCK */                                                                                                                                \
	     L1,     /* PED_ANSWER_PHONE */                                                                                                                        \
	     VFB(O)  /* VEHICLE_FIREWEAPON */                                                                                                                      \
	     X,      /* VEHICLE_ACCELERATE */                                                                                                                      \
	     Q,      /* VEHICLE_BRAKE */                                                                                                                           \
	     L1,     /* VEHICLE_CHANGE_RADIO_STATION */                                                                                                            \
	     L3,     /* VEHICLE_HORN */                                                                                                                            \
	     R3,     /* TOGGLE_SUBMISSIONS */                                                                                                                      \
	     R1,     /* VEHICLE_HANDBRAKE */                                                                                                                       \
	     nil,    /* PED_1RST_PERSON_LOOK_LEFT */                                                                                                               \
	     nil,    /* PED_1RST_PERSON_LOOK_RIGHT */                                                                                                              \
	     L2,     /* VEHICLE_LOOKLEFT */                                                                                                                        \
	     R2,     /* VEHICLE_LOOKRIGHT */                                                                                                                       \
	     nil,    /* VEHICLE_LOOKBEHIND */                                                                                                                      \
	     RSL,    /* VEHICLE_TURRETLEFT */                                                                                                                      \
	     RSR,    /* VEHICLE_TURRETRIGHT */                                                                                                                     \
	     UP,    /* VEHICLE_TURRETUP */                                                                                                                        \
	     DOWN,    /* VEHICLE_TURRETDOWN */                                                                                                                      \
	     L2,     /* PED_CYCLE_TARGET_LEFT */                                                                                                                   \
	     R2,     /* PED_CYCLE_TARGET_RIGHT */                                                                                                                  \
	     L1,     /* PED_CENTER_CAMERA_BEHIND_PLAYER */                                                                                                         \
	     R1,     /* PED_LOCK_TARGET */                                                                                                                         \
	     nil,    /* NETWORK_TALK */                                                                                                                            \
	     nil,    /* PED_1RST_PERSON_LOOK_UP */                                                                                                                 \
	     nil,    /* PED_1RST_PERSON_LOOK_DOWN */                                                                                                               \
	     nil,    /* _CONTROLLERACTION_36 */                                                                                                                    \
	     nil,    /* TOGGLE_DPAD */                                                                                                                             \
	     nil,    /* SWITCH_DEBUG_CAM_ON */                                                                                                                     \
	     nil,    /* TAKE_SCREEN_SHOT */                                                                                                                        \
	     nil,    /* UNKNOWN_ACTION */                                                                                                                          \
	     nil,    /* SHOW_MOUSE_POINTER_TOGGLE */                                                                                                               \
	 },                                                                                                                                                        \
	 {                                                                                                                                                         \
	     O,      /* PED_FIREWEAPON */                                                                                                                          \
	     R2,     /* PED_CYCLE_WEAPON_RIGHT */                                                                                                                  \
	     L2,     /* PED_CYCLE_WEAPON_LEFT */                                                                                                                   \
	     nil,    /* GO_FORWARD */                                                                                                                              \
	     nil,    /* GO_BACK */                                                                                                                                 \
	     LEFT,   /* GO_LEFT */                                                                                                                                 \
	     RIGHT,  /* GO_RIGHT */                                                                                                                                \
	     Q,      /* PED_SNIPER_ZOOM_IN */                                                                                                                      \
	     X,      /* PED_SNIPER_ZOOM_OUT */                                                                                                                     \
	     T,      /* VEHICLE_ENTER_EXIT */                                                                                                                      \
	     SELECT, /* CAMERA_CHANGE_VIEW_ALL_SITUATIONS */                                                                                                       \
	     Q,      /* PED_JUMPING */                                                                                                                             \
	     X,      /* PED_SPRINT */                                                                                                                              \
	     R3,     /* PED_LOOKBEHIND */                                                                                                                          \
	     L3,     /* PED_DUCK */                                                                                                                                \
	     L1,     /* PED_ANSWER_PHONE */                                                                                                                        \
	     VFB(O)  /* VEHICLE_FIREWEAPON */                                                                                                                      \
	     X,      /* VEHICLE_ACCELERATE */                                                                                                                      \
	     Q,      /* VEHICLE_BRAKE */                                                                                                                           \
	     SELECT, /* VEHICLE_CHANGE_RADIO_STATION */                                                                                                            \
	     L1,     /* VEHICLE_HORN */                                                                                                                            \
	     R3,     /* TOGGLE_SUBMISSIONS */                                                                                                                      \
	     R1,     /* VEHICLE_HANDBRAKE */                                                                                                                       \
	     nil,    /* PED_1RST_PERSON_LOOK_LEFT */                                                                                                               \
	     nil,    /* PED_1RST_PERSON_LOOK_RIGHT */                                                                                                              \
	     L2,     /* VEHICLE_LOOKLEFT */                                                                                                                        \
	     R2,     /* VEHICLE_LOOKRIGHT */                                                                                                                       \
	     nil,    /* VEHICLE_LOOKBEHIND */                                                                                                                      \
	     RSL,    /* VEHICLE_TURRETLEFT */                                                                                                                      \
	     RSR,    /* VEHICLE_TURRETRIGHT */                                                                                                                     \
	     UP,    /* VEHICLE_TURRETUP */                                                                                                                        \
	     DOWN,    /* VEHICLE_TURRETDOWN */                                                                                                                      \
	     L2,     /* PED_CYCLE_TARGET_LEFT */                                                                                                                   \
	     R2,     /* PED_CYCLE_TARGET_RIGHT */                                                                                                                  \
	     L1,     /* PED_CENTER_CAMERA_BEHIND_PLAYER */                                                                                                         \
	     R1,     /* PED_LOCK_TARGET */                                                                                                                         \
	     nil,    /* NETWORK_TALK */                                                                                                                            \
	     nil,    /* PED_1RST_PERSON_LOOK_UP */                                                                                                                 \
	     nil,    /* PED_1RST_PERSON_LOOK_DOWN */                                                                                                               \
	     nil,    /* _CONTROLLERACTION_36 */                                                                                                                    \
	     nil,    /* TOGGLE_DPAD */                                                                                                                             \
	     nil,    /* SWITCH_DEBUG_CAM_ON */                                                                                                                     \
	     nil,    /* TAKE_SCREEN_SHOT */                                                                                                                        \
	     nil,    /* UNKNOWN_ACTION */                                                                                                                          \
	     nil,    /* SHOW_MOUSE_POINTER_TOGGLE */                                                                                                               \
	 },                                                                                                                                                        \
	 {                                                                                                                                                         \
	     X,      /* PED_FIREWEAPON */                                                                                                                          \
	     R2,     /* PED_CYCLE_WEAPON_RIGHT */                                                                                                                  \
	     L2,     /* PED_CYCLE_WEAPON_LEFT */                                                                                                                   \
	     nil,    /* GO_FORWARD */                                                                                                                              \
	     nil,    /* GO_BACK */                                                                                                                                 \
	     LEFT,   /* GO_LEFT */                                                                                                                                 \
	     RIGHT,  /* GO_RIGHT */                                                                                                                                \
	     T,      /* PED_SNIPER_ZOOM_IN */                                                                                                                      \
	     Q,      /* PED_SNIPER_ZOOM_OUT */                                                                                                                     \
	     L1,     /* VEHICLE_ENTER_EXIT */                                                                                                                      \
	     SELECT, /* CAMERA_CHANGE_VIEW_ALL_SITUATIONS */                                                                                                       \
	     Q,      /* PED_JUMPING */                                                                                                                             \
	     O,      /* PED_SPRINT */                                                                                                                              \
	     R3,     /* PED_LOOKBEHIND */                                                                                                                          \
	     L3,     /* PED_DUCK */                                                                                                                                \
	     T,     /* PED_ANSWER_PHONE */                                                                                                                        \
	     VFB(O)  /* VEHICLE_FIREWEAPON */                                                                                                                      \
	     X,      /* VEHICLE_ACCELERATE */                                                                                                                      \
	     Q,      /* VEHICLE_BRAKE */                                                                                                                           \
	     L3,     /* VEHICLE_CHANGE_RADIO_STATION */                                                                                                            \
	     R1,     /* VEHICLE_HORN */                                                                                                                            \
	     R3,     /* TOGGLE_SUBMISSIONS */                                                                                                                      \
	     T,      /* VEHICLE_HANDBRAKE */                                                                                                                       \
	     nil,    /* PED_1RST_PERSON_LOOK_LEFT */                                                                                                               \
	     nil,    /* PED_1RST_PERSON_LOOK_RIGHT */                                                                                                              \
	     L2,     /* VEHICLE_LOOKLEFT */                                                                                                                        \
	     R2,     /* VEHICLE_LOOKRIGHT */                                                                                                                       \
	     nil,    /* VEHICLE_LOOKBEHIND */                                                                                                                      \
	     RSL,    /* VEHICLE_TURRETLEFT */                                                                                                                      \
	     RSR,    /* VEHICLE_TURRETRIGHT */                                                                                                                     \
	     UP,    /* VEHICLE_TURRETUP */                                                                                                                        \
	     DOWN,    /* VEHICLE_TURRETDOWN */                                                                                                                      \
	     L2,     /* PED_CYCLE_TARGET_LEFT */                                                                                                                   \
	     R2,     /* PED_CYCLE_TARGET_RIGHT */                                                                                                                  \
	     T,      /* PED_CENTER_CAMERA_BEHIND_PLAYER */                                                                                                         \
	     R1,     /* PED_LOCK_TARGET */                                                                                                                         \
	     nil,    /* NETWORK_TALK */                                                                                                                            \
	     nil,    /* PED_1RST_PERSON_LOOK_UP */                                                                                                                 \
	     nil,    /* PED_1RST_PERSON_LOOK_DOWN */                                                                                                               \
	     nil,    /* _CONTROLLERACTION_36 */                                                                                                                    \
	     nil,    /* TOGGLE_DPAD */                                                                                                                             \
	     nil,    /* SWITCH_DEBUG_CAM_ON */                                                                                                                     \
	     nil,    /* TAKE_SCREEN_SHOT */                                                                                                                        \
	     nil,    /* UNKNOWN_ACTION */                                                                                                                          \
	     nil,    /* SHOW_MOUSE_POINTER_TOGGLE */                                                                                                               \
	 },                                                                                                                                                        \
	 {                                                                                                                                                         \
	     R1,     /* PED_FIREWEAPON */                                                                                                                          \
	     R2,     /* PED_CYCLE_WEAPON_RIGHT */                                                                                                                  \
	     L2,     /* PED_CYCLE_WEAPON_LEFT */                                                                                                                   \
	     nil,    /* GO_FORWARD */                                                                                                                              \
	     nil,    /* GO_BACK */                                                                                                                                 \
	     LEFT,   /* GO_LEFT */                                                                                                                                 \
	     RIGHT,  /* GO_RIGHT */                                                                                                                                \
	     Q,      /* PED_SNIPER_ZOOM_IN */                                                                                                                      \
	     X,      /* PED_SNIPER_ZOOM_OUT */                                                                                                                     \
	     T,      /* VEHICLE_ENTER_EXIT */                                                                                                                      \
	     SELECT, /* CAMERA_CHANGE_VIEW_ALL_SITUATIONS */                                                                                                       \
	     Q,      /* PED_JUMPING */                                                                                                                             \
	     X,      /* PED_SPRINT */                                                                                                                              \
	     R3,     /* PED_LOOKBEHIND */                                                                                                                          \
	     L3,     /* PED_DUCK */                                                                                                                                \
	     O,     /* PED_ANSWER_PHONE */                                                                                                                        \
	     VFB(R1) /* VEHICLE_FIREWEAPON */                                                                                                                      \
	     RSU,    /* VEHICLE_ACCELERATE */                                                                                                                      \
	     RSD,    /* VEHICLE_BRAKE */                                                                                                                           \
	     O,      /* VEHICLE_CHANGE_RADIO_STATION */                                                                                                            \
	     L3,     /* VEHICLE_HORN */                                                                                                                            \
	     Q,      /* TOGGLE_SUBMISSIONS */                                                                                                                      \
	     L1,     /* VEHICLE_HANDBRAKE */                                                                                                                       \
	     nil,    /* PED_1RST_PERSON_LOOK_LEFT */                                                                                                               \
	     nil,    /* PED_1RST_PERSON_LOOK_RIGHT */                                                                                                              \
	     L2,     /* VEHICLE_LOOKLEFT */                                                                                                                        \
	     R2,     /* VEHICLE_LOOKRIGHT */                                                                                                                       \
	     nil,    /* VEHICLE_LOOKBEHIND */                                                                                                                      \
	     RSL,    /* VEHICLE_TURRETLEFT */                                                                                                                      \
	     RSR,    /* VEHICLE_TURRETRIGHT */                                                                                                                     \
	     UP,    /* VEHICLE_TURRETUP */                                                                                                                        \
	     DOWN,    /* VEHICLE_TURRETDOWN */                                                                                                                      \
	     L2,     /* PED_CYCLE_TARGET_LEFT */                                                                                                                   \
	     R2,     /* PED_CYCLE_TARGET_RIGHT */                                                                                                                  \
	     O,      /* PED_CENTER_CAMERA_BEHIND_PLAYER */                                                                                                         \
	     L1,     /* PED_LOCK_TARGET */                                                                                                                         \
	     nil,    /* NETWORK_TALK */                                                                                                                            \
	     nil,    /* PED_1RST_PERSON_LOOK_UP */                                                                                                                 \
	     nil,    /* PED_1RST_PERSON_LOOK_DOWN */                                                                                                               \
	     nil,    /* _CONTROLLERACTION_36 */                                                                                                                    \
	     nil,    /* TOGGLE_DPAD */                                                                                                                             \
	     nil,    /* SWITCH_DEBUG_CAM_ON */                                                                                                                     \
	     nil,    /* TAKE_SCREEN_SHOT */                                                                                                                        \
	     nil,    /* UNKNOWN_ACTION */                                                                                                                          \
	     nil,    /* SHOW_MOUSE_POINTER_TOGGLE */                                                                                                               \
	 }}

#ifdef BUTTON_ICONS
#define UP "~U~"
#define DOWN "~D~"
#define LEFT "~<~"
#define RIGHT "~>~"
#else
#define UP "UP"
#define DOWN "DOWN"
#define LEFT "LEFT"
#define RIGHT "RIGHT"
#endif

const char *XboxButtons_noIcons[][MAX_CONTROLLERACTIONS] = CONTROLLER_BUTTONS("Y", "B", "A", "X", "LB", "LT", "LS", "RB", "RT", "RS", "BACK", "right stick up", "right stick down", "right stick left", "right stick right");

#ifdef BUTTON_ICONS
const char *XboxButtons[][MAX_CONTROLLERACTIONS] = CONTROLLER_BUTTONS("~T~", "~O~", "~X~", "~Q~", "~K~", "~M~", "~A~", "~J~", "~V~", "~C~", "BACK", "~H~", "~L~", "~(~", "~)~");
#endif


#if 0 // set 1 for ps2 fonts
#define PS2_TRIANGLE "\""
#define PS2_CIRCLE "|"
#define PS2_CROSS "/"
#define PS2_SQUARE "^"
#else
#define PS2_TRIANGLE "TRIANGLE"
#define PS2_CIRCLE "CIRCLE"
#define PS2_CROSS "CROSS"
#define PS2_SQUARE "SQUARE"
#endif

const char *PlayStationButtons_noIcons[][MAX_CONTROLLERACTIONS] =
    CONTROLLER_BUTTONS(PS2_TRIANGLE, PS2_CIRCLE, PS2_CROSS, PS2_SQUARE, "L1", "L2", "L3", "R1", "R2", "R3", "SELECT", "right stick up", "right stick down", "right stick left", "right stick right");

#ifdef BUTTON_ICONS
const char *PlayStationButtons[][MAX_CONTROLLERACTIONS] =
    CONTROLLER_BUTTONS("~T~", "~O~", "~X~", "~Q~", "~K~", "~M~", "~A~", "~J~", "~V~", "~C~", "SELECT", "~H~", "~L~", "~(~", "~)~");
#endif

#undef PS2_TRIANGLE
#undef PS2_CIRCLE
#undef PS2_CROSS
#undef PS2_SQUARE

const char *NintendoSwitchButtons_noIcons[][MAX_CONTROLLERACTIONS] =
    CONTROLLER_BUTTONS("Y", "A", "B", "X", "L", "ZL", "LS", "R", "ZR", "RS", "BACK", "right stick up", "right stick down", "right stick left", "right stick right");

#ifdef BUTTON_ICONS
const char *NintendoSwitchButtons[][MAX_CONTROLLERACTIONS] =
    CONTROLLER_BUTTONS("~T~", "~O~", "~X~", "~Q~", "~K~", "~M~", "~A~", "~J~", "~V~", "~C~", "BACK", "~H~", "~L~", "~(~", "~)~");
#endif

#undef UP
#undef DOWN
#undef LEFT
#undef RIGHT

#undef CONTROLLER_BUTTONS
#undef VFB

void CControllerConfigManager::GetWideStringOfCommandKeys(uint16 action, wchar *text, uint16 leight)
{
#ifdef DETECT_PAD_INPUT_SWITCH
	if (Input_affectedByController) { // rouz edit (ChatGPT)
		wchar wstr[16];

		const char* (*Buttons)[MAX_CONTROLLERACTIONS];

#ifdef BUTTON_ICONS
	#ifdef GAMEPAD_MENU
		switch (FrontEndMenuManager.m_PrefsControllerType)
		{
		case CMenuManager::CONTROLLER_DUALSHOCK2:
		case CMenuManager::CONTROLLER_DUALSHOCK3:
		case CMenuManager::CONTROLLER_DUALSHOCK4:
			Buttons = CFont::ButtonsSlot != -1 ? PlayStationButtons : PlayStationButtons_noIcons;
			break;
		case CMenuManager::CONTROLLER_NINTENDO_SWITCH:
			Buttons = CFont::ButtonsSlot != -1 ? NintendoSwitchButtons : NintendoSwitchButtons_noIcons;
			break;
		default:
	#endif
			Buttons = CFont::ButtonsSlot != -1 ? XboxButtons : XboxButtons_noIcons;
	#ifdef GAMEPAD_MENU
			break;
		}
	#endif
#else
		switch (FrontEndMenuManager.m_PrefsControllerType)
		{
		case CMenuManager::CONTROLLER_DUALSHOCK2:
		case CMenuManager::CONTROLLER_DUALSHOCK3:
		case CMenuManager::CONTROLLER_DUALSHOCK4:
			Buttons = PlayStationButtons_noIcons;
			break;
		case CMenuManager::CONTROLLER_NINTENDO_SWITCH:
			Buttons = NintendoSwitchButtons_noIcons;
			break;
		default:
			Buttons = XboxButtons_noIcons;
			break;
		}
#endif

		assert(Buttons[CPad::GetPad(0)->Mode][action] != nil); // we cannot use these
		AsciiToUnicode(Buttons[CPad::GetPad(0)->Mode][action], wstr);

		Messages_WideStringCopy(text, wstr, leight);
		return;
	}
#endif

	int32 nums = Input_GetNumOfSettingsForAction(m_aSettings, (e_ControllerAction)action); // rouz edit (ChatGPT)

	int32 sets = 0;

	for (int32 i = SETORDER_1; i < MAX_SETORDERS; i++)
	{
		wchar *textorder = ControlsManager.GetControllerSettingTextWithOrderNumber((e_ControllerAction)action, (eContSetOrder)i);
		if (textorder != NULL)
		{
			uint16 len = Messages_GetWideStringLength(text);
			Messages_WideStringCopy(&text[len], textorder, leight - len);

			if (++sets < nums)
			{
				if (sets == nums - 1)
				{
					// if last - text += ' or ';
					uint16 pos1 = Messages_GetWideStringLength(text);
					text[pos1] = ' ';

					Messages_WideStringCopy(&text[pos1 + 1],
						Text_Get(&TheText,"FEC_ORR"),	// "or"
						leight - (pos1 + 1));
					
					uint16 pos2 = Messages_GetWideStringLength(text);
					text[pos2 + 0] = ' ';
					text[pos2 + 1] = '\0';
				}
				else
				{
					// text += ', ';
					uint16 pos1 = Messages_GetWideStringLength(text);
					text[pos1 + 0] = ',';
					text[pos1 + 1] = ' ';
					text[pos1 + 2] = '\0';

					uint16 pos2 = Messages_GetWideStringLength(text);
				}
			}
		}
	}
}



//+ rouz edit (ChatGPT)
#ifdef RW_GL3
static int32_t InputController_MapButton(int32_t index)
{
    // Retain the original GLFW-to-game button mapping
    return MapIdToButtonId(index);
}
#endif
void CControllerConfigManager::UpdateJoyButtonState(int32 padnumber)
{
    // Preserve the original reset and platform-specific source selection
    Input_ResetJoyButtons(m_aButtonStates, MAX_BUTTONS);
#ifdef __DINPUT_INCLUDED__
    Input_ReadDirectJoyButtons(m_aButtonStates, m_NewState.rgbButtons, MAX_BUTTONS);
#elif defined RW_GL3
    if (m_NewState.isGamepad)
        Input_ReadMappedJoyButtons(m_aButtonStates, m_NewState.mappedButtons, MAX_BUTTONS,
            GLFW_GAMEPAD_BUTTON_GUIDE, InputController_MapButton);
    else
        Input_ReadRawJoyButtons(m_aButtonStates, m_NewState.buttons, m_NewState.numButtons, MAX_BUTTONS);
#endif
}
//- rouz edit (ChatGPT)

bool CControllerConfigManager::GetIsActionAButtonCombo(e_ControllerAction action)
{
	switch (action)
	{
	case VEHICLE_LOOKBEHIND:
	case PED_CYCLE_TARGET_LEFT:
	case PED_CYCLE_TARGET_RIGHT:
		return true;
		break;
	default: break;
	}

	return false;
}

wchar *CControllerConfigManager::GetButtonComboText(e_ControllerAction action)
{
	switch (action)
	{
	case PED_CYCLE_TARGET_LEFT:
		return Text_Get(&TheText,"FEC_PTL");	// Use LockTarget with Weapon Switch Left.
		break;
	
	case PED_CYCLE_TARGET_RIGHT:
		return Text_Get(&TheText,"FEC_PTR");	// Use LockTarget with Weapon Switch Right.
		break;
	
	case VEHICLE_LOOKBEHIND:
		return Text_Get(&TheText,"FEC_LBC");	// Use Look Left With Look Right. 
		break;
	default: break;
	}

	return NULL;
}








//- rouz edit (ChatGPT)
