//+ rouz edit (ChatGPT)
#include "config.h"
#include "InputState.h"
#include "InputState_backend.h"
#include "RsKeys.h"
#include "Timer.h"
#include "ControllerActions.h"
#include "FileMgr.h"
#define ARRAY_SIZE(array) (sizeof(array)/sizeof((array)[0]))
CKeyboardState Input_oldKeyState;
CKeyboardState Input_newKeyState;
CKeyboardState Input_tempKeyState;
char Input_keyboardCheatString[30];
bool Input_mapPadOneToPadTwo;
bool Input_displayNoControllerMessage;
bool Input_obsoleteControllerMessage;
bool Input_oldDisplayNoControllerMessage;
bool Input_debugCameraOn;
bool Input_playerHasCheated;
bool Input_invertPadLook;
bool Input_affectedByController;
uint32_t Input_TimeSinceLastInput(int32_t lastTimeTouched)
{
    // Preserve the original unsigned timer subtraction
    return *Timer_GetTimeInMilliseconds() - lastTimeTouched;
}
CMousePointerStateHelper MousePointerStateHelper;
CMouseControllerState Input_oldMouseState;
CMouseControllerState Input_newMouseState;
CMouseControllerState Input_tempMouseState;

int32_t Input_FindNewDirectButton(const uint8_t *current, const uint8_t *previous, int32_t count)
{
    // Return the first newly set DirectInput high bit using one-based IDs
    int32_t i;
    for (i = 0; i < count; i++)
        if ((current[i] & 0x80) && !(previous[i] & 0x80))
            return i + 1;
    return 0;
}

int32_t Input_FindNewMappedButton(const bool *current, const bool *previous, int32_t count)
{
    // Return the first new mapped index for the native GLFW mapping
    int32_t i;
    for (i = 0; i < count; i++)
        if (current[i] && !previous[i])
            return i;
    return -1;
}

int32_t Input_FindNewRawButton(const uint8_t *current, const uint8_t *previous, int32_t sourceCount, int32_t count)
{
    // Return the first new raw button within the original bounded range
    int32_t i;
    for (i = 0; i < (sourceCount < count ? sourceCount : count); i++)
        if (current[i] && !previous[i])
            return i + 1;
    return 0;
}

void Input_ResetJoyButtons(bool *buttons, int32_t count)
{
    // Clear the original complete controller button array
    int32_t i;
    for (i = 0; i < count; i++)
        buttons[i] = false;
}

void Input_ReadDirectJoyButtons(bool *buttons, const uint8_t *source, int32_t count)
{
    // Preserve the DirectInput high-bit test for every button
    int32_t i;
    for (i = 0; i < count; i++)
        buttons[i] = (source[i] & 0x80) != 0;
}

void Input_ReadMappedJoyButtons(bool *buttons, const bool *source, int32_t count, int32_t guide, int32_t (*map)(int32_t))
{
    // Preserve mapped button order and skip the guide button
    int32_t i;
    for (i = 0; i < count; i++) {
        // Leave the guide entry untouched after the original reset
        if (i == guide)
            continue;
        buttons[map(i) - 1] = source[i];
    }
}

void Input_ReadRawJoyButtons(bool *buttons, const uint8_t *source, int32_t sourceCount, int32_t count)
{
    // Copy only the original bounded raw joystick button range
    int32_t i;
    for (i = 0; i < (sourceCount < count ? sourceCount : count); i++)
        buttons[i] = source[i] != 0;
}

void Input_DispatchPadButtons(int32_t padNumber, bool padTwo, bool camera, bool release, const bool *buttons)
{
    // Update button state and retain the distinct original press and release ranges
    int32_t i;
    InputEvents_UpdateJoyButtons(padNumber);
    for (i = release ? 1 : 0; i < 16; i++) {
        // Preserve the original live button and menu reads for every slot
        int32_t button = 0;
        bool state = buttons[i];
        if (release ? !state : state)
            button = i + 1;
        InputEvents_DispatchJoyButton(button, padNumber,
            InputEvents_MenuActive() || padTwo || (release && camera), release);
    }
}

void Input_MapPadKeyDown(CControllerState *state, int32_t c)
{
    // Preserve the original ordered keyboard-to-pad assignments

			if ( c == 'D' ) state->LeftStickX = 128;
			if ( c == 'A' ) state->LeftStickX = -128;
			if ( c == 'W' ) state->LeftStickY = 128;
			if ( c == 'S' ) state->LeftStickY = -128;
			if ( c == 'J' ) state->RightStickX = 128;
			if ( c == 'G' ) state->RightStickX = -128;
			if ( c == 'Y' ) state->RightStickY = 128;
			if ( c == 'H' ) state->RightStickY = -128;
			if ( c == 'Z' ) state->LeftShoulder1 = 255;
			if ( c == 'X' ) state->LeftShoulder2 = 255;
			if ( c == 'C' ) state->RightShoulder1 = 255;
			if ( c == 'V' ) state->RightShoulder2 = 255;
			if ( c == 'O' ) state->DPadUp = 255;
			if ( c == 'L' ) state->DPadDown = 255;
			if ( c == 'K' ) state->DPadLeft = 255;
			if ( c == ';' ) state->DPadRight = 255;
			if ( c == 'B' ) state->Start = 255;
			if ( c == 'N' ) state->Select = 255;
			if ( c == 'M' ) state->Square = 255;
			if ( c == ',' ) state->Triangle = 255;
			if ( c == '.' ) state->Cross = 255;
			if ( c == '/' ) state->Circle = 255;
			if ( c == rsRSHIFT ) state->LeftShock = 255;
			if ( c == rsRCTRL ) state->RightShock = 255;
}

