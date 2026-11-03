//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "ControllerActions.h"
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
extern CKeyboardState Input_oldKeyState;
extern CKeyboardState Input_newKeyState;
extern CKeyboardState Input_tempKeyState;
extern char Input_keyboardCheatString[30];
extern bool Input_mapPadOneToPadTwo;
extern bool Input_displayNoControllerMessage;
extern bool Input_obsoleteControllerMessage;
extern bool Input_oldDisplayNoControllerMessage;
extern bool Input_debugCameraOn;
extern bool Input_playerHasCheated;
extern bool Input_invertPadLook;
extern bool Input_affectedByController;
uint32_t Input_TimeSinceLastInput(int32_t lastTimeTouched);
extern CMousePointerStateHelper MousePointerStateHelper;
extern CMouseControllerState Input_oldMouseState;
extern CMouseControllerState Input_newMouseState;
extern CMouseControllerState Input_tempMouseState;
static inline bool Input_GetLeftMouseJustDown(void)
{
    // Preserve the original mouse state query
    return !!(Input_newMouseState.LMB && !Input_oldMouseState.LMB);
}

static inline bool Input_GetRightMouseJustDown(void)
{
    // Preserve the original mouse state query
    return !!(Input_newMouseState.RMB && !Input_oldMouseState.RMB);
}

static inline bool Input_GetMiddleMouseJustDown(void)
{
    // Preserve the original mouse state query
    return !!(Input_newMouseState.MMB && !Input_oldMouseState.MMB);
}

static inline bool Input_GetMouseWheelUpJustDown(void)
{
    // Preserve the original mouse state query
    return !!(Input_newMouseState.WHEELUP && !Input_oldMouseState.WHEELUP);
}

static inline bool Input_GetMouseWheelDownJustDown(void)
{
    // Preserve the original mouse state query
    return !!(Input_newMouseState.WHEELDN && !Input_oldMouseState.WHEELDN);
}

static inline bool Input_GetMouseX1JustDown(void)
{
    // Preserve the original mouse state query
    return !!(Input_newMouseState.MXB1    && !Input_oldMouseState.MXB1);
}

static inline bool Input_GetMouseX2JustDown(void)
{
    // Preserve the original mouse state query
    return !!(Input_newMouseState.MXB2    && !Input_oldMouseState.MXB2);
}

static inline bool Input_GetLeftMouseJustUp(void)
{
    // Preserve the original mouse state query
    return !!(!Input_newMouseState.LMB && Input_oldMouseState.LMB);
}

static inline bool Input_GetRightMouseJustUp(void)
{
    // Preserve the original mouse state query
    return !!(!Input_newMouseState.RMB && Input_oldMouseState.RMB);
}

static inline bool Input_GetMiddleMouseJustUp(void)
{
    // Preserve the original mouse state query
    return !!(!Input_newMouseState.MMB && Input_oldMouseState.MMB);
}

static inline bool Input_GetMouseWheelUpJustUp(void)
{
    // Preserve the original mouse state query
    return !!(!Input_newMouseState.WHEELUP && Input_oldMouseState.WHEELUP);
}

static inline bool Input_GetMouseWheelDownJustUp(void)
{
    // Preserve the original mouse state query
    return !!(!Input_newMouseState.WHEELDN && Input_oldMouseState.WHEELDN);
}

static inline bool Input_GetMouseX1JustUp(void)
{
    // Preserve the original mouse state query
    return !!(!Input_newMouseState.MXB1 && Input_oldMouseState.MXB1);
}

static inline bool Input_GetMouseX2JustUp(void)
{
    // Preserve the original mouse state query
    return !!(!Input_newMouseState.MXB2 && Input_oldMouseState.MXB2);
}

static inline bool Input_GetLeftMouse(void)
{
    // Preserve the original mouse state query
    return Input_newMouseState.LMB;
}

static inline bool Input_GetRightMouse(void)
{
    // Preserve the original mouse state query
    return Input_newMouseState.RMB;
}

static inline bool Input_GetMiddleMouse(void)
{
    // Preserve the original mouse state query
    return Input_newMouseState.MMB;
}

