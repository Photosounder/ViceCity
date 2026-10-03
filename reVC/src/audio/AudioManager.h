//+ rouz edit (ChatGPT)
#pragma once
#include "../core/config.h"

#include "audio_enums.h"
#include "AudioCollision.h"
#include "PolRadio.h"
#include "AudioPoliceGame.h"
#ifdef __cplusplus
#include "VehicleModelInfo.h"
#include "Vehicle.h"
#endif

#include "AudioSound.h"
#include "AudioMath.h"
#include "AudioGeometry.h"

#ifdef __cplusplus
VALIDATE_SIZE(tSound, 96);
#endif

#ifdef __cplusplus
class CPhysical;
class CAutomobile;
#endif

#include "AudioEntities.h"

#ifdef __cplusplus
VALIDATE_SIZE(tAudioEntity, 40);
#endif

#include "AudioPedComments.h"

#ifdef __cplusplus
VALIDATE_SIZE(cPedComments, 0x490);
#endif

#include "AudioMissionPosition.h"

#include "AudioState.h"

#ifdef __cplusplus
class CEntity;
class CPlane;
#endif

#ifdef __cplusplus
VALIDATE_SIZE(cVehicleParams, 0x1C);
#endif

#include "AudioReflectionTypes.h" // rouz edit (ChatGPT)