void Input_MapPadKeyUp(CControllerState *state, int32_t c)
{
    // Preserve the original ordered keyboard-to-pad assignments

			if ( c == 'D' ) state->LeftStickX = 0;
			if ( c == 'A' ) state->LeftStickX = 0;
			if ( c == 'W' ) state->LeftStickY = 0;
			if ( c == 'S' ) state->LeftStickY = 0;
			if ( c == 'J' ) state->RightStickX = 0;
			if ( c == 'G' ) state->RightStickX = 0;
			if ( c == 'Y' ) state->RightStickY = 0;
			if ( c == 'H' ) state->RightStickY = 0;
			if ( c == 'Z' ) state->LeftShoulder1 = 0;
			if ( c == 'X' ) state->LeftShoulder2 = 0;
			if ( c == 'C' ) state->RightShoulder1 = 0;
			if ( c == 'V' ) state->RightShoulder2 = 0;
			if ( c == 'O' ) state->DPadUp = 0;
			if ( c == 'L' ) state->DPadDown = 0;
			if ( c == 'K' ) state->DPadLeft = 0;
			if ( c == ';' ) state->DPadRight = 0;
			if ( c == 'B' ) state->Start = 0;
			if ( c == 'N' ) state->Select = 0;
			if ( c == 'M' ) state->Square = 0;
			if ( c == ',' ) state->Triangle = 0;
			if ( c == '.' ) state->Cross = 0;
			if ( c == '/' ) state->Circle = 0;
			if ( c == rsRSHIFT ) state->LeftShock = 0;
			if ( c == rsRCTRL ) state->RightShock = 0;
}

void Input_ClearSettingsAssociatedWithAction(InputControllerBinding bindings[][MAX_CONTROLLERTYPES], int32_t action, int32_t type)
{
	// Preserve the original controller binding operation
	switch (type)
	{
	case KEYBOARD:
		bindings[action][type].m_Key = rsNULL;
		bindings[action][type].m_ContSetOrder = SETORDER_NONE;
		break;
	case OPTIONAL_EXTRA:
		bindings[action][type].m_Key = rsNULL;
		bindings[action][type].m_ContSetOrder = SETORDER_NONE;
		break;
	case MOUSE:
		bindings[action][type].m_Key = 0;
		bindings[action][type].m_ContSetOrder = SETORDER_NONE;
		break;
	case JOYSTICK:
		bindings[action][type].m_Key = 0;
		bindings[action][type].m_ContSetOrder = SETORDER_NONE;
		break;
	default: break;
	}

	Input_ResetSettingOrder(bindings, action);
}

int32_t Input_GetNumOfSettingsForAction(InputControllerBinding bindings[][MAX_CONTROLLERTYPES], int32_t action)
{
	// Preserve the original controller binding operation
	int32_t num = 0;

	if (bindings[action][KEYBOARD].m_Key       != rsNULL) num++;
	if (bindings[action][OPTIONAL_EXTRA].m_Key != rsNULL) num++;
	if (bindings[action][MOUSE].m_Key          != 0)      num++;
	if (bindings[action][JOYSTICK].m_Key       != 0)      num++;

	return num;
}

int32_t Input_GetControllerKeyAssociatedWithAction(InputControllerBinding bindings[][MAX_CONTROLLERTYPES], int32_t action, int32_t type)
{
	// Preserve the original binding access and ordering behavior
	return bindings[action][type].m_Key;
}

void Input_SetControllerKeyAssociatedWithAction(InputControllerBinding bindings[][MAX_CONTROLLERTYPES], int32_t action, int32_t key, int32_t type)
{
	// Preserve the original binding access and ordering behavior
	Input_ResetSettingOrder(bindings, action); // rouz edit (ChatGPT)
	int numOfSettings = Input_GetNumOfSettingsForAction(bindings, action); // rouz edit (ChatGPT)
	
	bindings[action][type].m_Key = key;
	bindings[action][type].m_ContSetOrder = numOfSettings + 1;
}

int32_t Input_GetMouseButtonAssociatedWithAction(InputControllerBinding bindings[][MAX_CONTROLLERTYPES], int32_t action)
{
	// Preserve the original binding access and ordering behavior
	return bindings[action][MOUSE].m_Key;
}

void Input_SetMouseButtonAssociatedWithAction(InputControllerBinding bindings[][MAX_CONTROLLERTYPES], int32_t action, int32_t button)
{
	// Preserve the original binding access and ordering behavior
	int numOfSettings = Input_GetNumOfSettingsForAction(bindings, action); // rouz edit (ChatGPT)
	
	bindings[action][MOUSE].m_Key = button;
	bindings[action][MOUSE].m_ContSetOrder = numOfSettings + 1;
}

bool Input_GetIsKeyBlank(int32_t key, int32_t type)
{
	// Preserve the original device-specific blank key checks
	switch (type)
	{
	case KEYBOARD:
	case OPTIONAL_EXTRA:
		if (key != rsNULL)
			return false;
		break;

	case JOYSTICK:
		if (key != 0)
			return false;
		break;

	case MOUSE:
		if (key != 0)
			return false;
		break;
	default: break;
	}

	return true;
}

void Input_BlankControllerActions(InputControllerBinding bindings[][MAX_CONTROLLERTYPES])
{
    // Preserve controller-major clearing and per-entry order normalization
    int32_t type, action;
    for (type = 0; type < MAX_CONTROLLERTYPES; type++) {
        // Clear each action through the original binding operation
        for (action = 0; action < MAX_CONTROLLERACTIONS; action++)
            Input_ClearSettingsAssociatedWithAction(bindings, action, type);
    }
}

void Input_ClearSimButtonPressCheckers(bool checkers[][MAX_CONTROLLERTYPES])
{
    // Clear simulated input checkers in their original field order
    int32_t i;
    for (i = 0; i < MAX_SIMS; i++) {
        // Clear each device slot separately
        checkers[i][KEYBOARD] = false;
        checkers[i][OPTIONAL_EXTRA] = false;
        checkers[i][MOUSE] = false;
        checkers[i][JOYSTICK] = false;
    }
}

void Input_ResetSettingOrder(InputControllerBinding bindings[][MAX_CONTROLLERTYPES], int32_t action)
{
	// Preserve the original controller binding operation
	int32_t conttype = KEYBOARD;

	for (int32_t i = SETORDER_1; i < MAX_SETORDERS; i++)
	{
		bool isexist = false;
		for (int32_t j = 0; j < MAX_CONTROLLERTYPES; j++)
		{
			if (bindings[action][j].m_ContSetOrder == i)
				isexist = true;
		}

		bool init = false;

		if (!isexist)
		{
			for (int32_t k = 0; k < MAX_CONTROLLERTYPES; k++)
			{
				int32_t setorder = bindings[action][k].m_ContSetOrder;
				if (setorder > i && setorder != 0)
				{
					if (init)
					{
						if (setorder < bindings[action][conttype].m_ContSetOrder)
							conttype = k;
					}
					else
					{
						init = true;
						conttype = k;
					}
				}
			}

			if (init)
				bindings[action][conttype].m_ContSetOrder = i;
		}
	}
}