static inline bool Input_GetMouseWheelUp(void)
{
    // Preserve the original mouse state query
    return Input_newMouseState.WHEELUP;
}

static inline bool Input_GetMouseWheelDown(void)
{
    // Preserve the original mouse state query
    return Input_newMouseState.WHEELDN;
}

static inline bool Input_GetMouseX1(void)
{
    // Preserve the original mouse state query
    return Input_newMouseState.MXB1;
}

static inline bool Input_GetMouseX2(void)
{
    // Preserve the original mouse state query
    return Input_newMouseState.MXB2;
}

static inline bool Input_GetLeftMouseUp(void)
{
    // Preserve the original mouse state query
    return !Input_oldMouseState.LMB;
}

static inline bool Input_GetRightMouseUp(void)
{
    // Preserve the original mouse state query
    return !Input_oldMouseState.RMB;
}

static inline bool Input_GetMiddleMouseUp(void)
{
    // Preserve the original mouse state query
    return !Input_oldMouseState.MMB;
}

static inline bool Input_GetMouseWheelUpUp(void)
{
    // Preserve the original mouse state query
    return !Input_oldMouseState.WHEELUP;
}

static inline bool Input_GetMouseWheelDownUp(void)
{
    // Preserve the original mouse state query
    return !Input_oldMouseState.WHEELDN;
}

static inline bool Input_GetMouseX1Up(void)
{
    // Preserve the original mouse state query
    return !Input_oldMouseState.MXB1;
}

static inline bool Input_GetMouseX2Up(void)
{
    // Preserve the original mouse state query
    return !Input_oldMouseState.MXB2;
}

static inline float Input_GetMouseX(void)
{
    // Preserve the original mouse state query
    return Input_newMouseState.x;
}