typedef struct cAudioManager {
	uint8_t m_bIsInitialised;
	uint8_t m_bIsSurround; // used on PS2
	uint8_t m_bReduceReleasingPriority;
	uint8_t m_nActiveSamples;
	uint8_t m_bDoubleVolume; // unused
	uint8_t m_bDynamicAcousticModelingStatus;
	uint8_t m_nChannelOffset;
	float m_fSpeedOfSound;
	uint8_t m_bTimerJustReset;
	uint32_t m_nTimer;
	tSound m_sQueueSample;
	uint8_t m_nActiveQueue;
	tSound m_aRequestedQueue[NUM_SOUND_QUEUES][NUM_CHANNELS_GENERIC];
	uint8_t m_aRequestedOrderList[NUM_SOUND_QUEUES][NUM_CHANNELS_GENERIC];
	uint8_t m_nRequestedCount[NUM_SOUND_QUEUES];
	tSound m_asActiveSamples[NUM_CHANNELS_GENERIC];
	tAudioEntity m_asAudioEntities[NUM_AUDIOENTITIES];
	uint32_t m_aAudioEntityOrderList[NUM_AUDIOENTITIES];
	uint32_t m_nAudioEntitiesCount;
#ifdef AUDIO_REFLECTIONS
	AudioSoundPosition m_avecReflectionsPos[MAX_REFLECTIONS];
	float m_afReflectionsDistances[MAX_REFLECTIONS];
#endif
	cAudioScriptObjectManager m_sAudioScriptObjectManager;

	// miami
	uint8_t m_bIsPlayerShutUp;
	uint8_t m_nPlayerMood;
	uint32_t m_nPlayerMoodTimer;
	uint32_t field_4B38_vc;
	uint8_t m_bGenericSfx;

	cPedComments m_sPedComments;
	int32_t m_nFireAudioEntity;
	int32_t m_nWaterCannonEntity;
	int32_t m_nPoliceChannelEntity;
	cPoliceRadioQueue m_sPoliceRadioQueue;
	cAMCrime m_aCrimes[AUDIO_CRIME_COUNT];
	int32_t m_nFrontEndEntity;
	int32_t m_nCollisionEntity;
	cAudioCollisionManager m_sCollisionManager;
	int32_t m_nProjectileEntity;
	int32_t m_nEscalatorEntity;
	int32_t m_nExtraSoundsEntity;
#ifdef GTA_BRIDGE
	int32_t m_nBridgeEntity;
#endif

	// Mission audio stuff
	// So instead of making an array of struct they've added [MISSION_AUDIO_SLOTS] to every field...
	// Only someone with a VERY EXTRAORDINARY mind could have come up with that
	AudioSoundPosition m_vecMissionAudioPosition[MISSION_AUDIO_SLOTS];
	uint8_t m_bIsMissionAudio2D[MISSION_AUDIO_SLOTS];
	uint32_t m_nMissionAudioSampleIndex[MISSION_AUDIO_SLOTS];
	uint8_t m_nMissionAudioLoadingStatus[MISSION_AUDIO_SLOTS];
	uint8_t m_nMissionAudioPlayStatus[MISSION_AUDIO_SLOTS];
	uint8_t m_bIsMissionAudioPlaying[MISSION_AUDIO_SLOTS];
	int32_t m_nMissionAudioFramesToPlay[MISSION_AUDIO_SLOTS]; // possibly unsigned
	uint8_t m_bIsMissionAudioAllowedToPlay[MISSION_AUDIO_SLOTS];
	uint8_t m_bIsMissionAudioPhoneCall[MISSION_AUDIO_SLOTS];
	uint8_t m_nGlobalSfxVolumeMultiplier; // used to lower sfx volume during phone calls

	int32_t m_anRandomTable[5];
	uint8_t m_nTimeSpent;
	uint8_t m_bIsPaused;
	uint8_t m_bWasPaused;
	uint32_t m_FrameCounter;

#ifdef __cplusplus
	cAudioManager();
	~cAudioManager();

	
#ifdef GTA_PC
#ifdef AUDIO_REFLECTIONS
#endif
#endif

#ifdef AUDIO_REFLECTIONS
#endif
#ifdef GTA_PS2
	void LoadBankIfNecessary(uint8 bank); // this is used only on PS2 but technically not a platform code
#endif

#ifdef EXTERNAL_3D_SOUND // actually must have been && AUDIO_MSS as well
#endif

	// audio logic
	void PreInitialiseGameSpecificSetup();
	void PostInitialiseGameSpecificSetup();
	void PreTerminateGameSpecificShutdown();
	void PostTerminateGameSpecificShutdown();

	// vehicles
	void ProcessVehicle(CVehicle *vehicle);
	bool8 ProcessCarHeli(cVehicleParams &params);
	bool8 ProcessReverseGear(cVehicleParams &params);
	void ProcessModelHeliVehicle(cVehicleParams &params);
	void ProcessModelVehicle(cVehicleParams &params);
	bool8 ProcessVehicleEngine(cVehicleParams &params);
	void UpdateGasPedalAudio(CVehicle *veh, int vehType);
	void PlayerJustGotInCar();
	void PlayerJustLeftCar();
	void AddPlayerCarSample(uint8 emittingVolume, uint32 freq, uint32 sample, uint8 bank, uint8 counter, bool8 notLooping);
	void ProcessCesna(cVehicleParams &params);
	void ProcessPlayersVehicleEngine(cVehicleParams &params, CVehicle *veh);
	bool8 ProcessVehicleSkidding(cVehicleParams &params);
	float GetVehicleDriveWheelSkidValue(CVehicle *veh, tWheelState wheelState, float gasPedalAudio, cTransmission *transmission, float velocityChange);
	float GetVehicleNonDriveWheelSkidValue(CVehicle *veh, tWheelState wheelState, cTransmission *transmission, float velocityChange);
	bool8 ProcessVehicleHorn(cVehicleParams &params);
	bool8 UsesSiren(cVehicleParams &params);
	bool8 UsesSirenSwitching(cVehicleParams &params);
	bool8 ProcessVehicleSirenOrAlarm(cVehicleParams &params);
	bool8 UsesReverseWarning(uint32 model);
	bool8 ProcessVehicleReverseWarning(cVehicleParams &params);
	bool8 ProcessVehicleDoors(cVehicleParams &params);
	bool8 ProcessAirBrakes(cVehicleParams &params);
	bool8 HasAirBrakes(uint32 model);
	bool8 ProcessEngineDamage(cVehicleParams &params);
	bool8 ProcessCarBombTick(cVehicleParams &params);
	void ProcessVehicleOneShots(cVehicleParams &params);
#ifdef GTA_TRAIN
	bool8 ProcessTrainNoise(cVehicleParams &params);
#endif
	bool8 ProcessBoatEngine(cVehicleParams &params);
	bool8 ProcessBoatMovingOverWater(cVehicleParams &params);
	void ProcessPlane(cVehicleParams &params);
	void ProcessJumbo(cVehicleParams &params);
	void ProcessJumboTaxi();
	void ProcessJumboAccel(CPlane *plane);
	void ProcessJumboTakeOff(CPlane *plane);
	void ProcessJumboFlying();
	void ProcessJumboLanding(CPlane *plane);
	void ProcessJumboDecel(CPlane *plane);
	bool8 SetupJumboTaxiSound(uint8 vol);
	bool8 SetupJumboWhineSound(uint8 emittingVol, uint32 freq);
	bool8 SetupJumboEngineSound(uint8 vol, uint32 freq);
	bool8 SetupJumboFlySound(uint8 emittingVol);
	bool8 SetupJumboRumbleSound(uint8 emittingVol);
	int32 GetJumboTaxiFreq(); // inlined in vc

	// peds
	void ProcessPed(CPhysical *ped);
	void ProcessPedOneShots(cPedParams &params);
	void SetPedTalkingStatus(CPed *ped, bool8 status);
	void SetPlayersMood(uint8 mood, uint32 time);
	void ProcessPlayerMood();

	// ped comments
	void SetupPedComments(cPedParams &params, uint16 sound);
	uint32 GetPedCommentSfx(CPed *ped, uint16 sound);
	void GetPhrase(uint32 &phrase, uint32 &prevPhrase, uint32 sample, uint32 maxOffset);
	uint32 GetPlayerTalkSfx(CPed *ped, uint16 sound);
	uint32 GetGenericMaleTalkSfx(CPed *ped, uint16 sound);   // inlined in vc
	uint32 GetGenericFemaleTalkSfx(CPed *ped, uint16 sound); // inlined in vc
	uint32 GetDefaultTalkSfx(CPed *ped, uint16 sound);
	uint32 GetCopTalkSfx(CPed *ped, uint16 sound);
	uint32 GetSwatTalkSfx(CPed *ped, uint16 sound);
	uint32 GetFBITalkSfx(CPed *ped, uint16 sound);
	uint32 GetArmyTalkSfx(CPed *ped, uint16 sound);
	uint32 GetMedicTalkSfx(CPed *ped, uint16 sound);
	uint32 GetFiremanTalkSfx(CPed *ped, uint16 sound);
	uint32 GetWFYG1TalkSfx(CPed *ped, uint16 sound);
	uint32 GetWFYG2TalkSfx(CPed *ped, uint16 sound);
	uint32 GetHFYSTTalkSfx(CPed *ped, uint16 sound);
	uint32 GetHFOSTTalkSfx(CPed *ped, uint16 sound);
	uint32 GetHMYSTTalkSfx(CPed *ped, uint16 sound);
	uint32 GetHMOSTTalkSfx(CPed *ped, uint16 sound);
	uint32 GetHFYRITalkSfx(CPed *ped, uint16 sound);
	uint32 GetHFORITalkSfx(CPed *ped, uint16 sound);
	uint32 GetHMYRITalkSfx(CPed *ped, uint16 sound);
	uint32 GetHMORITalkSfx(CPed *ped, uint16 sound);
	uint32 GetHFYBETalkSfx(CPed *ped, uint16 sound);
	uint32 GetHFOBETalkSfx(CPed *ped, uint16 sound);
	uint32 GetHMYBETalkSfx(CPed *ped, uint16 sound);
	uint32 GetHMOBETalkSfx(CPed *ped, uint16 sound);
	uint32 GetHFYBUTalkSfx(CPed *ped, uint16 sound);
	uint32 GetHFYMDTalkSfx(CPed *ped, uint16 sound);
	uint32 GetHFYCGTalkSfx(CPed *ped, uint16 sound);
	uint32 GetHFYPRTalkSfx(CPed *ped, uint16 sound);
	uint32 GetHFOTRTalkSfx(CPed *ped, uint16 sound);
	uint32 GetHMOTRTalkSfx(CPed *ped, uint16 sound);
	uint32 GetHMOCATalkSfx(CPed *ped, uint16 sound);
	uint32 GetBMYCRTalkSfx(CPed *ped, uint16 sound);
	uint32 GetBFYSTTalkSfx(CPed *ped, uint16 sound);
	uint32 GetBFOSTTalkSfx(CPed *ped, uint16 sound);
	uint32 GetBMYSTTalkSfx(CPed *ped, uint16 sound);
	uint32 GetBMOSTTalkSfx(CPed *ped, uint16 sound);
	uint32 GetBFYRITalkSfx(CPed *ped, uint16 sound);
	uint32 GetBFORITalkSfx(CPed *ped, uint16 sound);
	uint32 GetBMYRITalkSfx(CPed *ped, uint16 sound);
	uint32 GetBFYBETalkSfx(CPed *ped, uint16 sound);
	uint32 GetBMYBETalkSfx(CPed *ped, uint16 sound);
	uint32 GetBFOBETalkSfx(CPed *ped, uint16 sound);
	uint32 GetBMOBETalkSfx(CPed *ped, uint16 sound);
	uint32 GetBMYBUTalkSfx(CPed *ped, uint16 sound);
	uint32 GetBFYPRTalkSfx(CPed *ped, uint16 sound);
	uint32 GetBFOTRTalkSfx(CPed *ped, uint16 sound);
	uint32 GetBMOTRTalkSfx(CPed *ped, uint16 sound);
	uint32 GetBMYPITalkSfx(CPed *ped, uint16 sound);
	uint32 GetBMYBBTalkSfx(CPed *ped, uint16 sound);
	uint32 GetWMYCRTalkSfx(CPed *ped, uint16 sound);
	uint32 GetWFYSTTalkSfx(CPed *ped, uint16 sound);
	uint32 GetWFYSKTalkSfx(CPed *ped, uint16 sound);
	uint32 GetWMYSKTalkSfx(CPed *ped, uint16 sound);
	uint32 GetWFOSTTalkSfx(CPed *ped, uint16 sound);
	uint32 GetWMYSTTalkSfx(CPed *ped, uint16 sound);
	uint32 GetWMOSTTalkSfx(CPed *ped, uint16 sound);
	uint32 GetWFYRITalkSfx(CPed *ped, uint16 sound);
	uint32 GetWFORITalkSfx(CPed *ped, uint16 sound);
	uint32 GetWMYRITalkSfx(CPed *ped, uint16 sound);
	uint32 GetWMORITalkSfx(CPed *ped, uint16 sound);
	uint32 GetWFYBETalkSfx(CPed *ped, uint16 sound);
	uint32 GetWMYBETalkSfx(CPed *ped, uint16 sound);
	uint32 GetWFOBETalkSfx(CPed *ped, uint16 sound);
	uint32 GetWMOBETalkSfx(CPed *ped, uint16 sound);
	uint32 GetWMYCWTalkSfx(CPed *ped, uint16 sound);
	uint32 GetWMYGOTalkSfx(CPed *ped, uint16 sound);
	uint32 GetWFOGOTalkSfx(CPed *ped, uint16 sound);
	uint32 GetWMOGOTalkSfx(CPed *ped, uint16 sound);
	uint32 GetWFYLGTalkSfx(CPed *ped, uint16 sound);
	uint32 GetWMYLGTalkSfx(CPed *ped, uint16 sound);
	uint32 GetWFYBUTalkSfx(CPed *ped, uint16 sound);
	uint32 GetWMYBUTalkSfx(CPed *ped, uint16 sound);
	uint32 GetWMOBUTalkSfx(CPed *ped, uint16 sound);
	uint32 GetWFYPRTalkSfx(CPed *ped, uint16 sound);
	uint32 GetWFOTRTalkSfx(CPed *ped, uint16 sound);
	uint32 GetWMOTRTalkSfx(CPed *ped, uint16 sound);
	uint32 GetWMYPITalkSfx(CPed *ped, uint16 sound);
	uint32 GetWMOCATalkSfx(CPed *ped, uint16 sound);
	uint32 GetWFYSHTalkSfx(CPed *ped, uint16 sound);
	uint32 GetWFOSHTalkSfx(CPed *ped, uint16 sound);
	uint32 GetJFOTOTalkSfx(CPed *ped, uint16 sound);
	uint32 GetJMOTOTalkSfx(CPed *ped, uint16 sound);
	uint32 GetHNTalkSfx(CPed *ped, uint16 sound);
	uint32 GetBKTalkSfx(CPed *ped, uint16 sound);
	uint32 GetCBTalkSfx(CPed *ped, uint16 sound);
	uint32 GetSGTalkSfx(CPed *ped, uint16 sound);
	uint32 GetCLTalkSfx(CPed *ped, uint16 sound);
	uint32 GetGDTalkSfx(CPed *ped, uint16 sound);
	uint32 GetPGTalkSfx(CPed *ped, uint16 sound);
	uint32 GetViceWhiteTalkSfx(CPed *ped, uint16 sound);
	uint32 GetViceBlackTalkSfx(CPed *ped, uint16 sound);
	uint32 GetBMODKTalkSfx(CPed *ped, uint16 sound);
	uint32 GetHMYAPTalkSfx(CPed *ped, uint16 sound);
	uint32 GetWFYJGTalkSfx(CPed *ped, uint16 sound);
	uint32 GetWMYJGTalkSfx(CPed *ped, uint16 sound);
	uint32 GetSpecialCharacterTalkSfx(CPed *ped, int32 model, uint16 sound);

	void DebugPlayPedComment(int32 sound);

	// particles
	void ProcessExplosions(int32 explosion);
	void ProcessFires(int32 entity);
	void ProcessWaterCannon(int32);

	// script objects
	void ProcessScriptObject(int32 id);
	void ProcessOneShotScriptObject(uint8 sound);
	void ProcessLoopingScriptObject(uint8 sound);

	// misc
	void ProcessWeather(int32 id);
	void ProcessFrontEnd();
	//void ProcessCrane();
	void ProcessProjectiles();
	void ProcessEscalators();
	void ProcessExtraSounds();
	void ProcessGarages();
	void ProcessFireHydrant();

#ifdef GTA_BRIDGE
	void ProcessBridge();
	void ProcessBridgeWarning();
	void ProcessBridgeMotor();
	void ProcessBridgeOneShots();
#endif

	// mission audio

	// police radio
		
	// collision stuff

	float Sqrt(float v) const { return v <= 0.0f ? 0.0f : ::Sqrt(v); }
#endif
} cAudioManager;