void Input_WriteControllerBindings(int32_t file, const InputControllerBinding bindings[][MAX_CONTROLLERTYPES])
{
    // Preserve the original file guard and controller-major write order
    int32_t type, action;
    if (!file)
        return;
    for (type = 0; type < MAX_CONTROLLERTYPES; type++) {
        // Write each action record exactly as stored
        for (action = 0; action < MAX_CONTROLLERACTIONS; action++)
            FileMgr_Write(file, (const char *)&bindings[action][type], sizeof(InputControllerBinding));
    }
}

void Input_ReadControllerBindings(int32_t file, InputControllerBinding bindings[][MAX_CONTROLLERTYPES], bool skipVehicleFireWeapon)
{
    // Preserve the original controller-major read order and version skip
    int32_t type, action;
    for (type = 0; type < MAX_CONTROLLERTYPES; type++) {
        // Read each record unless the older format omits vehicle fire
        for (action = 0; action < MAX_CONTROLLERACTIONS; action++) {
            // Keep skipped entries unchanged
            if (skipVehicleFireWeapon && action == VEHICLE_FIREWEAPON)
                continue;
            FileMgr_Read(file, (char *)&bindings[action][type], sizeof(InputControllerBinding));
        }
    }
}

void Input_AffectPadFromKeyboard(const InputControllerBinding bindings[][MAX_CONTROLLERTYPES])
{
    // Translate shift and preserve the original menu short circuit
    int32_t i;
    bool processdown = false;
    InputConfig_TranslateShift();
    if (!Input_mapPadOneToPadTwo && !InputConfig_MenuActive())
        processdown = true;
    for (i = 0; i < MAX_CONTROLLERACTIONS; i++) {
        // Read both live bindings and preserve the original repeated queries
        int32_t key = bindings[i][KEYBOARD].m_Key;
        int32_t extrakey;
        if (Input_GetIsKeyboardKeyDown(key) && processdown)
            InputConfig_ButtonDown(key, KEYBOARD);
        extrakey = bindings[i][OPTIONAL_EXTRA].m_Key;
        if (Input_GetIsKeyboardKeyDown(extrakey) && processdown)
            InputConfig_ButtonDown(extrakey, OPTIONAL_EXTRA);
        if (!Input_GetIsKeyboardKeyDown(key))
            InputConfig_ButtonUp(key, KEYBOARD);
        else if (!Input_GetIsKeyboardKeyDown(extrakey))
            InputConfig_ButtonUp(key, OPTIONAL_EXTRA);
    }
}

void Input_AffectPadFromMouse(const InputControllerBinding bindings[][MAX_CONTROLLERTYPES])
{
    // Preserve the original menu short circuit before scanning bindings
    int32_t i;
    bool processdown = false;
    if (!Input_mapPadOneToPadTwo && !InputConfig_MenuActive())
        processdown = true;
    for (i = 0; i < MAX_CONTROLLERACTIONS; i++) {
        // Read each live binding before the down and up checks
        int32_t button = bindings[i][MOUSE].m_Key;
        if (Input_GetIsMouseButtonDown(button) && processdown)
            InputConfig_ButtonDown(button, MOUSE);
        if (Input_GetIsMouseButtonUp(button))
            InputConfig_ButtonUp(button, MOUSE);
    }
}