static inline float Input_GetMouseY(void)
{
    // Preserve the original mouse state query
    return Input_newMouseState.y;
}
static inline bool Input_GetCharJustDown(int32_t c);
static inline bool Input_GetFJustDown(int32_t n);
static inline bool Input_GetEscapeJustDown(void);
static inline bool Input_GetInsertJustDown(void);
static inline bool Input_GetDeleteJustDown(void);
static inline bool Input_GetHomeJustDown(void);
static inline bool Input_GetEndJustDown(void);
static inline bool Input_GetPageUpJustDown(void);
static inline bool Input_GetPageDownJustDown(void);
static inline bool Input_GetUpJustDown(void);
static inline bool Input_GetDownJustDown(void);
static inline bool Input_GetLeftJustDown(void);
static inline bool Input_GetRightJustDown(void);
static inline bool Input_GetScrollLockJustDown(void);
static inline bool Input_GetPauseJustDown(void);
static inline bool Input_GetNumLockJustDown(void);
static inline bool Input_GetDivideJustDown(void);
static inline bool Input_GetTimesJustDown(void);
static inline bool Input_GetMinusJustDown(void);
static inline bool Input_GetPlusJustDown(void);
static inline bool Input_GetPadEnterJustDown(void);
static inline bool Input_GetPadDelJustDown(void);
static inline bool Input_GetPad1JustDown(void);
static inline bool Input_GetPad2JustDown(void);
static inline bool Input_GetPad3JustDown(void);
static inline bool Input_GetPad4JustDown(void);
static inline bool Input_GetPad5JustDown(void);
static inline bool Input_GetPad6JustDown(void);
static inline bool Input_GetPad7JustDown(void);
static inline bool Input_GetPad8JustDown(void);
static inline bool Input_GetPad9JustDown(void);
static inline bool Input_GetPad0JustDown(void);
static inline bool Input_GetBackspaceJustDown(void);
static inline bool Input_GetTabJustDown(void);
static inline bool Input_GetCapsLockJustDown(void);
static inline bool Input_GetReturnJustDown(void);
static inline bool Input_GetLeftShiftJustDown(void);
static inline bool Input_GetShiftJustDown(void);
static inline bool Input_GetRightShiftJustDown(void);
static inline bool Input_GetLeftCtrlJustDown(void);
static inline bool Input_GetRightCtrlJustDown(void);
static inline bool Input_GetLeftAltJustDown(void);
static inline bool Input_GetRightAltJustDown(void);
static inline bool Input_GetLeftWinJustDown(void);
static inline bool Input_GetRightWinJustDown(void);
static inline bool Input_GetAppsJustDown(void);
static inline bool Input_GetEnterJustDown(void);
static inline bool Input_GetAltJustDown(void);
static inline bool Input_GetLeftJustUp(void);
static inline bool Input_GetRightJustUp(void);
static inline bool Input_GetEnterJustUp(void);
static inline bool Input_GetReturnJustUp(void);
static inline bool Input_GetPadEnterJustUp(void);
static inline bool Input_GetChar(int32_t c);
static inline bool Input_GetF(int32_t n);
static inline bool Input_GetEscape(void);
static inline bool Input_GetInsert(void);
static inline bool Input_GetDelete(void);
static inline bool Input_GetHome(void);
static inline bool Input_GetEnd(void);
static inline bool Input_GetPageUp(void);
static inline bool Input_GetPageDown(void);
static inline bool Input_GetUp(void);
static inline bool Input_GetDown(void);
static inline bool Input_GetLeft(void);
static inline bool Input_GetRight(void);
static inline bool Input_GetScrollLock(void);
static inline bool Input_GetPause(void);
static inline bool Input_GetNumLock(void);
static inline bool Input_GetDivide(void);
static inline bool Input_GetTimes(void);
static inline bool Input_GetMinus(void);
static inline bool Input_GetPlus(void);
static inline bool Input_GetPadEnter(void);
static inline bool Input_GetPadDel(void);
static inline bool Input_GetPad1(void);
static inline bool Input_GetPad2(void);
static inline bool Input_GetPad3(void);
static inline bool Input_GetPad4(void);
static inline bool Input_GetPad5(void);
static inline bool Input_GetPad6(void);
static inline bool Input_GetPad7(void);
static inline bool Input_GetPad8(void);
static inline bool Input_GetPad9(void);
static inline bool Input_GetPad0(void);
static inline bool Input_GetBackspace(void);
static inline bool Input_GetTab(void);
static inline bool Input_GetCapsLock(void);
static inline bool Input_GetEnter(void);
static inline bool Input_GetLeftShift(void);
static inline bool Input_GetShift(void);
static inline bool Input_GetRightShift(void);
static inline bool Input_GetLeftCtrl(void);
static inline bool Input_GetRightCtrl(void);
static inline bool Input_GetLeftAlt(void);
static inline bool Input_GetRightAlt(void);
static inline bool Input_GetLeftWin(void);
static inline bool Input_GetRightWin(void);
static inline bool Input_GetApps(void);

static inline bool Input_GetCharJustDown(int32_t c)
{
    // Preserve the original keyboard state query
    return !!(Input_newKeyState.VK_KEYS[c] && !Input_oldKeyState.VK_KEYS[c]);
}

static inline bool Input_GetFJustDown(int32_t n)
{
    // Preserve the original keyboard state query
    return !!(Input_newKeyState.F[n] && !Input_oldKeyState.F[n]);
}

static inline bool Input_GetEscapeJustDown(void)
{
    // Preserve the original keyboard state query
    return !!(Input_newKeyState.ESC && !Input_oldKeyState.ESC);
}

static inline bool Input_GetInsertJustDown(void)
{
    // Preserve the original keyboard state query
    return !!(Input_newKeyState.INS && !Input_oldKeyState.INS);
}

static inline bool Input_GetDeleteJustDown(void)
{
    // Preserve the original keyboard state query
    return !!(Input_newKeyState.DEL && !Input_oldKeyState.DEL);
}

static inline bool Input_GetHomeJustDown(void)
{
    // Preserve the original keyboard state query
    return !!(Input_newKeyState.HOME && !Input_oldKeyState.HOME);
}

static inline bool Input_GetEndJustDown(void)
{
    // Preserve the original keyboard state query
    return !!(Input_newKeyState.END && !Input_oldKeyState.END);
}

