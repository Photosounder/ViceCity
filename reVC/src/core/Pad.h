//+ rouz edit (ChatGPT)
#pragma once
#include "InputState.h"

enum {
	PLAYERCONTROL_ENABLED = 0,
	PLAYERCONTROL_CAMERA = 1,
	PLAYERCONTROL_UNK2 = 2,
	PLAYERCONTROL_GARAGE = 4,
	PLAYERCONTROL_UNK8 = 8,
	PLAYERCONTROL_UNK10 = 16,
	PLAYERCONTROL_PLAYERINFO = 32,
	PLAYERCONTROL_PHONE = 64,
	PLAYERCONTROL_CUTSCENE = 128,
	PLAYERCONTROL_SHORTCUT_TAXI = 256,
};


VALIDATE_SIZE(CControllerState, 0x2A);



VALIDATE_SIZE(CMouseControllerState, 0x10);



VALIDATE_SIZE(CMousePointerStateHelper, 0x2);






VALIDATE_SIZE(CKeyboardState, 0x270);

enum
{
	// taken from miss2
	PAD1 = 0,
	PAD2,
	
	MAX_PADS
};

class CPad
{
public:
	enum
	{
		HORNHISTORY_SIZE = 5,
		DRUNK_STEERING_BUFFER_SIZE = 10,
	};
	CControllerState NewState;
	CControllerState OldState;
	int16 SteeringLeftRightBuffer[DRUNK_STEERING_BUFFER_SIZE];
	int32 DrunkDrivingBufferUsed;
	CControllerState PCTempKeyState;
	CControllerState PCTempJoyState;
	CControllerState PCTempMouseState;
	// straight out of my IDB
	int16 Phase;
	int16 Mode;
	int16 ShakeDur;
	uint16 DisablePlayerControls;
	uint8 ShakeFreq;
	bool bHornHistory[HORNHISTORY_SIZE];
	uint8 iCurrHornHistory;
	int8 JustOutOfFrontend;
	int8 bApplyBrakes;
	char CheatString[12];
	int32 LastTimeTouched;
	int32 AverageWeapon;
	int32 AverageEntries;

#ifdef DETECT_PAD_INPUT_SWITCH
#endif
	CPad() { }
	~CPad() { }


	
	
#ifdef GTA_PS2_STUFF
	static void Initialise(void);
#endif
	void Clear(bool bResetPlayerControls);
	void UpdateMouse();
	void StartShake(int16 nDur, uint8 nFreq);
	void StartShake_Distance(int16 nDur, uint8 nFreq, float fX, float fY, float fz);
	void StartShake_Train(float fX, float fY);
#ifdef GTA_PS2_STUFF
	void AddToCheatString(char c);
#endif
	void AddToPCCheatString(char c);

	static void UpdatePads(void);
	void ProcessPCSpecificStuff(void);
	void Update(int16 pad);
	
	static void DoCheats(void);
	void DoCheats(int16 unk);
	
	static void StopPadsShaking(void);
	void StopShaking(int16 pad);
	
	static CPad *GetPad(int32 pad);
	
	int16 GetSteeringLeftRight(void);
	int16 GetSteeringUpDown(void);
	int16 GetCarGunUpDown(void);
	int16 GetCarGunLeftRight(void);
	int16 GetPedWalkLeftRight(void);
	int16 GetPedWalkUpDown(void);
	int16 GetAnalogueUpDown(void);
	int16 GetAnalogueLeftRight(void);
	bool GetLookLeft(void);
	bool GetLookRight(void);
	bool GetLookBehindForCar(void);
	bool GetLookBehindForPed(void);
	bool GetHorn(void);
	bool HornJustDown(void);
	bool GetCarGunFired(void);
	bool CarGunJustDown(void);
	int16 GetHandBrake(void);
	int16 GetBrake(void);
	bool GetExitVehicle(void);
	bool ExitVehicleJustDown(void);
	int32 GetWeapon(void);
	bool WeaponJustDown(void);
	int16 GetAccelerate(void);
	bool CycleCameraModeJustDown(void);
	bool CycleCameraModeUpJustDown(void);
	bool CycleCameraModeDownJustDown(void);
	bool ChangeStationJustDown(void);
	bool CycleWeaponLeftJustDown(void);
	bool CycleWeaponRightJustDown(void);
	bool GetTarget(void);
	bool TargetJustDown(void);
	bool DuckJustDown(void);
	bool CollectPickupJustDown(void);
	bool JumpJustDown(void);
	bool GetSprint(void);
	bool ShiftTargetLeftJustDown(void);
	bool ShiftTargetRightJustDown(void);
	bool GetAnaloguePadUp(void);
	bool GetAnaloguePadDown(void);
	bool GetAnaloguePadLeft(void);
	bool GetAnaloguePadRight(void);
	bool GetAnaloguePadLeftJustUp(void);
	bool GetAnaloguePadRightJustUp(void);
	bool ForceCameraBehindPlayer(void);
	bool SniperZoomIn(void);
	bool SniperZoomOut(void);
	int16 SniperModeLookLeftRight(void);
	int16 SniperModeLookUpDown(void);
	int16 LookAroundLeftRight(void);
	int16 LookAroundUpDown(void);
	void ResetAverageWeapon(void);
	static void FixPadsAfterSave(void);
	static void PrintErrorMessage(void);
	static void ResetCheats(void);
	static char *EditString(char *pStr, int32 nSize);
	static int32 *EditCodesForControls(int32 *pRsKeys, int32 nSize);
	void SetDrunkInputDelay(int32 delay) { DrunkDrivingBufferUsed = delay; }

#ifdef XINPUT
	static int XInputJoy1;
	static int XInputJoy2;
	void AffectFromXinput(uint32 pad);
#endif


