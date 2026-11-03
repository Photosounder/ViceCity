//+ rouz edit (ChatGPT)
#pragma once

#if defined RW_D3D9 || defined RWLIBS
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#endif

// based on x-gtasa



#include "ControllerActions.h"

enum e_ControllerActionType
{
	ACTIONTYPE_1RSTPERSON = 0,
	ACTIONTYPE_3RDPERSON,
	ACTIONTYPE_VEHICLE,
	ACTIONTYPE_VEHICLE_3RDPERSON,
	ACTIONTYPE_COMMON,
	ACTIONTYPE_1RST3RDPERSON,
	ACTIONTYPE_NONE,
};





struct CMouseControllerState;
struct CControllerState;


#define JOY_BUTTONS 16
#define MAX_BUTTONS (JOY_BUTTONS+1)


#ifdef RW_GL3
struct GlfwJoyState {
	int8 id;
	bool isGamepad;
	uint8 numButtons;
	uint8* buttons;
	bool mappedButtons[17];
};
#endif

class CControllerConfigManager
{
public:

	bool                  m_bFirstCapture;
#if defined RW_GL3
	GlfwJoyState           m_OldState;
	GlfwJoyState           m_NewState;
#else
	DIJOYSTATE2           m_OldState;
	DIJOYSTATE2           m_NewState;
#endif
	wchar                 m_aActionNames[MAX_CONTROLLERACTIONS][ACTIONNAME_LENGTH];
	bool                  m_aButtonStates[MAX_BUTTONS];
	InputControllerBinding m_aSettings[MAX_CONTROLLERACTIONS][MAX_CONTROLLERTYPES];
	bool                  m_aSimCheckers[MAX_SIMS][MAX_CONTROLLERTYPES];
	bool                  m_bMouseAssociated;

#ifdef LOAD_INI_SETTINGS
	static uint32 ms_padButtonsInited;
#endif
	
	CControllerConfigManager();

	void  MakeControllerActionsBlank();
	
	int32 GetJoyButtonJustDown();
	
	void  LoadSettings(int32 file);
	
	void  InitDefaultControlConfiguration();
	void  InitDefaultControlConfigMouse(CMouseControllerState const &availableButtons);
	void  InitDefaultControlConfigJoyPad(uint32 buttons);
	void  InitialiseControllerActionNameArray();

	void  UpdateJoyInConfigMenus_ButtonDown                           (int32 button, int32 padnumber);
	void  AffectControllerStateOn_ButtonDown                          (int32 button, eControllerType type);
	void  AffectControllerStateOn_ButtonDown_Driving                  (int32 button, eControllerType type, CControllerState &state);
	void  AffectControllerStateOn_ButtonDown_FirstPersonOnly          (int32 button, eControllerType type, CControllerState &state);
	void  AffectControllerStateOn_ButtonDown_ThirdPersonOnly          (int32 button, eControllerType type, CControllerState &state);
	void  AffectControllerStateOn_ButtonDown_FirstAndThirdPersonOnly  (int32 button, eControllerType type, CControllerState &state);
	void  AffectControllerStateOn_ButtonDown_AllStates                (int32 button, eControllerType type, CControllerState &state);
	void  AffectControllerStateOn_ButtonDown_VehicleAndThirdPersonOnly(int32 button, eControllerType type, CControllerState &state);

	void  UpdateJoyInConfigMenus_ButtonUp(int32 button, int32 padnumber);
	void  AffectControllerStateOn_ButtonUp(int32 button, eControllerType type);
	void  AffectControllerStateOn_ButtonUp_All_Player_States(int32 button, eControllerType type, CControllerState &state);
	
	




	void DeleteMatchingCommonControls           (e_ControllerAction action, int32 key, eControllerType type);
	void DeleteMatching3rdPersonControls        (e_ControllerAction action, int32 key, eControllerType type);
	void DeleteMatching1rst3rdPersonControls    (e_ControllerAction action, int32 key, eControllerType type);
	void DeleteMatchingVehicleControls          (e_ControllerAction action, int32 key, eControllerType type);
	void DeleteMatchingVehicle_3rdPersonControls(e_ControllerAction action, int32 key, eControllerType type);
	void DeleteMatching1rstPersonControls       (e_ControllerAction action, int32 key, eControllerType type);
	void DeleteMatchingActionInitiators         (e_ControllerAction action, int32 key, eControllerType type);

#ifdef RADIO_SCROLL_TO_PREV_STATION
	bool IsAnyVehicleActionAssignedToMouseKey(int32 key);
#endif 

	e_ControllerActionType GetActionType(e_ControllerAction action);

	wchar *GetControllerSettingTextWithOrderNumber(e_ControllerAction action, eContSetOrder setorder);
	wchar *GetControllerSettingTextKeyBoard       (e_ControllerAction action, eControllerType type);
	wchar *GetControllerSettingTextMouse          (e_ControllerAction action);
	wchar *GetControllerSettingTextJoystick       (e_ControllerAction action);

	void  GetWideStringOfCommandKeys(uint16 action, wchar *text, uint16 leight);


	void  UpdateJoyButtonState(int32 padnumber);
	
	bool  GetIsActionAButtonCombo             (e_ControllerAction action);
	wchar *GetButtonComboText                 (e_ControllerAction action);




};

#ifndef RW_GL3
VALIDATE_SIZE(CControllerConfigManager, 0x143C);
#endif

extern CControllerConfigManager ControlsManager;
//- rouz edit (ChatGPT)