static inline bool Input_GetPageUpJustDown(void)
{
    // Preserve the original keyboard state query
    return !!(Input_newKeyState.PGUP && !Input_oldKeyState.PGUP);
}

static inline bool Input_GetPageDownJustDown(void)
{
    // Preserve the original keyboard state query
    return !!(Input_newKeyState.PGDN && !Input_oldKeyState.PGDN);
}

static inline bool Input_GetUpJustDown(void)
{
    // Preserve the original keyboard state query
    return !!(Input_newKeyState.UP && !Input_oldKeyState.UP);
}

static inline bool Input_GetDownJustDown(void)
{
    // Preserve the original keyboard state query
    return !!(Input_newKeyState.DOWN && !Input_oldKeyState.DOWN);
}

static inline bool Input_GetLeftJustDown(void)
{
    // Preserve the original keyboard state query
    return !!(Input_newKeyState.LEFT && !Input_oldKeyState.LEFT);
}

static inline bool Input_GetRightJustDown(void)
{
    // Preserve the original keyboard state query
    return !!(Input_newKeyState.RIGHT && !Input_oldKeyState.RIGHT);
}

static inline bool Input_GetScrollLockJustDown(void)
{
    // Preserve the original keyboard state query
    return !!(Input_newKeyState.SCROLLLOCK && !Input_oldKeyState.SCROLLLOCK);
}

static inline bool Input_GetPauseJustDown(void)
{
    // Preserve the original keyboard state query
    return !!(Input_newKeyState.PAUSE && !Input_oldKeyState.PAUSE);
}

static inline bool Input_GetNumLockJustDown(void)
{
    // Preserve the original keyboard state query
    return !!(Input_newKeyState.NUMLOCK && !Input_oldKeyState.NUMLOCK);
}

static inline bool Input_GetDivideJustDown(void)
{
    // Preserve the original keyboard state query
    return !!(Input_newKeyState.DIV && !Input_oldKeyState.DIV);
}

static inline bool Input_GetTimesJustDown(void)
{
    // Preserve the original keyboard state query
    return !!(Input_newKeyState.MUL && !Input_oldKeyState.MUL);
}

static inline bool Input_GetMinusJustDown(void)
{
    // Preserve the original keyboard state query
    return !!(Input_newKeyState.SUB && !Input_oldKeyState.SUB);
}

static inline bool Input_GetPlusJustDown(void)
{
    // Preserve the original keyboard state query
    return !!(Input_newKeyState.ADD && !Input_oldKeyState.ADD);
}

static inline bool Input_GetPadEnterJustDown(void)
{
    // Preserve the original keyboard state query
    return !!(Input_newKeyState.ENTER && !Input_oldKeyState.ENTER);
}

static inline bool Input_GetPadDelJustDown(void)
{
    // Preserve the original keyboard state query
    return !!(Input_newKeyState.DECIMAL && !Input_oldKeyState.DECIMAL);
}

static inline bool Input_GetPad1JustDown(void)
{
    // Preserve the original keyboard state query
    return !!(Input_newKeyState.NUM1 && !Input_oldKeyState.NUM1);
}

static inline bool Input_GetPad2JustDown(void)
{
    // Preserve the original keyboard state query
    return !!(Input_newKeyState.NUM2 && !Input_oldKeyState.NUM2);
}

static inline bool Input_GetPad3JustDown(void)
{
    // Preserve the original keyboard state query
    return !!(Input_newKeyState.NUM3 && !Input_oldKeyState.NUM3);
}

static inline bool Input_GetPad4JustDown(void)
{
    // Preserve the original keyboard state query
    return !!(Input_newKeyState.NUM4 && !Input_oldKeyState.NUM4);
}

static inline bool Input_GetPad5JustDown(void)
{
    // Preserve the original keyboard state query
    return !!(Input_newKeyState.NUM5 && !Input_oldKeyState.NUM5);
}

static inline bool Input_GetPad6JustDown(void)
{
    // Preserve the original keyboard state query
    return !!(Input_newKeyState.NUM6 && !Input_oldKeyState.NUM6);
}