bool Input_GetIsMouseButtonDown(int32_t keycode)
{
	// Preserve the original mouse button code checks
	switch (keycode)
	{
	case rsMOUSELEFTBUTTON:
		if (Input_GetLeftMouse()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsMOUSMIDDLEBUTTON:
		if (Input_GetMiddleMouse()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsMOUSERIGHTBUTTON:
		if (Input_GetRightMouse()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsMOUSEWHEELUPBUTTON:
		if (Input_GetMouseWheelUp()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsMOUSEWHEELDOWNBUTTON:
		if (Input_GetMouseWheelDown()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsMOUSEX1BUTTON:
		if (Input_GetMouseX1()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsMOUSEX2BUTTON:
		if (Input_GetMouseX2()) // rouz edit (ChatGPT)
			return true;
		break;
	default: break;
	}

	return false;
}

bool Input_GetIsMouseButtonUp(int32_t keycode)
{
	// Preserve the original mouse button code checks
	switch (keycode)
	{
	case rsMOUSELEFTBUTTON:
		if (Input_GetLeftMouseUp()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsMOUSMIDDLEBUTTON:
		if (Input_GetMiddleMouseUp()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsMOUSERIGHTBUTTON:
		if (Input_GetRightMouseUp()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsMOUSEWHEELUPBUTTON:
		if (Input_GetMouseWheelUpUp()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsMOUSEWHEELDOWNBUTTON:
		if (Input_GetMouseWheelDownUp()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsMOUSEX1BUTTON:
		if (Input_GetMouseX1Up()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsMOUSEX2BUTTON:
		if (Input_GetMouseX2Up()) // rouz edit (ChatGPT)
			return true;
		break;
	default: break;
	}

	return false;
}

bool Input_GetIsKeyboardKeyDown(int32_t keycode)
{
	// Preserve the original ordered keyboard code checks
	if (keycode < 255)
	{
		if (Input_GetChar(keycode)) // rouz edit (ChatGPT)
			return true;
	}

	for (int32_t i = 0; i < 12; i++)
	{
		if (i + rsF1 == keycode)
		{
			if (Input_GetF(i)) // rouz edit (ChatGPT)
				return true;
		}
	}

	switch (keycode)
	{
	case rsESC:
		if (Input_GetEscape()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsINS:
		if (Input_GetInsert()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsDEL:
		if (Input_GetDelete()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsHOME:
		if (Input_GetHome()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsEND:
		if (Input_GetEnd()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsPGUP:
		if (Input_GetPageUp()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsPGDN:
		if (Input_GetPageDown()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsUP:
		if (Input_GetUp()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsDOWN:
		if (Input_GetDown()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsLEFT:
		if (Input_GetLeft()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsRIGHT:
		if (Input_GetRight()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsSCROLL:
		if (Input_GetScrollLock()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsPAUSE:
		if (Input_GetPause()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsNUMLOCK:
		if (Input_GetNumLock()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsDIVIDE:
		if (Input_GetDivide()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsTIMES:
		if (Input_GetTimes()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsMINUS:
		if (Input_GetMinus()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsPLUS:
		if (Input_GetPlus()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsPADENTER:
		if (Input_GetPadEnter()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsPADDEL:
		if (Input_GetPadDel()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsPADEND:
		if (Input_GetPad1()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsPADDOWN:
		if (Input_GetPad2()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsPADPGDN:
		if (Input_GetPad3()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsPADLEFT:
		if (Input_GetPad4()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsPAD5:
		if (Input_GetPad5()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsPADRIGHT:
		if (Input_GetPad6()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsPADHOME:
		if (Input_GetPad7()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsPADUP:
		if (Input_GetPad8()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsPADPGUP:
		if (Input_GetPad9()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsPADINS:
		if (Input_GetPad0()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsBACKSP:
		if (Input_GetBackspace()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsTAB:
		if (Input_GetTab()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsCAPSLK:
		if (Input_GetCapsLock()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsENTER:
		if (Input_GetEnter()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsLSHIFT:
		if (Input_GetLeftShift()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsSHIFT:
		if (Input_GetShift()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsRSHIFT:
		if (Input_GetRightShift()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsLCTRL:
		if (Input_GetLeftCtrl()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsRCTRL:
		if (Input_GetRightCtrl()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsLALT:
		if (Input_GetLeftAlt()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsRALT:
		if (Input_GetRightAlt()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsLWIN:
		if (Input_GetLeftWin()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsRWIN:
		if (Input_GetRightWin()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsAPPS:
		if (Input_GetApps()) // rouz edit (ChatGPT)
			return true;
		break;
	default: break;
	}

	return false;
}

bool Input_GetIsKeyboardKeyJustDown(int32_t keycode)
{
	// Preserve the original ordered keyboard code checks
	if (keycode < 255)
	{
		if (Input_GetCharJustDown(keycode)) // rouz edit (ChatGPT)
			return true;
	}

	for (int32_t i = 0; i < 12; i++)
	{
		if (i + rsF1 == keycode)
		{
			if (Input_GetFJustDown(i)) // rouz edit (ChatGPT)
				return true;
		}
	}

	switch (keycode)
	{
	case rsESC:
		if (Input_GetEscapeJustDown()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsINS:
		if (Input_GetInsertJustDown()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsDEL:
		if (Input_GetDeleteJustDown()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsHOME:
		if (Input_GetHomeJustDown()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsEND:
		if (Input_GetEndJustDown()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsPGUP:
		if (Input_GetPageUpJustDown()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsPGDN:
		if (Input_GetPageDownJustDown()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsUP:
		if (Input_GetUpJustDown()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsDOWN:
		if (Input_GetDownJustDown()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsLEFT:
		if (Input_GetLeftJustDown()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsRIGHT:
		if (Input_GetRightJustDown()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsSCROLL:
		if (Input_GetScrollLockJustDown()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsPAUSE:
		if (Input_GetPauseJustDown()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsNUMLOCK:
		if (Input_GetNumLockJustDown()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsDIVIDE:
		if (Input_GetDivideJustDown()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsTIMES:
		if (Input_GetTimesJustDown()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsMINUS:
		if (Input_GetMinusJustDown()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsPLUS:
		if (Input_GetPlusJustDown()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsPADENTER:
		if (Input_GetPadEnterJustDown()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsPADDEL:
		if (Input_GetPadDelJustDown()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsPADEND:
		if (Input_GetPad1JustDown()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsPADDOWN:
		if (Input_GetPad2JustDown()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsPADPGDN:
		if (Input_GetPad3JustDown()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsPADLEFT:
		if (Input_GetPad4JustDown()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsPAD5:
		if (Input_GetPad5JustDown()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsPADRIGHT:
		if (Input_GetPad6JustDown()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsPADHOME:
		if (Input_GetPad7JustDown()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsPADUP:
		if (Input_GetPad8JustDown()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsPADPGUP:
		if (Input_GetPad9JustDown()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsPADINS:
		if (Input_GetPad0JustDown()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsBACKSP:
		if (Input_GetBackspaceJustDown()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsTAB:
		if (Input_GetTabJustDown()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsCAPSLK:
		if (Input_GetCapsLockJustDown()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsENTER:
		if (Input_GetReturnJustDown()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsLSHIFT:
		if (Input_GetLeftShiftJustDown()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsSHIFT:
		if (Input_GetShiftJustDown()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsRSHIFT:
		if (Input_GetRightShiftJustDown()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsLCTRL:
		if (Input_GetLeftCtrlJustDown()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsRCTRL:
		if (Input_GetRightCtrlJustDown()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsLALT:
		if (Input_GetLeftAltJustDown()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsRALT:
		if (Input_GetRightAltJustDown()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsLWIN:
		if (Input_GetLeftWinJustDown()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsRWIN:
		if (Input_GetRightWinJustDown()) // rouz edit (ChatGPT)
			return true;
		break;
	case rsAPPS:
		if (Input_GetAppsJustDown()) // rouz edit (ChatGPT)
			return true;
		break;
	default: break;
	}

	return false;
}

void Input_HandleKeyDown(int32_t c, void (*cheat)(void *, int32_t), void *context)
{
    // Preserve the original key-down switch and cheat callbacks
		switch (c)
		{
			case rsESC:
				{
					Input_tempKeyState.ESC = 255; // rouz edit (ChatGPT)
					break;
				}
			
			case rsINS:
				{
					Input_tempKeyState.INS = 255; // rouz edit (ChatGPT)
					break;
				}
				
			case rsDEL:
				{
					Input_tempKeyState.DEL = 255; // rouz edit (ChatGPT)
					break;
				}
				
			case rsHOME:
				{
					Input_tempKeyState.HOME = 255; // rouz edit (ChatGPT)
					break;
				}
				
			case rsEND:
				{
					Input_tempKeyState.END = 255; // rouz edit (ChatGPT)
					break;
				}
				
			case rsPGUP:
				{
					Input_tempKeyState.PGUP = 255; // rouz edit (ChatGPT)
					break;
				}
				
			case rsPGDN:
				{
					Input_tempKeyState.PGDN = 255; // rouz edit (ChatGPT)
					break;
				}
				
			case rsUP:
				{
					Input_tempKeyState.UP = 255; // rouz edit (ChatGPT)
					break;
				}
				
			case rsDOWN:
				{
					Input_tempKeyState.DOWN = 255; // rouz edit (ChatGPT)
					break;
				}
				
			case rsLEFT:
				{
					Input_tempKeyState.LEFT = 255; // rouz edit (ChatGPT)
					break;
				}
				
			case rsRIGHT:
				{
					Input_tempKeyState.RIGHT = 255; // rouz edit (ChatGPT)
					break;
				}
				
			case rsNUMLOCK:
				{
					Input_tempKeyState.NUMLOCK = 255; // rouz edit (ChatGPT)
					break;
				}
				
			case rsPADDEL:
				{
					Input_tempKeyState.DECIMAL = 255; // rouz edit (ChatGPT)
					break;
				}
				
			case rsPADEND:
				{
					Input_tempKeyState.NUM1 = 255; // rouz edit (ChatGPT)
					break;
				}
				
			case rsPADDOWN:
				{
					Input_tempKeyState.NUM2 = 255; // rouz edit (ChatGPT)
					break;
				}
				
			case rsPADPGDN:
				{
					Input_tempKeyState.NUM3 = 255; // rouz edit (ChatGPT)
					break;
				}
				
			case rsPADLEFT:
				{
					Input_tempKeyState.NUM4 = 255; // rouz edit (ChatGPT)
					break;
				}
				
			case rsPAD5:
				{
					Input_tempKeyState.NUM5 = 255; // rouz edit (ChatGPT)
					break;
				}
				
			case rsPADRIGHT:
				{
					Input_tempKeyState.NUM6 = 255; // rouz edit (ChatGPT)
					break;
				}
				
			case rsPADHOME:
				{
					Input_tempKeyState.NUM7 = 255; // rouz edit (ChatGPT)
					break;
				}
				
			case rsPADUP:
				{
					Input_tempKeyState.NUM8 = 255; // rouz edit (ChatGPT)
					break;
				}
				
			case rsPADPGUP:
				{
					Input_tempKeyState.NUM9 = 255; // rouz edit (ChatGPT)
					break;
				}
				
			case rsPADINS:
				{
					Input_tempKeyState.NUM0 = 255; // rouz edit (ChatGPT)
					break;
				}
				
			case rsDIVIDE:
				{
					Input_tempKeyState.DIV = 255; // rouz edit (ChatGPT)
					break;
				}
				
			case rsTIMES:
				{
					Input_tempKeyState.MUL = 255; // rouz edit (ChatGPT)
					break;
				}
				
			case rsMINUS:
				{
					Input_tempKeyState.SUB = 255; // rouz edit (ChatGPT)
					break;
				}
				
			case rsPADENTER:
				{
					Input_tempKeyState.ENTER = 255; // rouz edit (ChatGPT)
					break;
				}
				
			case rsPLUS:
				{
					Input_tempKeyState.ADD = 255; // rouz edit (ChatGPT)
					cheat(context, '+');	// rouz edit to make + sent to the cheats function
					break;
				}
				
			case rsENTER:
				{
					Input_tempKeyState.EXTENTER = 255; // rouz edit (ChatGPT)
					break;
				}
				
			case rsSCROLL:
				{
					Input_tempKeyState.SCROLLLOCK = 255; // rouz edit (ChatGPT)
					break;
				}
				
			case rsPAUSE:
				{
					Input_tempKeyState.PAUSE = 255; // rouz edit (ChatGPT)
					break;
				}
				
			case rsBACKSP:
				{
					Input_tempKeyState.BACKSP = 255; // rouz edit (ChatGPT)
					break;
				}
				
			case rsTAB:
				{
					Input_tempKeyState.TAB = 255; // rouz edit (ChatGPT)
					break;
				}
				
			case rsCAPSLK:
				{
					Input_tempKeyState.CAPSLOCK = 255; // rouz edit (ChatGPT)
					break;
				}
				
			case rsLSHIFT:
				{
					Input_tempKeyState.LSHIFT = 255; // rouz edit (ChatGPT)
					break;
				}
				
			case rsSHIFT:
				{
					Input_tempKeyState.SHIFT = 255; // rouz edit (ChatGPT)
					break;
				}
				
			case rsRSHIFT:
				{
					Input_tempKeyState.RSHIFT = 255; // rouz edit (ChatGPT)
					break;
				}
				
			case rsLCTRL:
				{
					Input_tempKeyState.LCTRL = 255; // rouz edit (ChatGPT)
					break;
				}
				
			case rsRCTRL:
				{
					Input_tempKeyState.RCTRL = 255; // rouz edit (ChatGPT)
					break;
				}
				
			case rsLALT:
				{
					Input_tempKeyState.LALT = 255; // rouz edit (ChatGPT)
					break;
				}
				
			case rsRALT:
				{
					Input_tempKeyState.RALT = 255; // rouz edit (ChatGPT)
					break;
				}
				
				
			case rsLWIN:
				{
					Input_tempKeyState.LWIN = 255; // rouz edit (ChatGPT)
					break;
				}
				
			case rsRWIN:
				{
					Input_tempKeyState.RWIN = 255; // rouz edit (ChatGPT)
					break;
				}
				
			case rsAPPS:
				{
					Input_tempKeyState.APPS = 255; // rouz edit (ChatGPT)
					break;
				}
				
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
					Input_tempKeyState.F[c - rsF1] = 255; // rouz edit (ChatGPT)
					break;
				}
				
			default:
				{
					if ( c < 255 )
					{
						Input_tempKeyState.VK_KEYS[c] = 255; // rouz edit (ChatGPT)
						cheat(context, c);
					}
					break;
				}
		}
		
}

void Input_HandleKeyUp(int32_t c)
{
    // Preserve the original key-up switch
		switch (c)
		{
			case rsESC:
				{
					Input_tempKeyState.ESC = 0; // rouz edit (ChatGPT)
					break;
				}
			
			case rsINS:
				{
					Input_tempKeyState.INS = 0; // rouz edit (ChatGPT)
					break;
				}
				
			case rsDEL:
				{
					Input_tempKeyState.DEL = 0; // rouz edit (ChatGPT)
					break;
				}
				
			case rsHOME:
				{
					Input_tempKeyState.HOME = 0; // rouz edit (ChatGPT)
					break;
				}
				
			case rsEND:
				{
					Input_tempKeyState.END = 0; // rouz edit (ChatGPT)
					break;
				}
				
			case rsPGUP:
				{
					Input_tempKeyState.PGUP = 0; // rouz edit (ChatGPT)
					break;
				}
				
			case rsPGDN:
				{
					Input_tempKeyState.PGDN = 0; // rouz edit (ChatGPT)
					break;
				}
				
			case rsUP:
				{
					Input_tempKeyState.UP = 0; // rouz edit (ChatGPT)
					break;
				}
				
			case rsDOWN:
				{
					Input_tempKeyState.DOWN = 0; // rouz edit (ChatGPT)
					break;
				}
				
			case rsLEFT:
				{
					Input_tempKeyState.LEFT = 0; // rouz edit (ChatGPT)
					break;
				}
				
			case rsRIGHT:
				{
					Input_tempKeyState.RIGHT = 0; // rouz edit (ChatGPT)
					break;
				}
				
			case rsNUMLOCK:
				{
					Input_tempKeyState.NUMLOCK = 0; // rouz edit (ChatGPT)
					break;
				}
				
			case rsPADDEL:
				{
					Input_tempKeyState.DECIMAL = 0; // rouz edit (ChatGPT)
					break;
				}
				
			case rsPADEND:
				{
					Input_tempKeyState.NUM1 = 0; // rouz edit (ChatGPT)
					break;
				}
				
			case rsPADDOWN:
				{
					Input_tempKeyState.NUM2 = 0; // rouz edit (ChatGPT)
					break;
				}
				
			case rsPADPGDN:
				{
					Input_tempKeyState.NUM3 = 0; // rouz edit (ChatGPT)
					break;
				}
				
			case rsPADLEFT:
				{
					Input_tempKeyState.NUM4 = 0; // rouz edit (ChatGPT)
					break;
				}
				
			case rsPAD5:
				{
					Input_tempKeyState.NUM5 = 0; // rouz edit (ChatGPT)
					break;
				}
				
			case rsPADRIGHT:
				{
					Input_tempKeyState.NUM6 = 0; // rouz edit (ChatGPT)
					break;
				}
				
			case rsPADHOME:
				{
					Input_tempKeyState.NUM7 = 0; // rouz edit (ChatGPT)
					break;
				}
				
			case rsPADUP:
				{
					Input_tempKeyState.NUM8 = 0; // rouz edit (ChatGPT)
					break;
				}
				
			case rsPADPGUP:
				{
					Input_tempKeyState.NUM9 = 0; // rouz edit (ChatGPT)
					break;
				}
				
			case rsPADINS:
				{
					Input_tempKeyState.NUM0 = 0; // rouz edit (ChatGPT)
					break;
				}
				
			case rsDIVIDE:
				{
					Input_tempKeyState.DIV = 0; // rouz edit (ChatGPT)
					break;
				}
				
			case rsTIMES:
				{
					Input_tempKeyState.MUL = 0; // rouz edit (ChatGPT)
					break;
				}
				
			case rsMINUS:
				{
					Input_tempKeyState.SUB = 0; // rouz edit (ChatGPT)
					break;
				}
				
			case rsPADENTER:
				{
					Input_tempKeyState.ENTER = 0; // rouz edit (ChatGPT)
					break;
				}
				
			case rsPLUS:
				{
					Input_tempKeyState.ADD = 0; // rouz edit (ChatGPT)
					break;
				}
				
			case rsENTER:
				{
					Input_tempKeyState.EXTENTER = 0; // rouz edit (ChatGPT)
					break;
				}
				
			case rsSCROLL:
				{
					Input_tempKeyState.SCROLLLOCK = 0; // rouz edit (ChatGPT)
					break;
				}
				
			case rsPAUSE:
				{
					Input_tempKeyState.PAUSE = 0; // rouz edit (ChatGPT)
					break;
				}
				
			case rsBACKSP:
				{
					Input_tempKeyState.BACKSP = 0; // rouz edit (ChatGPT)
					break;
				}
				
			case rsTAB:
				{
					Input_tempKeyState.TAB = 0; // rouz edit (ChatGPT)
					break;
				}
				
			case rsCAPSLK:
				{
					Input_tempKeyState.CAPSLOCK = 0; // rouz edit (ChatGPT)
					break;
				}
				
			case rsLSHIFT:
				{
					Input_tempKeyState.LSHIFT = 0; // rouz edit (ChatGPT)
					break;
				}
				
			case rsSHIFT:
				{
					Input_tempKeyState.SHIFT = 0; // rouz edit (ChatGPT)
					break;
				}
				
			case rsRSHIFT:
				{
					Input_tempKeyState.RSHIFT = 0; // rouz edit (ChatGPT)
					break;
				}
				
			case rsLCTRL:
				{
					Input_tempKeyState.LCTRL = 0; // rouz edit (ChatGPT)
					break;
				}
				
			case rsRCTRL:
				{
					Input_tempKeyState.RCTRL = 0; // rouz edit (ChatGPT)
					break;
				}
				
			case rsLALT:
				{
					Input_tempKeyState.LALT = 0; // rouz edit (ChatGPT)
					break;
				}
				
			case rsRALT:
				{
					Input_tempKeyState.RALT = 0; // rouz edit (ChatGPT)
					break;
				}
				
				
			case rsLWIN:
				{
					Input_tempKeyState.LWIN = 0; // rouz edit (ChatGPT)
					break;
				}
				
			case rsRWIN:
				{
					Input_tempKeyState.RWIN = 0; // rouz edit (ChatGPT)
					break;
				}
				
			case rsAPPS:
				{
					Input_tempKeyState.APPS = 0; // rouz edit (ChatGPT)
					break;
				}
				
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
					Input_tempKeyState.F[c - rsF1] = 0; // rouz edit (ChatGPT)
					break;
				}
				
			default:
				{
					if ( c < 255 )
					{
						Input_tempKeyState.VK_KEYS[c] = 0; // rouz edit (ChatGPT)
					}
					break;
				}
		}
		
}

CControllerState Input_ReconcileControllers(const CControllerState *State1, const CControllerState *State2)
{
	// Preserve the original shared scratch state and reconciliation order
	static CControllerState ReconState;

	ControllerState_Clear(&ReconState);

#define _RECONCILE_BUTTON(button) \
	{ if ( State1->button || State2->button ) ReconState.button = 255; }

#define _RECONCILE_AXIS_POSITIVE(axis) \
	{ if ( State1->axis >= 0 && State2->axis >= 0 ) ReconState.axis = ((State1->axis) > (State2->axis) ? (State1->axis) : (State2->axis)); }

#define _RECONCILE_AXIS_NEGATIVE(axis) \
	{ if ( State1->axis <= 0 && State2->axis <= 0 ) ReconState.axis = ((State1->axis) < (State2->axis) ? (State1->axis) : (State2->axis)); }

#define _RECONCILE_AXIS(axis) \
	{ _RECONCILE_AXIS_POSITIVE(axis); _RECONCILE_AXIS_NEGATIVE(axis); }

#define _FIX_AXIS_DIR(axis) \
	{ if ( State1->axis > 0 && State2->axis < 0 || State1->axis < 0 && State2->axis > 0 ) ReconState.axis = 0; }

#define _FIX_RECON_DIR(pos, neg, axis) \
	{ if ( (ReconState.pos || ReconState.axis < 0) && (ReconState.neg || ReconState.axis > 0) ) { ReconState.pos = 0; ReconState.neg = 0; ReconState.axis = 0; } }

	_RECONCILE_BUTTON(LeftShoulder1);
	_RECONCILE_BUTTON(LeftShoulder2);
	_RECONCILE_BUTTON(RightShoulder1);
	_RECONCILE_BUTTON(RightShoulder2);
	_RECONCILE_BUTTON(Start);
	_RECONCILE_BUTTON(Select);
	_RECONCILE_BUTTON(Square);
	_RECONCILE_BUTTON(Triangle);
	_RECONCILE_BUTTON(Cross);
	_RECONCILE_BUTTON(Circle);
	_RECONCILE_BUTTON(LeftShock);
	_RECONCILE_BUTTON(RightShock);
	_RECONCILE_BUTTON(NetworkTalk);
	_RECONCILE_AXIS(LeftStickX);
	_RECONCILE_AXIS(LeftStickY);
	_FIX_AXIS_DIR(LeftStickX);
	_FIX_AXIS_DIR(LeftStickY);
	_RECONCILE_AXIS(RightStickX);
	_RECONCILE_AXIS(RightStickY);
	_FIX_AXIS_DIR(RightStickX);
	_FIX_AXIS_DIR(RightStickY);
	_RECONCILE_BUTTON(DPadUp);
	_RECONCILE_BUTTON(DPadDown);
	_RECONCILE_BUTTON(DPadLeft);
	_RECONCILE_BUTTON(DPadRight);
	_FIX_RECON_DIR(DPadUp, DPadDown, LeftStickY);
	_FIX_RECON_DIR(DPadLeft, DPadRight, LeftStickX);

	return ReconState;

#undef _RECONCILE_BUTTON
#undef _RECONCILE_AXIS_POSITIVE
#undef _RECONCILE_AXIS_NEGATIVE
#undef _RECONCILE_AXIS
#undef _FIX_AXIS_DIR
#undef _FIX_RECON_DIR
}


bool
ControllerState_CheckForInput(CControllerState *self)
{
	// Preserve the original field updates and intentionally untouched members
	return !!self->RightStickX || !!self->RightStickY || !!self->LeftStickX || !!self->LeftStickY
		|| !!self->DPadUp || !!self->DPadDown || !!self->DPadLeft || !!self->DPadRight
		|| !!self->Triangle || !!self->Cross || !!self->Circle || !!self->Square
		|| !!self->Start || !!self->Select
		|| !!self->LeftShoulder1 || !!self->LeftShoulder2 || !!self->RightShoulder1 || !!self->RightShoulder2
		|| !!self->LeftShock || !!self->RightShock;
}

void
ControllerState_Clear(CControllerState *self)
{
	// Preserve the original field updates and intentionally untouched members
	self->LeftStickX = self->LeftStickY = self->RightStickX = self->RightStickY = 0;
	self->LeftShoulder1 = self->LeftShoulder2 = self->RightShoulder1 = self->RightShoulder2 = 0;
	self->DPadUp = self->DPadDown = self->DPadLeft = self->DPadRight = 0;
	self->Start = self->Select = 0;
	self->Square = self->Triangle = self->Cross = self->Circle = 0;
	self->LeftShock = self->RightShock = 0;
	self->NetworkTalk = 0;
}

void Input_StartShake(int16_t *duration, uint8_t *frequency, int16_t requestedDuration, uint8_t requestedFrequency)
{
    // Preserve the original zero-frequency reset and duration comparison
    if (requestedFrequency == 0) {
        // Stop vibration regardless of the requested duration
        *duration = 0;
        *frequency = 0;
        return;
    }
    if (requestedDuration > *duration) {
        // Replace only a shorter active vibration
        *duration = requestedDuration;
        *frequency = requestedFrequency;
    }
}

void Input_StartTrainShake(int16_t *duration, uint8_t *frequency, float distance)
{
    // Preserve the original distance gate and frequency calculation
    if (distance < 70.0f) {
        // Update only when the existing shake is shorter than the train shake
        int32_t freq = (int32_t)((70.0f - distance) * 70.0f / 70.0f + 30.0f);
        if (*duration < 100) {
            // Store the same duration and narrowed frequency
            *duration = 100;
            *frequency = freq;
        }
    }
}

void Input_AdvanceKeyboardHistory(void)
{
    // Copy the live keyboard records in their original order
    Input_oldKeyState = Input_newKeyState;
    Input_newKeyState = Input_tempKeyState;
}

void Input_AdvanceMouseHistory(void)
{
    // Copy the live mouse records in their original order
    Input_oldMouseState = Input_newMouseState;
    Input_newMouseState = Input_tempMouseState;
}

void Input_BeginGlfwMouseSample(double x, double y, double previousX, double previousY, bool menuActive)
{
    // Preserve the original menu-dependent axis inversion
    int32_t signX = 1, signY = 1;
    if (!menuActive) {
        // Apply each configured axis inversion independently
        if (MousePointerStateHelper.bInvertVertically)
            signY = -1;
        if (MousePointerStateHelper.bInvertHorizontally)
            signX = -1;
    }
    // Clear buttons before storing cursor deltas with the original precision
    MouseControllerState_Clear(&Input_tempMouseState);
    Input_tempMouseState.x = (float)(signX * (x - previousX));
    Input_tempMouseState.y = (float)(signY * (y - previousY));
}

void Input_SetMouseWheel(double wheel)
{
    // Preserve the original positive and negative wheel tests
    if (wheel > 0)
        Input_tempMouseState.WHEELUP = 1;
    else if (wheel < 0)
        Input_tempMouseState.WHEELDN = 1;
}

void Input_ApplyDirectMouseSample(int32_t x, int32_t y, int32_t wheel, const uint8_t *buttons, bool menuActive)
{
    // Preserve the original menu-dependent axis inversion
    int32_t signX = 1, signY = 1;
    if (!menuActive) {
        // Apply each configured axis inversion independently
        if (MousePointerStateHelper.bInvertVertically)
            signY = -1;
        if (MousePointerStateHelper.bInvertHorizontally)
            signX = -1;
    }
    // Clear the temporary buttons and copy the direct input sample
    MouseControllerState_Clear(&Input_tempMouseState);
    Input_tempMouseState.x = (float)(signX * x);
    Input_tempMouseState.y = (float)(signY * y);
    Input_tempMouseState.LMB = buttons[0] & 128;
    Input_tempMouseState.RMB = buttons[1] & 128;
    Input_tempMouseState.MMB = buttons[2] & 128;
    Input_tempMouseState.MXB1 = buttons[3] & 128;
    Input_tempMouseState.MXB2 = buttons[4] & 128;
    if (wheel > 0)
        Input_tempMouseState.WHEELUP = 1;
    else if (wheel < 0)
        Input_tempMouseState.WHEELDN = 1;
    Input_AdvanceMouseHistory();
}

void Input_ClearKeyboardHistory(void)
{
    // Clear the keyboard records in the original order
    KeyboardState_Clear(&Input_newKeyState);
    KeyboardState_Clear(&Input_oldKeyState);
    KeyboardState_Clear(&Input_tempKeyState);
}

void Input_ClearMouseHistory(void)
{
    // Clear mouse button history in the original order without resetting motion
    MouseControllerState_Clear(&Input_tempMouseState);
    MouseControllerState_Clear(&Input_newMouseState);
    MouseControllerState_Clear(&Input_oldMouseState);
}

void
KeyboardState_Clear(CKeyboardState *self)
{
	// Preserve the original field updates and intentionally untouched members
	for ( int32_t i = 0; i < ARRAY_SIZE(self->F); i++	)
		self->F[i] = 0;

	for ( int32_t i = 0; i < ARRAY_SIZE(self->VK_KEYS); i++	 )
		self->VK_KEYS[i] = 0;

	self->ESC = self->INS = self->DEL = self->HOME = self->END = self->PGUP = self->PGDN = 0;

	self->UP = self->DOWN = self->LEFT = self->RIGHT = 0;

	self->NUMLOCK = 0;

	self->DIV = self->MUL = self->SUB = self->ADD = 0;

	self->DECIMAL = self->NUM1 = self->NUM2 = self->NUM3 = self->NUM4 = 0;

	self->NUM5 = self->NUM6 = self->NUM7 = self->NUM8 = 0;

	self->NUM9 = self->NUM0 = self->SCROLLLOCK = self->PAUSE = 0;

	self->BACKSP = self->TAB = self->CAPSLOCK = self->EXTENTER = 0;

	self->LSHIFT = self->SHIFT = self->RSHIFT = self->LCTRL = self->RCTRL = self->LALT = self->RALT = 0;

	self->LWIN = self->RWIN = self->APPS = 0;
}

void
MouseControllerState_Init(CMouseControllerState *self)
{
	// Preserve the original field updates and intentionally untouched members
	self->LMB = 0;
	self->RMB = 0;
	self->MMB = 0;
	self->WHEELUP = 0;
	self->WHEELDN = 0;
	self->MXB1 = 0;
	self->MXB2 = 0;

	self->x = 0.0f;
	self->y = 0.0f;
}

void
MouseControllerState_Clear(CMouseControllerState *self)
{
	// Preserve the original field updates and intentionally untouched members
	self->LMB = 0;
	self->RMB = 0;
	self->MMB = 0;
	self->WHEELUP = 0;
	self->WHEELDN = 0;
	self->MXB1 = 0;
	self->MXB2 = 0;
}

CMouseControllerState
MousePointerState_GetMouseSetUp(CMousePointerStateHelper *self)
{
	// Preserve the complete platform-dependent mouse capability selection
	(void)self;
	CMouseControllerState state;
	// Initialize the automatic record at the former constructor boundary
	MouseControllerState_Init(&state);

#if defined RW_D3D9 || defined RWLIBS
	if ( !InputState_BackendHasMouse() )
		InputState_BackendInitialiseMouse(!InputState_BackendMenuActive() && InputState_BackendNeedsExclusive());

	if ( InputState_BackendHasMouse() )
	{
		// Select the supported buttons and wheel from the queried capabilities
		uint32_t buttons,axes;
		InputState_BackendCapabilities(&buttons,&axes);
		switch ( buttons )
		{
			// Preserve the original capability cases and fallthrough behavior
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
				state.MMB = true;

			case 2:
				state.RMB = true;

			case 1:
				state.LMB = true;
		}

		if ( axes == 3 )
		{
			// Preserve the original capability cases and fallthrough behavior
			state.WHEELDN = true;
			state.WHEELUP = true;
		}
	}
#else
	// It seems there is no way to get number of buttons on mouse, so assign all buttons if we have mouse.
	double xpos = InputState_BackendCursorX();

	if (xpos != 0.f) {
		// Expose the same buttons whenever the cursor query reports a nonzero X value
		state.MMB = true;
		state.RMB = true;
		state.LMB = true;
		state.WHEELDN = true;
		state.WHEELUP = true;
	}
#endif

	return state;
}

float
ControllerState_GetLeftStickX(CControllerState *self)
{
    // Retain the original signed axis normalization including the negative endpoint
    return self->LeftStickX/32767.0f;
}

float
ControllerState_GetLeftStickY(CControllerState *self)
{
    // Retain the original signed axis normalization including the negative endpoint
    return self->LeftStickY/32767.0f;
}

float
ControllerState_GetRightStickX(CControllerState *self)
{
    // Retain the original signed axis normalization including the negative endpoint
    return self->RightStickX/32767.0f;
}

float
ControllerState_GetRightStickY(CControllerState *self)
{
    // Retain the original signed axis normalization including the negative endpoint
    return self->RightStickY/32767.0f;
}

//- rouz edit (ChatGPT)