#ifdef __cplusplus
extern "C" {
#endif
int32_t AudioManager_CreateEntity(cAudioManager *manager, enum eAudioType type, void *entity);
uint8_t
AudioMission_GetLoadingStatus(cAudioManager *manager, uint8_t slot);
void
AudioMission_AllowPlay(cAudioManager *manager, uint8_t slot);
uint8_t
AudioMission_ShouldDuck(cAudioManager *manager, uint8_t slot);
uint8_t
AudioMission_IsPlaying(cAudioManager *manager, uint8_t slot);
uint8_t
AudioMission_IsFinished(cAudioManager *manager, uint8_t slot);
void
AudioMission_Clear(cAudioManager *manager, uint8_t slot);
uint8_t
AudioMission_UsesPoliceChannel(uint32_t soundMission);
extern uint8_t g_bMissionAudioLoadFailed[MISSION_AUDIO_SLOTS];
uint32_t AudioMission_FindSfx(const char *name);
const char *AudioMission_GetLoadedLabel(cAudioManager *manager, uint8_t slot);
void AudioMission_Preload(cAudioManager *manager, uint8_t slot, const char *name);
void AudioMission_ProcessSlot(cAudioManager *manager, uint8_t slot);
void AudioMission_Process(cAudioManager *manager);
void AudioPolice_Init(cAudioManager *manager);
void AudioPolice_Reset(cAudioManager *manager);
void AudioPolice_Crackle(cAudioManager *manager);
void AudioPolice_ServiceChannel(cAudioManager *manager, uint8_t wantedLevel);
#ifdef FIX_BUGS
uint32_t AudioPoliceHost_LogicalFramesPassed(void);
#endif
#ifdef USE_TIME_SCALE_FOR_AUDIO
float AudioPoliceHost_TimeScale(void);
#endif
void AudioPolice_Service(cAudioManager *manager);
uint8_t AudioPolice_SetupCrimeReport(cAudioManager *manager);
void AudioPolice_PlaySuspect(cAudioManager *manager, float x, float y, float z);
void AudioPolice_SuspectReport(cAudioManager *manager);
void AudioPolice_ReportCrime(cAudioManager *manager, enum eCrimeType type, const CVector *position);
void AudioRequests_Submit(cAudioManager *manager);
#ifdef AUDIO_REFLECTIONS
void AudioRequests_AddReflections(cAudioManager *manager);
uint8_t AudioRequestsHost_InRoom(void);
#ifndef USE_TIME_SCALE_FOR_AUDIO
uint8_t AudioRequestsHost_SlowMotion(void);
#endif
#endif
#ifdef AUDIO_REFLECTIONS
void AudioReflections_Update(cAudioManager *manager);
uint8_t AudioReflectionsHost_Line(const AudioSoundPosition *start, const AudioSoundPosition *end, AudioSoundPosition *hit);
uint8_t AudioReflectionsHost_Vertical(const AudioSoundPosition *start, float endZ, float *hitZ);
#endif
void AudioReload_ResetTimers(cAudioManager *manager, uint32_t time);
void AudioReload_DestroyEntities(cAudioManager *manager);
void AudioReloadHost_ResetMusic(void);
void AudioLifecycle_Initialise(cAudioManager *manager);
void AudioLifecycle_Terminate(cAudioManager *manager);
void AudioLifecycleHost_PreInitialise(cAudioManager *manager);
void AudioLifecycleHost_PostInitialise(cAudioManager *manager);
void AudioLifecycleHost_PreTerminate(cAudioManager *manager);
void AudioLifecycleHost_PostTerminate(cAudioManager *manager);
void AudioLifecycleHost_MusicInitialise(void);
void AudioLifecycleHost_MusicTerminate(void);
void AudioService_Run(cAudioManager *manager);
void AudioService_ResetLogicTimers(cAudioManager *manager, uint32_t time);
void AudioServiceHost_ResetMusicTimers(uint32_t time);
uint8_t AudioServiceHost_UserPaused(void);
void AudioServiceHost_Music(void);
uint8_t AudioServiceHost_IsPed(CPed *ped);
void AudioServiceHost_PedLastStart(CPed *ped, uint32_t time);
void AudioServiceHost_PedSoundStart(CPed *ped, uint32_t time);
void AudioEffects_Service(cAudioManager *manager);
void AudioEffects_Interrogate(cAudioManager *manager);
void AudioRelease_Add(cAudioManager *manager);
float AudioReleaseHost_TimeStep(void);
void AudioReleaseHost_Position(void *entity, AudioSoundPosition *position);
uint8_t AudioReleaseHost_LineClear(const AudioSoundPosition *position);
void AudioActive_Process(cAudioManager *manager);
uint8_t AudioActiveHost_CameraSwitched(void);
void AudioEnvironment_Reverb(cAudioManager *manager);
void AudioEnvironment_Special(cAudioManager *manager);
uint8_t AudioEnvironmentHost_Replay(void);
void AudioEnvironmentHost_Mood(cAudioManager *manager);
CVehicle *AudioEnvironmentHost_Remote(void);
int32_t AudioEnvironmentHost_EntityId(const CPlayerPed *player);
uint8_t AudioEnvironmentHost_Entering(CPlayerPed *player);
uint8_t AudioEnvironmentHost_InVehicle(const CPlayerPed *player);
void AudioDispatch_Entity(cAudioManager *manager, int32_t id);
void AudioDispatch_Physical(cAudioManager *manager, int32_t id);
int32_t AudioDispatchHost_Area(void);
int32_t AudioDispatchHost_PhysicalType(void *entity);
void AudioDispatchHost_Vehicle(cAudioManager *manager, void *entity);
void AudioDispatchHost_Ped(cAudioManager *manager, void *entity);
void AudioDispatchHost_ProcessExplosions(cAudioManager *manager, int32_t id);
void AudioDispatchHost_ProcessFires(cAudioManager *manager, int32_t id);
void AudioDispatchHost_ProcessWeather(cAudioManager *manager, int32_t id);
void AudioDispatchHost_ProcessScriptObject(cAudioManager *manager, int32_t id);
void AudioDispatchHost_ProcessWaterCannon(cAudioManager *manager, int32_t id);
void AudioDispatchHost_ProcessFrontEnd(cAudioManager *manager);
void AudioDispatchHost_ProcessProjectiles(cAudioManager *manager);
void AudioDispatchHost_ProcessGarages(cAudioManager *manager);
void AudioDispatchHost_ProcessFireHydrant(cAudioManager *manager);
void AudioDispatchHost_ProcessEscalators(cAudioManager *manager);
void AudioDispatchHost_ProcessExtraSounds(cAudioManager *manager);
#ifdef GTA_BRIDGE
void AudioDispatchHost_ProcessBridge(cAudioManager *manager);
#endif
void AudioCollisionService_Run(cAudioManager *manager);
void AudioCollisionReport_Report(cAudioManager *manager, CEntity *entity1, CEntity *entity2, uint8_t surface1, uint8_t surface2, float collisionPower, float velocity);
uint8_t AudioCollisionReportHost_IsBuilding(CEntity *entity);
void AudioCollisionReportHost_Position(CEntity *entity, AudioSoundPosition *position);
void AudioCollisionSounds_OneShot(cAudioManager *manager, const cAudioCollision *col);
void AudioCollisionSounds_Looping(cAudioManager *manager, const cAudioCollision *col, uint8_t counter);
uint32_t AudioCollisionSounds_LoopingVolume(cAudioManager *manager, const cAudioCollision *audioCollision);
void AudioVehicleRain_Process(cAudioManager *manager, cVehicleParams *params);
float AudioVehicleRainHost_Rain(void);
uint8_t AudioVehicleRainHost_CameraNoRain(void);
uint8_t AudioVehicleRainHost_PlayerNoRain(void);
void AudioVehicleRainHost_Counters(CVehicle *vehicle, uint8_t **audioCounter, uint8_t **sampleCounter);
uint8_t AudioVehicleRoad_FlatTyre(cAudioManager *manager, cVehicleParams *params);
uint8_t AudioVehicleRoad_Dry(cAudioManager *manager, cVehicleParams *params);
uint8_t AudioVehicleRoad_Wet(cAudioManager *manager, cVehicleParams *params);
uint8_t AudioVehicleRoadHost_WheelStatus(CVehicle *vehicle, int32_t type, int32_t wheel);
float AudioVehicleRoadHost_WheelTimer(CVehicle *vehicle, int32_t type, int32_t wheel);
uint8_t AudioVehicleRoadHost_WheelsOnGround(CVehicle *vehicle, int32_t type);
float AudioVehicleRoadHost_MaxVelocity(const cTransmission *transmission);
uint8_t AudioVehicleRoadHost_Surface(const CVehicle *vehicle);
float AudioVehicleRoadHost_WetRoads(void);
void AudioManager_InitState(cAudioManager *manager);
void AudioManager_GenerateRandomTable(int32_t randomTable[5]);
#ifdef __cplusplus
}
#endif