static inline bool Input_GetPad7JustDown(void)
{
    // Preserve the original keyboard state query
    return !!(Input_newKeyState.NUM7 && !Input_oldKeyState.NUM7);
}

static inline bool Input_GetPad8JustDown(void)
{
    // Preserve the original keyboard state query
    return !!(Input_newKeyState.NUM8 && !Input_oldKeyState.NUM8);
}

static inline bool Input_GetPad9JustDown(void)
{
    // Preserve the original keyboard state query
    return !!(Input_newKeyState.NUM9 && !Input_oldKeyState.NUM9);
}

static inline bool Input_GetPad0JustDown(void)
{
    // Preserve the original keyboard state query
    return !!(Input_newKeyState.NUM0 && !Input_oldKeyState.NUM0);
}

static inline bool Input_GetBackspaceJustDown(void)
{
    // Preserve the original keyboard state query
    return !!(Input_newKeyState.BACKSP && !Input_oldKeyState.BACKSP);
}

static inline bool Input_GetTabJustDown(void)
{
    // Preserve the original keyboard state query
    return !!(Input_newKeyState.TAB && !Input_oldKeyState.TAB);
}

static inline bool Input_GetCapsLockJustDown(void)
{
    // Preserve the original keyboard state query
    return !!(Input_newKeyState.CAPSLOCK && !Input_oldKeyState.CAPSLOCK);
}

static inline bool Input_GetReturnJustDown(void)
{
    // Preserve the original keyboard state query
    return !!(Input_newKeyState.EXTENTER && !Input_oldKeyState.EXTENTER);
}

static inline bool Input_GetLeftShiftJustDown(void)
{
    // Preserve the original keyboard state query
    return !!(Input_newKeyState.LSHIFT && !Input_oldKeyState.LSHIFT);
}

static inline bool Input_GetShiftJustDown(void)
{
    // Preserve the original keyboard state query
    return !!(Input_newKeyState.SHIFT && !Input_oldKeyState.SHIFT);
}

static inline bool Input_GetRightShiftJustDown(void)
{
    // Preserve the original keyboard state query
    return !!(Input_newKeyState.RSHIFT && !Input_oldKeyState.RSHIFT);
}

static inline bool Input_GetLeftCtrlJustDown(void)
{
    // Preserve the original keyboard state query
    return !!(Input_newKeyState.LCTRL && !Input_oldKeyState.LCTRL);
}

static inline bool Input_GetRightCtrlJustDown(void)
{
    // Preserve the original keyboard state query
    return !!(Input_newKeyState.RCTRL && !Input_oldKeyState.RCTRL);
}

static inline bool Input_GetLeftAltJustDown(void)
{
    // Preserve the original keyboard state query
    return !!(Input_newKeyState.LALT && !Input_oldKeyState.LALT);
}

static inline bool Input_GetRightAltJustDown(void)
{
    // Preserve the original keyboard state query
    return !!(Input_newKeyState.RALT && !Input_oldKeyState.RALT);
}

static inline bool Input_GetLeftWinJustDown(void)
{
    // Preserve the original keyboard state query
    return !!(Input_newKeyState.LWIN && !Input_oldKeyState.LWIN);
}

static inline bool Input_GetRightWinJustDown(void)
{
    // Preserve the original keyboard state query
    return !!(Input_newKeyState.RWIN && !Input_oldKeyState.RWIN);
}

static inline bool Input_GetAppsJustDown(void)
{
    // Preserve the original keyboard state query
    return !!(Input_newKeyState.APPS && !Input_oldKeyState.APPS);
}

static inline bool Input_GetEnterJustDown(void)
{
    // Preserve the original keyboard state query
    return Input_GetPadEnterJustDown() || Input_GetReturnJustDown();
}

static inline bool Input_GetAltJustDown(void)
{
    // Preserve the original keyboard state query
    return Input_GetLeftAltJustDown() || Input_GetRightAltJustDown();
}

static inline bool Input_GetLeftJustUp(void)
{
    // Preserve the original keyboard state query
    return !!(!Input_newKeyState.LEFT && Input_oldKeyState.LEFT);
}