	// pad

	bool GetTriangleJustDown()       { return !!(NewState.Triangle && !OldState.Triangle); }
	bool GetCircleJustDown()         { return !!(NewState.Circle && !OldState.Circle); }
	bool GetCrossJustDown()          { return !!(NewState.Cross && !OldState.Cross); }
	bool GetSquareJustDown()         { return !!(NewState.Square && !OldState.Square); }
	bool GetDPadUpJustDown()         { return !!(NewState.DPadUp && !OldState.DPadUp); }
	bool GetDPadDownJustDown()       { return !!(NewState.DPadDown && !OldState.DPadDown); }
	bool GetDPadLeftJustDown()       { return !!(NewState.DPadLeft && !OldState.DPadLeft); }
	bool GetDPadRightJustDown()      { return !!(NewState.DPadRight && !OldState.DPadRight); }
	bool GetLeftShoulder1JustDown()  { return !!(NewState.LeftShoulder1 && !OldState.LeftShoulder1); }
	bool GetLeftShoulder2JustDown()  { return !!(NewState.LeftShoulder2 && !OldState.LeftShoulder2); }
	bool GetRightShoulder1JustDown() { return !!(NewState.RightShoulder1 && !OldState.RightShoulder1); }
	bool GetRightShoulder2JustDown() { return !!(NewState.RightShoulder2 && !OldState.RightShoulder2); }
	bool GetLeftShockJustDown()      { return !!(NewState.LeftShock && !OldState.LeftShock); }
	bool GetRightShockJustDown()     { return !!(NewState.RightShock && !OldState.RightShock); }
	bool GetStartJustDown()          { return !!(NewState.Start && !OldState.Start); }
	bool GetSelectJustDown()          { return !!(NewState.Select && !OldState.Select); }
	bool GetLeftStickXJustDown() { return !!(NewState.LeftStickX && !OldState.LeftStickX); }
	bool GetLeftStickYJustDown() { return !!(NewState.LeftStickY && !OldState.LeftStickY); }
  
	bool GetTriangleJustUp() { return !!(!NewState.Triangle && OldState.Triangle); }
	bool GetCircleJustUp() { return !!(!NewState.Circle && OldState.Circle); }
	bool GetCrossJustUp() { return !!(!NewState.Cross && OldState.Cross); }
	bool GetSquareJustUp() { return !!(!NewState.Square && OldState.Square); }
	bool GetDPadUpJustUp() { return !!(!NewState.DPadUp && OldState.DPadUp); }
	bool GetDPadDownJustUp() { return !!(!NewState.DPadDown && OldState.DPadDown); }
	bool GetDPadLeftJustUp() { return !!(!NewState.DPadLeft && OldState.DPadLeft); }
	bool GetDPadRightJustUp() { return !!(!NewState.DPadRight && OldState.DPadRight); }

	bool GetTriangle()           { return !!NewState.Triangle; }
	bool GetCircle()             { return !!NewState.Circle; }
	bool GetCross()              { return !!NewState.Cross; }
	bool GetSquare()             { return !!NewState.Square; }
	bool GetDPadUp()             { return !!NewState.DPadUp; }
	bool GetDPadDown()           { return !!NewState.DPadDown; }
	bool GetDPadLeft()           { return !!NewState.DPadLeft; }
	bool GetDPadRight()          { return !!NewState.DPadRight; }
	bool GetLeftShoulder1(void)  { return !!NewState.LeftShoulder1; }
	bool GetLeftShoulder2(void)  { return !!NewState.LeftShoulder2; }
	bool GetRightShoulder1(void) { return !!NewState.RightShoulder1; }
	bool GetRightShoulder2(void) { return !!NewState.RightShoulder2; }
	bool GetStart()              { return !!NewState.Start; }
	bool GetSelect()              { return !!NewState.Select; }
	int16 GetLeftStickX(void)    { return NewState.LeftStickX; }
	int16 GetLeftStickY(void)    { return NewState.LeftStickY; }
	int16 GetRightStickX(void)    { return NewState.RightStickX; }
	int16 GetRightStickY(void)    { return NewState.RightStickY; }

	bool ArePlayerControlsDisabled(void) { return DisablePlayerControls != PLAYERCONTROL_ENABLED; }
	void SetDisablePlayerControls(uint16 who) { DisablePlayerControls |= who; }
	void SetEnablePlayerControls(uint16 who) { DisablePlayerControls &= ~who; }
	bool IsPlayerControlsDisabledBy(uint16 who) { return DisablePlayerControls & who; }
	
	int16 GetMode() { return Mode; }
	void SetMode(int16 mode) { Mode = mode; }
	
	static bool IsNoOrObsolete() { return Input_displayNoControllerMessage || Input_obsoleteControllerMessage; } // rouz edit (ChatGPT)
};

VALIDATE_SIZE(CPad, 0xFC);
extern CPad Pads[MAX_PADS];
//- rouz edit (ChatGPT)