/*
   Manual loop points are not on PS2 so let's have these macros to avoid massive ifndefs.
   Setting these manually was pointless anyway since they never change from sdt values.
   What were they thinking?
*/
#ifndef GTA_PS2
#define RESET_LOOP_OFFSETS \
	m_sQueueSample.m_nLoopStart = 0; \
	m_sQueueSample.m_nLoopEnd = -1;
#define SET_LOOP_OFFSETS(sample) \
	m_sQueueSample.m_nLoopStart = SampleManager_GetSampleLoopStartOffset(&SampleManager, sample); \
	m_sQueueSample.m_nLoopEnd = SampleManager_GetSampleLoopEndOffset(&SampleManager, sample);
#else
#define RESET_LOOP_OFFSETS
#define SET_LOOP_OFFSETS(sample)
#endif
#ifdef EXTERNAL_3D_SOUND
#define SET_EMITTING_VOLUME(vol) m_sQueueSample.m_nEmittingVolume = vol
#else
#define SET_EMITTING_VOLUME(vol)
#endif
#ifdef AUDIO_REFLECTIONS
#define SET_SOUND_REFLECTION(b) m_sQueueSample.m_bReflections = b
#else
#define SET_SOUND_REFLECTION(b)
#endif
#ifdef AUDIO_REVERB
#define SET_SOUND_REVERB(b) m_sQueueSample.m_bReverb = b
#else
#define SET_SOUND_REVERB(b)
#endif

#if defined(AUDIO_MSS) && !defined(PS2_AUDIO_CHANNELS)
#ifdef __cplusplus
static_assert(sizeof(cAudioManager) == 0x5558, "cAudioManager: error");
#else
_Static_assert(sizeof(cAudioManager) == 0x5558, "cAudioManager: error");
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif
extern cAudioManager AudioManager;
#ifdef __cplusplus
}
#endif

enum
{
	PED_COMMENT_VOLUME = 127,
	PED_COMMENT_VOLUME_BEHIND_WALL = 31,
	COLLISION_MAX_DIST = 60,
};
//- rouz edit (ChatGPT)