static inline bool Input_GetRightJustUp(void)
{
    // Preserve the original keyboard state query
    return !!(!Input_newKeyState.RIGHT && Input_oldKeyState.RIGHT);
}

static inline bool Input_GetEnterJustUp(void)
{
    // Preserve the original keyboard state query
    return Input_GetPadEnterJustUp() || Input_GetReturnJustUp();
}

static inline bool Input_GetReturnJustUp(void)
{
    // Preserve the original keyboard state query
    return !!(!Input_newKeyState.EXTENTER && Input_oldKeyState.EXTENTER);
}

static inline bool Input_GetPadEnterJustUp(void)
{
    // Preserve the original keyboard state query
    return !!(!Input_newKeyState.ENTER && Input_oldKeyState.ENTER);
}

static inline bool Input_GetChar(int32_t c)
{
    // Preserve the original keyboard state query
    return Input_newKeyState.VK_KEYS[c];
}

static inline bool Input_GetF(int32_t n)
{
    // Preserve the original keyboard state query
    return Input_newKeyState.F[n];
}

static inline bool Input_GetEscape(void)
{
    // Preserve the original keyboard state query
    return Input_newKeyState.ESC;
}

static inline bool Input_GetInsert(void)
{
    // Preserve the original keyboard state query
    return Input_newKeyState.INS;
}

static inline bool Input_GetDelete(void)
{
    // Preserve the original keyboard state query
    return Input_newKeyState.DEL;
}

static inline bool Input_GetHome(void)
{
    // Preserve the original keyboard state query
    return Input_newKeyState.HOME;
}

static inline bool Input_GetEnd(void)
{
    // Preserve the original keyboard state query
    return Input_newKeyState.END;
}

static inline bool Input_GetPageUp(void)
{
    // Preserve the original keyboard state query
    return Input_newKeyState.PGUP;
}

static inline bool Input_GetPageDown(void)
{
    // Preserve the original keyboard state query
    return Input_newKeyState.PGDN;
}

static inline bool Input_GetUp(void)
{
    // Preserve the original keyboard state query
    return Input_newKeyState.UP;
}

static inline bool Input_GetDown(void)
{
    // Preserve the original keyboard state query
    return Input_newKeyState.DOWN;
}

static inline bool Input_GetLeft(void)
{
    // Preserve the original keyboard state query
    return Input_newKeyState.LEFT;
}

static inline bool Input_GetRight(void)
{
    // Preserve the original keyboard state query
    return Input_newKeyState.RIGHT;
}

static inline bool Input_GetScrollLock(void)
{
    // Preserve the original keyboard state query
    return Input_newKeyState.SCROLLLOCK;
}

static inline bool Input_GetPause(void)
{
    // Preserve the original keyboard state query
    return Input_newKeyState.PAUSE;
}

static inline bool Input_GetNumLock(void)
{
    // Preserve the original keyboard state query
    return Input_newKeyState.NUMLOCK;
}

static inline bool Input_GetDivide(void)
{
    // Preserve the original keyboard state query
    return Input_newKeyState.DIV;
}

static inline bool Input_GetTimes(void)
{
    // Preserve the original keyboard state query
    return Input_newKeyState.MUL;
}

static inline bool Input_GetMinus(void)
{
    // Preserve the original keyboard state query
    return Input_newKeyState.SUB;
}

static inline bool Input_GetPlus(void)
{
    // Preserve the original keyboard state query
    return Input_newKeyState.ADD;
}

static inline bool Input_GetPadEnter(void)
{
    // Preserve the original keyboard state query
    return Input_newKeyState.ENTER;
}

static inline bool Input_GetPadDel(void)
{
    // Preserve the original keyboard state query
    return Input_newKeyState.DECIMAL;
}

static inline bool Input_GetPad1(void)
{
    // Preserve the original keyboard state query
    return Input_newKeyState.NUM1;
}

static inline bool Input_GetPad2(void)
{
    // Preserve the original keyboard state query
    return Input_newKeyState.NUM2;
}

static inline bool Input_GetPad3(void)
{
    // Preserve the original keyboard state query
    return Input_newKeyState.NUM3;
}

