//+ rouz edit (ChatGPT)
#include "config.h"
#include "InputState.h"
#include "InputState_backend.h"
#define ARRAY_SIZE(array) (sizeof(array)/sizeof((array)[0]))
CMousePointerStateHelper MousePointerStateHelper;

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
