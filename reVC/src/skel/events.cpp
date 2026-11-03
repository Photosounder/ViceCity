#include "common.h"
#include "Pad.h"
#include "ControllerConfig.h"
#include "Frontend.h"
#include "Camera.h"

#include "rwcore.h"
#include "skeleton.h"
#include "events.h"


/*
 *****************************************************************************
 */
//+ rouz edit (ChatGPT)
static void InputEvents_AddCheat(void *context, int32_t character)
{
    // Preserve the original pad cheat input call
    ((CPad *)context)->AddToPCCheatString(character);
}
//- rouz edit (ChatGPT)

static RsEventStatus 
HandleKeyDown(RsKeyStatus *keyStatus)
{
	CPad *pad0 = CPad::GetPad(0);
	CPad *pad1 = CPad::GetPad(1);
	
	RwInt32 c = keyStatus->keyCharCode;
	
	if ( c != rsNULL )
	{
		Input_HandleKeyDown(c, InputEvents_AddCheat, pad0); // rouz edit (ChatGPT)
	

		if ( Input_mapPadOneToPadTwo ) // rouz edit (ChatGPT)
			Input_MapPadKeyDown(&pad1->PCTempKeyState, c); // rouz edit (ChatGPT)
	}
	
	return rsEVENTPROCESSED;
}


static RsEventStatus 
HandleKeyUp(RsKeyStatus *keyStatus)
{
	CPad *pad0 = CPad::GetPad(0);
	CPad *pad1 = CPad::GetPad(1);
	
	RwInt32 c = keyStatus->keyCharCode;
	
	if ( c != rsNULL )
	{
		Input_HandleKeyUp(c); // rouz edit (ChatGPT)
	

		if ( Input_mapPadOneToPadTwo ) // rouz edit (ChatGPT)
			Input_MapPadKeyUp(&pad1->PCTempKeyState, c); // rouz edit (ChatGPT)
	}
	
	return rsEVENTPROCESSED;
}


/*
 *****************************************************************************
 */
static RsEventStatus 
KeyboardHandler(RsEvent event, void *param)
{
	/*
	 * ...then the application events, if necessary...
	 */
	switch( event )
	{
		case rsKEYDOWN:
		{
			return HandleKeyDown((RsKeyStatus *)param);
		}

		case rsKEYUP:
		{
			return HandleKeyUp((RsKeyStatus *)param);
		}

		default:
		{
			return rsEVENTNOTPROCESSED;
		}
	}
}

/*
 *****************************************************************************
 */
//+ rouz edit (ChatGPT)
void InputEvents_UpdateJoyButtons(int32_t padNumber)
{
    // Update the original live controller button array
    ControlsManager.UpdateJoyButtonState(padNumber);
}
bool InputEvents_MenuActive(void)
{
    // Read the live menu flag for each button
    return FrontEndMenuManager.m_bMenuActive;
}
void InputEvents_DispatchJoyButton(int32_t button, int32_t padNumber, bool menu, bool release)
{
    // Invoke the original menu or gameplay operation
    if (release) {
        // Preserve the release dispatch targets
        if (menu)
            ControlsManager.UpdateJoyInConfigMenus_ButtonUp(button, padNumber);
        else
            ControlsManager.AffectControllerStateOn_ButtonUp(button, JOYSTICK);
    } else {
        // Preserve the press dispatch targets
        if (menu)
            ControlsManager.UpdateJoyInConfigMenus_ButtonDown(button, padNumber);
        else
            ControlsManager.AffectControllerStateOn_ButtonDown(button, JOYSTICK);
    }
}
//- rouz edit (ChatGPT)

static RsEventStatus
HandlePadButtonDown(RsPadButtonStatus *padButtonStatus)
{
	bool bPadTwo = false;
	int32 padNumber = padButtonStatus->padID;
	
	CPad *pad = CPad::GetPad(padNumber);
	
	if ( Input_mapPadOneToPadTwo ) // rouz edit (ChatGPT)
		padNumber = 1;
	
	if ( padNumber == 1 )
		bPadTwo = true;
	
	Input_DispatchPadButtons(padNumber, bPadTwo, false, false, ControlsManager.m_aButtonStates); // rouz edit (ChatGPT)

	return rsEVENTPROCESSED;
}


/*
 *****************************************************************************
 */
static RsEventStatus
HandlePadButtonUp(RsPadButtonStatus *padButtonStatus)
{
	bool bPadTwo = false;
	int32 padNumber = padButtonStatus->padID;

	CPad *pad = CPad::GetPad(padNumber);
	
	if ( Input_mapPadOneToPadTwo ) // rouz edit (ChatGPT)
		padNumber = 1;

	if ( padNumber == 1 )
		bPadTwo = true;
	
	bool bCam = false;
	int16 mode = TheCamera.Cams[TheCamera.ActiveCam].Mode;
	if ( mode == CCam::MODE_FLYBY || mode == CCam::MODE_FIXED )
		bCam = true;
	
	Input_DispatchPadButtons(padNumber, bPadTwo, bCam, true, ControlsManager.m_aButtonStates); // rouz edit (ChatGPT)

	return rsEVENTPROCESSED;
}

/*
 *****************************************************************************
 */
static RsEventStatus 
PadHandler(RsEvent event, void *param)
{
	switch( event )
	{
		case rsPADBUTTONDOWN:
		{
			return HandlePadButtonDown((RsPadButtonStatus *)param);
		}

		case rsPADBUTTONUP:
		{
			return HandlePadButtonUp((RsPadButtonStatus *)param);
		}

		default:
		{
			return rsEVENTNOTPROCESSED;
		}
	}
}


/*
 *****************************************************************************
 */
RwBool
AttachInputDevices(void)
{
#ifndef IGNORE_MOUSE_KEYBOARD
	RsInputDeviceAttach(rsKEYBOARD, KeyboardHandler);
#endif

	RsInputDeviceAttach(rsPAD, PadHandler);

	return TRUE;
}
