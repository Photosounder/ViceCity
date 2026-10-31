//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
typedef struct CControllerState {
	int16_t LeftStickX, LeftStickY;
	int16_t RightStickX, RightStickY;
	int16_t LeftShoulder1, LeftShoulder2;
	int16_t RightShoulder1, RightShoulder2;
	int16_t DPadUp, DPadDown, DPadLeft, DPadRight;
	int16_t Start, Select;
	int16_t Square, Triangle, Cross, Circle;
	int16_t LeftShock, RightShock;
	int16_t NetworkTalk;
} CControllerState;
typedef struct CMouseControllerState {
	//uint32 btns;	// bit 0-2 button 1-3

	bool LMB;
	bool RMB;
	bool MMB;
	bool WHEELUP;
	bool WHEELDN;
	bool MXB1;
	bool MXB2;
	char _pad0;
	
	float x, y;
} CMouseControllerState;
typedef struct CMousePointerStateHelper {
	bool bInvertHorizontally;
	bool bInvertVertically;
} CMousePointerStateHelper;
typedef struct CKeyboardState {
	int16_t F[12];
	int16_t VK_KEYS[256];
	int16_t ESC;
	int16_t INS;
	int16_t DEL;
	int16_t HOME;
	int16_t END;
	int16_t PGUP;
	int16_t PGDN;
	int16_t UP;
	int16_t DOWN;
	int16_t LEFT;
	int16_t RIGHT;
	int16_t SCROLLLOCK;
	int16_t PAUSE;
	int16_t NUMLOCK;
	int16_t DIV;
	int16_t MUL;
	int16_t SUB;
	int16_t ADD;
	int16_t ENTER;
	int16_t DECIMAL;
	int16_t NUM1;
	int16_t NUM2;
	int16_t NUM3;
	int16_t NUM4;
	int16_t NUM5;
	int16_t NUM6;
	int16_t NUM7;
	int16_t NUM8;
	int16_t NUM9;
	int16_t NUM0;
	int16_t BACKSP;
	int16_t TAB;
	int16_t CAPSLOCK;
	int16_t EXTENTER;
	int16_t LSHIFT;
	int16_t RSHIFT;
	int16_t SHIFT;
	int16_t LCTRL;
	int16_t RCTRL;
	int16_t LALT;
	int16_t RALT;
	int16_t LWIN;
	int16_t RWIN;
	int16_t APPS;
} CKeyboardState;
#ifdef __cplusplus
extern "C" {
#endif
extern CMousePointerStateHelper MousePointerStateHelper;
bool ControllerState_CheckForInput(CControllerState *self);
void ControllerState_Clear(CControllerState *self);
float ControllerState_GetLeftStickX(CControllerState *self);
float ControllerState_GetLeftStickY(CControllerState *self);
float ControllerState_GetRightStickX(CControllerState *self);
float ControllerState_GetRightStickY(CControllerState *self);
void KeyboardState_Clear(CKeyboardState *self);
void MouseControllerState_Init(CMouseControllerState *self);
void MouseControllerState_Clear(CMouseControllerState *self);
CMouseControllerState MousePointerState_GetMouseSetUp(CMousePointerStateHelper *self);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