static inline bool Input_GetPad4(void)
{
    // Preserve the original keyboard state query
    return Input_newKeyState.NUM4;
}

static inline bool Input_GetPad5(void)
{
    // Preserve the original keyboard state query
    return Input_newKeyState.NUM5;
}

static inline bool Input_GetPad6(void)
{
    // Preserve the original keyboard state query
    return Input_newKeyState.NUM6;
}

static inline bool Input_GetPad7(void)
{
    // Preserve the original keyboard state query
    return Input_newKeyState.NUM7;
}

static inline bool Input_GetPad8(void)
{
    // Preserve the original keyboard state query
    return Input_newKeyState.NUM8;
}

static inline bool Input_GetPad9(void)
{
    // Preserve the original keyboard state query
    return Input_newKeyState.NUM9;
}

static inline bool Input_GetPad0(void)
{
    // Preserve the original keyboard state query
    return Input_newKeyState.NUM0;
}

static inline bool Input_GetBackspace(void)
{
    // Preserve the original keyboard state query
    return Input_newKeyState.BACKSP;
}

static inline bool Input_GetTab(void)
{
    // Preserve the original keyboard state query
    return Input_newKeyState.TAB;
}

static inline bool Input_GetCapsLock(void)
{
    // Preserve the original keyboard state query
    return Input_newKeyState.CAPSLOCK;
}

static inline bool Input_GetEnter(void)
{
    // Preserve the original keyboard state query
    return Input_newKeyState.EXTENTER;
}

static inline bool Input_GetLeftShift(void)
{
    // Preserve the original keyboard state query
    return Input_newKeyState.LSHIFT;
}

static inline bool Input_GetShift(void)
{
    // Preserve the original keyboard state query
    return Input_newKeyState.SHIFT;
}

static inline bool Input_GetRightShift(void)
{
    // Preserve the original keyboard state query
    return Input_newKeyState.RSHIFT;
}

static inline bool Input_GetLeftCtrl(void)
{
    // Preserve the original keyboard state query
    return Input_newKeyState.LCTRL;
}

static inline bool Input_GetRightCtrl(void)
{
    // Preserve the original keyboard state query
    return Input_newKeyState.RCTRL;
}

static inline bool Input_GetLeftAlt(void)
{
    // Preserve the original keyboard state query
    return Input_newKeyState.LALT;
}

static inline bool Input_GetRightAlt(void)
{
    // Preserve the original keyboard state query
    return Input_newKeyState.RALT;
}

static inline bool Input_GetLeftWin(void)
{
    // Preserve the original keyboard state query
    return Input_newKeyState.LWIN;
}

static inline bool Input_GetRightWin(void)
{
    // Preserve the original keyboard state query
    return Input_newKeyState.RWIN;
}

static inline bool Input_GetApps(void)
{
    // Preserve the original keyboard state query
    return Input_newKeyState.APPS;
}

