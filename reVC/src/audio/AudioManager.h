//+ rouz edit (ChatGPT)
#pragma once
#include "../vehicles/WheelState.h"
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
#ifndef USE_TIME_SCALE_FOR_AUDIO
#endif
#endif
#ifdef AUDIO_REFLECTIONS
void AudioReflections_Update(cAudioManager *manager);
uint8_t AudioReflectionsHost_Line(const AudioSoundPosition *start, const AudioSoundPosition *end, AudioSoundPosition *hit);
uint8_t AudioReflectionsHost_Vertical(const AudioSoundPosition *start, float endZ, float *hitZ);
#endif
void AudioReload_ResetTimers(cAudioManager *manager, uint32_t time);
void AudioReload_DestroyEntities(cAudioManager *manager);
void AudioLifecycle_Initialise(cAudioManager *manager);
void AudioLifecycle_Terminate(cAudioManager *manager);
void AudioService_Run(cAudioManager *manager);
void AudioService_ResetLogicTimers(cAudioManager *manager, uint32_t time);
void AudioServiceHost_ResetMusicTimers(uint32_t time);
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
CVehicle *AudioEnvironmentHost_Remote(void);
int32_t AudioEnvironmentHost_EntityId(const CPlayerPed *player);
uint8_t AudioEnvironmentHost_Entering(CPlayerPed *player);
uint8_t AudioEnvironmentHost_InVehicle(const CPlayerPed *player);
void AudioDispatch_Entity(cAudioManager *manager, int32_t id);
void AudioDispatch_Physical(cAudioManager *manager, int32_t id);
int32_t AudioDispatchHost_Area(void);
int32_t AudioDispatchHost_PhysicalType(void *entity);
#ifdef GTA_BRIDGE
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
void AudioVehicleRainHost_Counters(CVehicle *vehicle, uint8_t **audioCounter, uint8_t **sampleCounter);
uint8_t AudioVehicleRoad_FlatTyre(cAudioManager *manager, cVehicleParams *params);
uint8_t AudioVehicleRoad_Dry(cAudioManager *manager, cVehicleParams *params);
uint8_t AudioVehicleRoad_Wet(cAudioManager *manager, cVehicleParams *params);
uint8_t AudioVehicleRoadHost_WheelStatus(CVehicle *vehicle, int32_t type, int32_t wheel);
float AudioVehicleRoadHost_WheelTimer(CVehicle *vehicle, int32_t type, int32_t wheel);
uint8_t AudioVehicleRoadHost_WheelsOnGround(CVehicle *vehicle, int32_t type);
uint8_t AudioVehicleRoadHost_Surface(const CVehicle *vehicle);
float AudioVehicleRoadHost_WetRoads(void);
uint8_t AudioVehicleReverse_Process(cAudioManager *manager, cVehicleParams *params);
int16_t AudioVehicleReverseHost_Model(const CVehicle *vehicle);
uint8_t AudioVehicleReverseHost_Engine(const CVehicle *vehicle);
float AudioVehicleReverseHost_Gas(const CVehicle *vehicle);
uint8_t AudioVehicleReverseHost_Gear(const CVehicle *vehicle);
uint8_t AudioVehicleReverseHost_DriveWheels(const CVehicle *vehicle);
uint8_t AudioVehicleReverseHost_PreviousDriveWheels(const CVehicle *vehicle);
float *AudioVehicleReverseHost_GasAudio(CVehicle *vehicle);
void AudioVehicleModel_Heli(cAudioManager *manager, cVehicleParams *params);
void AudioVehicleModel_Car(cAudioManager *manager, cVehicleParams *params);
int16_t AudioVehicleModelHost_Model(const CVehicle *vehicle);
float AudioVehicleModelHost_BrakePedal(const CVehicle *vehicle);
uint8_t AudioVehicleModelHost_Drowning(const CVehicle *vehicle);
int16_t AudioVehicleModelHost_Brake(void);
int16_t AudioVehicleModelHost_Accelerate(void);
int16_t AudioVehicleModelHost_GunUpDown(void);
#ifdef THIS_IS_STUPID
void AudioVehicleModelHost_Up(CVehicle *vehicle, AudioSoundPosition *up);
#else
float AudioVehicleModelHost_UpY(CVehicle *vehicle);
#endif
uint8_t AudioVehicleEngine_Process(cAudioManager *manager, cVehicleParams *params);
int32_t AudioVehicleEngineHost_Status(const CVehicle *vehicle);
uint8_t AudioVehicleEngineHost_Handbrake(const CVehicle *vehicle);
uint8_t AudioVehicleEngineHost_BikeWheels(const CVehicle *vehicle);
uint8_t AudioVehicleEngineHost_BikePreviousWheels(const CVehicle *vehicle);
float * AudioVehicleEngineHost_BikeGasAudio(CVehicle *vehicle);
enum tWheelState * AudioVehicleEngineHost_CarStates(CVehicle *vehicle);
enum tWheelState * AudioVehicleEngineHost_BikeStates(CVehicle *vehicle);
uint8_t AudioVehicleEngineHost_Bank(int32_t index);
int32_t AudioVehicleEngineHost_Acceleration(int32_t index);
void AudioVehicleEngineHost_Unknown(int32_t type);
void AudioVehiclePlayer_UpdateGas(CVehicle *vehicle, int32_t type);
void AudioVehiclePlayer_AddSample(cAudioManager *manager, uint8_t volume, uint32_t frequency, uint32_t sample, uint8_t bank, uint8_t counter, uint8_t looping);
void AudioPlayerEngine_Process(cAudioManager *manager, cVehicleParams *params, CVehicle *vehicle);
void AudioPlayerEngine_Entered(cAudioManager *manager);
void AudioPlayerEngine_Left(cAudioManager *manager);
float AudioPlayerEngineHost_CarVelocity(const CVehicle *vehicle);
float AudioPlayerEngineHost_BikeVelocity(const CVehicle *vehicle);
uint8_t AudioPlayerEngineHost_ExtraSpeed(const CVehicle *vehicle);
uint8_t AudioPlayerEngineHost_Sand(const CVehicle *vehicle);
float AudioPlayerEngineHost_SpeedZ(const CVehicle *vehicle);
uint32_t AudioPlayerEngineHost_Time(void);
#ifdef FIX_BUGS
uint8_t AudioPlayerEngineHost_Exiting(void);
#endif
void AudioPlayerEngineHost_ChangingGear(CVehicle *vehicle);
uint8_t AudioVehicleSkid_Process(cAudioManager *manager, cVehicleParams *params);
float AudioVehicleSkidHost_CarGas(const CVehicle *vehicle);
float AudioVehicleSkidHost_BikeGas(const CVehicle *vehicle);
float AudioVehicleSkidHost_TurnZ(const CVehicle *vehicle);
void AudioVehicleSkidHost_Unknown(int32_t type);
uint8_t AudioVehicleHorn_Horn(cAudioManager *manager, cVehicleParams *params);
uint8_t AudioVehicleHorn_Siren(cAudioManager *manager, cVehicleParams *params);
uint8_t AudioVehicleHornHost_Enabled(const CVehicle *vehicle);
uint8_t AudioVehicleHornHost_Alarm(CVehicle *vehicle);
uint8_t AudioVehicleHornHost_Uses(CVehicle *vehicle);
uint32_t AudioVehicleHornHost_Timer(const CVehicle *vehicle);
uint8_t AudioVehicleHornHost_Pattern(const CVehicle *vehicle);
int32_t AudioVehicleHornHost_HornSample(uint32_t index);
int32_t AudioVehicleHornHost_HornFrequency(uint32_t index);
uint8_t AudioVehicleHornHost_SirenSample(uint32_t index);
int32_t AudioVehicleHornHost_SirenFrequency(uint32_t index);
void AudioVehicleHornHost_SetTimer(CVehicle *vehicle, uint32_t value);
void AudioVehicleHornHost_SetPattern(CVehicle *vehicle, uint8_t value);
uint8_t AudioVehicleWarning_UsesReverse(uint32_t model);
uint8_t AudioVehicleWarning_Reverse(cAudioManager *manager, cVehicleParams *params);
uint8_t AudioVehicleWarning_AirBrakes(cAudioManager *manager, cVehicleParams *params);
uint8_t AudioVehicleWarning_HasAirBrakes(uint32_t model);
uint8_t AudioVehicleDoors_Process(cAudioManager *manager, cVehicleParams *params);
int32_t AudioVehicleDoorsHost_Status(CVehicle *vehicle, uint8_t door);
int8_t AudioVehicleDoorsHost_State(const CVehicle *vehicle, uint8_t door);
float AudioVehicleDoorsHost_Velocity(const CVehicle *vehicle, uint8_t door);
uint8_t AudioVehicleCondition_Damage(cAudioManager *manager, cVehicleParams *params);
uint8_t AudioVehicleCondition_BombTick(cAudioManager *manager, cVehicleParams *params);
float AudioVehicleConditionHost_Health(const CVehicle *vehicle);
uint8_t AudioVehicleConditionHost_Bomb(const CVehicle *vehicle);
void AudioVehicleConditionHost_Unknown(int32_t type);
uint8_t AudioBoat_Engine(cAudioManager *manager, cVehicleParams *params);
uint8_t AudioBoat_Water(cAudioManager *manager, cVehicleParams *params);
uint8_t AudioBoatHost_Propeller(const CVehicle *vehicle);
uint8_t AudioBoatHost_InWater(const CVehicle *vehicle);
uint8_t AudioAircraft_Heli(cAudioManager *manager, cVehicleParams *params);
void AudioAircraft_Cesna(cAudioManager *manager, cVehicleParams *params);
float AudioAircraftHost_MovingSpeed(const CVehicle *vehicle);
float AudioAircraftHost_WheelSpeed(const CVehicle *vehicle);
void AudioAircraftHost_Forward(CVehicle *vehicle, AudioSoundPosition *forward);
void AudioAircraftHost_Boot(CVehicle *vehicle, AudioSoundPosition *position);
void AudioAircraftHost_Matrix(CVehicle *vehicle, AudioCameraMatrix *matrix);
uint8_t AudioAircraftHost_FixedCamera(void);
#ifdef GTA_PS2
void AudioAircraftHost_CameraBegin(AudioCameraMatrix *matrix, void **attachment, uint8_t *owned);
void AudioAircraftHost_CameraEnd(void *attachment, uint8_t owned);
#endif
void AudioPlane_Plane(cAudioManager *manager, cVehicleParams *params);
void AudioPlane_Jumbo(cAudioManager *manager, cVehicleParams *params);
void AudioPlane_JumboTaxi(cAudioManager *manager);
void AudioPlane_JumboAccel(cAudioManager *manager, CVehicle *plane);
void AudioPlane_JumboTakeOff(cAudioManager *manager, CVehicle *plane);
void AudioPlane_JumboFlying(cAudioManager *manager);
void AudioPlane_JumboLanding(cAudioManager *manager, CVehicle *plane);
void AudioPlane_JumboDecel(cAudioManager *manager, CVehicle *plane);
uint8_t AudioPlane_JumboTaxiSound(cAudioManager *manager, uint8_t vol);
uint8_t AudioPlane_JumboWhineSound(cAudioManager *manager, uint8_t Vol, uint32_t freq);
uint8_t AudioPlane_JumboEngineSound(cAudioManager *manager, uint8_t Vol, uint32_t freq);
uint8_t AudioPlane_JumboFlySound(cAudioManager *manager, uint8_t Vol);
uint8_t AudioPlane_JumboRumbleSound(cAudioManager *manager, uint8_t Vol);
int32_t AudioPlane_JumboTaxiFreq(cAudioManager *manager);
float AudioPlaneHost_Speed(const CVehicle *vehicle);
float AudioPlaneHost_Position(const CVehicle *vehicle);
float AudioPlaneHost_TakeOffPoint(void);
float AudioPlaneHost_LandingPoint(void);
void AudioVehicle_Process(cAudioManager *manager, CVehicle *vehicle);
void *AudioVehicleHost_Handling(CVehicle *vehicle);
cTransmission *AudioVehicleHost_Transmission(void *handling);
float AudioVehicleHost_TrafficSpeed(CVehicle *vehicle);
void AudioVehicleHost_VelocityVectors(CVehicle *vehicle, AudioSoundPosition *speed, AudioSoundPosition *direction);
int32_t AudioVehicleHost_Type(CVehicle *vehicle);
int32_t AudioVehicleHost_Appearance(CVehicle *vehicle);
void AudioVehicleHost_CarVelocity(CVehicle *vehicle, float velocity);
void AudioVehicleHost_BikeVelocity(CVehicle *vehicle, float velocity);
void AudioVehicleOneShots_Process(cAudioManager *manager, cVehicleParams *params);
uint8_t AudioVehicleOneShotsHost_DoorType(uint32_t index);
int16_t AudioVehicleOneShotsHost_HandBrake(void);
CPed *AudioVehicleOneShotsHost_Driver(const CVehicle *vehicle);
uint8_t AudioVehicleOneShotsHost_HasWeapon(CPed *ped, uint8_t slot);
uint8_t AudioVehicleOneShotsHost_DriveBy(CPed *ped);
int32_t AudioVehicleOneShotsHost_Weapon(CPed *ped, uint8_t slot);
#ifdef GTA_TRAIN
uint8_t AudioTrain_Process(cAudioManager *manager, cVehicleParams *params);
float AudioTrainHost_Speed(const CVehicle *vehicle);
float AudioTrainHost_WagonPosition(const CVehicle *vehicle);
#endif
void AudioPed_Process(cAudioManager *manager, CPed *ped);
#ifdef __cplusplus
class CWeapon;
#else
typedef struct CWeapon CWeapon;
#endif
void AudioPedOneShots_Process(cAudioManager *manager, cPedParams *params);
CEntity * AudioPedOneShotsHost_Entity(CPed *ped);
CWeapon * AudioPedOneShotsHost_Weapon(CPed *ped);
int32_t AudioPedOneShotsHost_WeaponType(const CWeapon *weapon);
uint8_t AudioPedOneShotsHost_IsPlayer(CPed *ped);
CVehicle * AudioPedOneShotsHost_Vehicle(const CPed *ped);
uint8_t AudioPedOneShotsHost_InAir(const CPed *ped);
uint8_t AudioPedOneShotsHost_Surface(const CPed *ped);
int32_t AudioPedOneShotsHost_MoveState(const CPed *ped);
uint32_t AudioPedOneShotsHost_FightMove(const CPed *ped);
int32_t AudioPedOneShotsHost_State(const CPed *ped);
void * AudioPedOneShotsHost_DamageEntity(const CPed *ped);
uint8_t AudioPedOneShotsHost_EntityType(const void *entity);
CPed * AudioPedOneShotsHost_Player(void);
void AudioPedSpeech_SetTalking(CPed *ped, uint8_t status);
void AudioPedSpeech_SetMood(cAudioManager *manager, uint8_t mood, uint32_t time);
void AudioPedSpeech_Mood(cAudioManager *manager);
void AudioPedSpeech_Setup(cAudioManager *manager, cPedParams *params, uint16_t sound);
void AudioPedSpeechHost_SetTalking(CPed *ped, uint8_t status);
uint8_t AudioPedSpeechHost_CanTalk(const CPed *ped);
uint8_t AudioPedSpeechHost_LineClear(const AudioSoundPosition *position);
uint32_t AudioPedSpeechHost_MissionTime(void);
void AudioPedSpeechHost_SetMissionTime(uint32_t time);
uint32_t AudioPedTalk_Select(cAudioManager *manager, CPed *ped, uint16_t sound);
void AudioPedTalk_Phrase(cAudioManager *manager, uint32_t *phrase, uint32_t *prevPhrase, uint32_t sample, uint32_t maxOffset);
uint32_t AudioPedTalkHost_GetUpTimer(const CPed *ped);
int16_t AudioPedTalkHost_Model(const CPed *ped);
uint32_t *AudioPedTalkHost_LastComment(CPed *ped);
int32_t AudioPedTalkHost_PlayerState(void);
uint32_t AudioPedTalk_Player(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_GenericMale(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_GenericFemale(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_Default(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_Cop(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_Swat(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_FBI(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_Army(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_Medic(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_Fireman(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_WFYG1(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_WFYG2(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_HFYST(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_HFOST(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_HMYST(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_HMOST(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_HFYRI(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_HFORI(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_HMYRI(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_HMORI(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_HFYBE(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_HFOBE(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_HMYBE(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_HMOBE(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_HFYBU(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_HFYMD(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_HFYCG(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_HFYPR(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_HFOTR(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_HMOTR(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_HMOCA(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_BMYCR(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_BFYST(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_BFOST(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_BMYST(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_BMOST(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_BFYRI(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_BFORI(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_BMYRI(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_BFYBE(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_BMYBE(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_BFOBE(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_BMOBE(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_BMYBU(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_BFYPR(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_BFOTR(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_BMOTR(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_BMYPI(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_BMYBB(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_WMYCR(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_WFYST(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_WFYSK(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_WMYSK(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_WFOST(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_WMYST(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_WMOST(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_WFYRI(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_WFORI(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_WMYRI(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_WMORI(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_WFYBE(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_WMYBE(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_WFOBE(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_WMOBE(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_WMYCW(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_WMYGO(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_WFOGO(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_WMOGO(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_WFYLG(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_WMYLG(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_WFYBU(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_WMYBU(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_WMOBU(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_WFYPR(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_WFOTR(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_WMOTR(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_WMYPI(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_WMOCA(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_WFYSH(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_WFOSH(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_JFOTO(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_JMOTO(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_HN(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_BK(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_CB(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_SG(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_CL(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_GD(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_PG(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_ViceWhite(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_ViceBlack(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_BMODK(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_HMYAP(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_WFYJG(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_WMYJG(cAudioManager *manager, CPed *ped, uint16_t sound);
uint32_t AudioPedTalk_SpecialCharacter(cAudioManager *manager, CPed *ped, int32_t model, uint16_t sound);
void AudioPedPlayback_Debug(cAudioManager *manager, int32_t sound);
void AudioPedPlaybackHost_DebugPosition(PedCommentPosition *position);
void AudioExplosions_Process(cAudioManager *manager, int32_t id);
void AudioFires_Process(cAudioManager *manager, int32_t id);
uint8_t AudioFiresHost_Ongoing(uint8_t id);
uint8_t AudioFiresHost_AudioSet(uint8_t id);
CEntity * AudioFiresHost_Entity(uint8_t id);
void AudioFiresHost_Position(uint8_t id, AudioSoundPosition *position);
uint8_t AudioFiresHost_Water(uint8_t id);
void AudioFiresHost_ClearWater(uint8_t id);
float AudioFiresHost_Countdown(uint8_t id);
void AudioWaterCannon_Process(cAudioManager *manager, int32_t id);
int32_t AudioWaterCannonHost_Id(uint32_t index);
void AudioWaterCannonHost_Position(uint32_t index, AudioSoundPosition *position);
void AudioScriptSounds_Process(cAudioManager *manager, int32_t id);
void AudioScriptSounds_OneShot(cAudioManager *manager, uint8_t sound);
void AudioScriptSounds_Looping(cAudioManager *manager, uint8_t sound);
void AudioWeather_Process(cAudioManager *manager, int32_t id);
void AudioFrontEnd_Process(cAudioManager *manager);
float AudioWeatherHost_SoundDistUp(void);
void AudioWeatherHost_WindModifier(float *x, float *y);
float AudioWeatherHost_Wind(void);
float AudioWeatherHost_NearestTree(void);
void AudioWeatherHost_ResetNearestTree(void);
void AudioWorldLoops_Projectiles(cAudioManager *manager);
void AudioWorldLoops_Escalators(cAudioManager *manager);
void AudioWorldLoops_ExtraSounds(cAudioManager *manager);
uint8_t AudioWorldLoopsHost_ProjectileInUse(uint8_t index);
int32_t AudioWorldLoopsHost_ProjectileWeapon(uint8_t index);
void AudioWorldLoopsHost_ProjectilePosition(uint8_t index, AudioSoundPosition *position);
int32_t AudioWorldLoopsHost_EscalatorCount(void);
uint8_t AudioWorldLoopsHost_EscalatorActive(int32_t index);
void AudioWorldLoopsHost_EscalatorPosition(int32_t index, AudioSoundPosition *position);
extern AudioSoundPosition aVecExtraSoundPosition[2];
void AudioGarageHydrant_Garages(cAudioManager *manager);
void AudioGarageHydrant_Hydrant(cAudioManager *manager);
uint32_t AudioGarageHydrantHost_Count(void);
uint8_t AudioGarageHydrantHost_Type(uint32_t index);
CEntity *AudioGarageHydrantHost_Door(uint32_t index);
uint8_t AudioGarageHydrantHost_State(uint32_t index);
void AudioGarageHydrantHost_Position(void *entity, AudioSoundPosition *position);
#ifdef GTA_BRIDGE
void AudioBridge_Process(cAudioManager *manager);
void AudioBridge_Warning(cAudioManager *manager);
void AudioBridge_Motor(cAudioManager *manager);
void AudioBridge_OneShots(cAudioManager *manager);
CEntity *AudioBridgeHost_Road(void);
int32_t AudioBridgeHost_State(void);
int32_t AudioBridgeHost_OldState(void);
int32_t AudioBridgeHost_CommercialPassed(void);
#endif
void AudioGameSetup_PreInitialise(cAudioManager *manager);
void AudioGameSetup_PostInitialise(cAudioManager *manager);
void AudioGameSetup_PreTerminate(cAudioManager *manager);
void AudioGameSetup_PostTerminate(cAudioManager *manager);
void *AudioGameSetupHost_FireManager(void);
#ifdef GTA_PS2
void AudioBank_LoadIfNecessary(uint8_t bank);
#endif
void AudioOwner_Destroy(cAudioManager *manager);
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
