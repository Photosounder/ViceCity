//+ rouz edit (ChatGPT)
#include "common.h"
#include "Frontend.h"
#include "Camera.h"
#include "CutsceneMgr.h"
#include "Draw_backend.h"

int8_t Draw_BackendAspectPreference(void)
{
	// Read the original signed menu preference
	return FrontEndMenuManager.m_PrefsUseWideScreen;
}
bool Draw_BackendWidescreen(void)
{
	// Read the live camera widescreen flag at the original branch point
	return TheCamera.m_WideScreenOn;
}
float Draw_BackendScreenWidth(void)
{
	// Retain the original platform-dependent screen width and float conversion
	return SCREEN_WIDTH;
}
float Draw_BackendScreenHeight(void)
{
	// Retain the original platform-dependent screen height and float conversion
	return SCREEN_HEIGHT;
}
float Draw_BackendFovMultiplier(void)
{
	// Read the original live float FOV multiplier
	return rouz.fov_mul;
}
//- rouz edit (ChatGPT)