bool ControllerState_CheckForInput(CControllerState *self);
void ControllerState_Clear(CControllerState *self);
CControllerState Input_ReconcileControllers(const CControllerState *first, const CControllerState *second);
float ControllerState_GetLeftStickX(CControllerState *self);
float ControllerState_GetLeftStickY(CControllerState *self);
float ControllerState_GetRightStickX(CControllerState *self);
float ControllerState_GetRightStickY(CControllerState *self);
void KeyboardState_Clear(CKeyboardState *self);
void Input_ClearKeyboardHistory(void);
void Input_ClearMouseHistory(void);
void Input_StartShake(int16_t *duration, uint8_t *frequency, int16_t requestedDuration, uint8_t requestedFrequency);
void Input_StartTrainShake(int16_t *duration, uint8_t *frequency, float distance);
void Input_AdvanceKeyboardHistory(void);
void Input_AdvanceMouseHistory(void);
void Input_BeginGlfwMouseSample(double x, double y, double previousX, double previousY, bool menuActive);
void Input_SetMouseWheel(double wheel);
void Input_ApplyDirectMouseSample(int32_t x, int32_t y, int32_t wheel, const uint8_t *buttons, bool menuActive);
bool Input_GetIsMouseButtonDown(int32_t keycode);
bool Input_GetIsMouseButtonUp(int32_t keycode);
bool Input_GetIsKeyboardKeyDown(int32_t keycode);
bool Input_GetIsKeyboardKeyJustDown(int32_t keycode);
int32_t Input_GetControllerKeyAssociatedWithAction(InputControllerBinding bindings[][MAX_CONTROLLERTYPES], int32_t action, int32_t type);
void Input_SetControllerKeyAssociatedWithAction(InputControllerBinding bindings[][MAX_CONTROLLERTYPES], int32_t action, int32_t key, int32_t type);
int32_t Input_GetMouseButtonAssociatedWithAction(InputControllerBinding bindings[][MAX_CONTROLLERTYPES], int32_t action);
void Input_SetMouseButtonAssociatedWithAction(InputControllerBinding bindings[][MAX_CONTROLLERTYPES], int32_t action, int32_t button);
bool Input_GetIsKeyBlank(int32_t key, int32_t type);
void Input_BlankControllerActions(InputControllerBinding bindings[][MAX_CONTROLLERTYPES]);
void Input_ClearSimButtonPressCheckers(bool checkers[][MAX_CONTROLLERTYPES]);
void Input_ResetSettingOrder(InputControllerBinding bindings[][MAX_CONTROLLERTYPES], int32_t action);
void Input_ClearSettingsAssociatedWithAction(InputControllerBinding bindings[][MAX_CONTROLLERTYPES], int32_t action, int32_t type);
int32_t Input_GetNumOfSettingsForAction(InputControllerBinding bindings[][MAX_CONTROLLERTYPES], int32_t action);
void Input_WriteControllerBindings(int32_t file, const InputControllerBinding bindings[][MAX_CONTROLLERTYPES]);
void Input_ReadControllerBindings(int32_t file, InputControllerBinding bindings[][MAX_CONTROLLERTYPES], bool skipVehicleFireWeapon);
void Input_AffectPadFromKeyboard(const InputControllerBinding bindings[][MAX_CONTROLLERTYPES]);
void Input_AffectPadFromMouse(const InputControllerBinding bindings[][MAX_CONTROLLERTYPES]);
void InputConfig_TranslateShift(void);
bool InputConfig_MenuActive(void);
void InputConfig_ButtonDown(int32_t key, int32_t type);
void InputConfig_ButtonUp(int32_t key, int32_t type);
void Input_HandleKeyDown(int32_t key, void (*cheat)(void *, int32_t), void *context);
void Input_HandleKeyUp(int32_t key);
void Input_MapPadKeyDown(CControllerState *state, int32_t key);
void Input_MapPadKeyUp(CControllerState *state, int32_t key);
void Input_DispatchPadButtons(int32_t padNumber, bool padTwo, bool camera, bool release, const bool *buttons);
int32_t Input_FindNewDirectButton(const uint8_t *current, const uint8_t *previous, int32_t count);
int32_t Input_FindNewMappedButton(const bool *current, const bool *previous, int32_t count);
int32_t Input_FindNewRawButton(const uint8_t *current, const uint8_t *previous, int32_t sourceCount, int32_t count);
void Input_ResetJoyButtons(bool *buttons, int32_t count);
void Input_ReadDirectJoyButtons(bool *buttons, const uint8_t *source, int32_t count);
void Input_ReadMappedJoyButtons(bool *buttons, const bool *source, int32_t count, int32_t guide, int32_t (*map)(int32_t));
void Input_ReadRawJoyButtons(bool *buttons, const uint8_t *source, int32_t sourceCount, int32_t count);
void InputEvents_UpdateJoyButtons(int32_t padNumber);
bool InputEvents_MenuActive(void);
void InputEvents_DispatchJoyButton(int32_t button, int32_t padNumber, bool menu, bool release);
void MouseControllerState_Init(CMouseControllerState *self);
void MouseControllerState_Clear(CMouseControllerState *self);
CMouseControllerState MousePointerState_GetMouseSetUp(CMousePointerStateHelper *self);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
