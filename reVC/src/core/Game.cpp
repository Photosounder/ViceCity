//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
#include "common.h"
#include "ModelRegistry.h" // rouz edit (ChatGPT)
#include "platform.h"

#include "Game.h"
#include "main.h"
#include "RwHelper.h"
#include "Accident.h"
#include "Antennas.h"
#include "Bridge.h"
#include "CarCtrl.h"
#include "CarGen.h"
#include "CdStream.h"
#include "Clock.h"
#include "Clouds.h"
#include "Collision.h"
#include "Console.h"
#include "Coronas.h"
#include "Cranes.h"
#include "Credits.h"
#include "CutsceneMgr.h"
#include "DMAudio.h"
#include "Darkel.h"
#include "Debug.h"
#include "EventList.h"
#include "FileLoader.h"
#include "FileMgr.h"
#include "Fire.h"
#include "Fluff.h"
#include "Font.h"
#include "Frontend.h"
#include "frontendoption.h"
#include "GameLogic.h"
#include "Garages.h"
#include "GenericGameStorage.h"
#include "Glass.h"
#include "HandlingMgr.h"
#include "Heli.h"
#include "Hud.h"
#include "IniFile.h"
#include "Lights.h"
#include "MBlur.h"
#include "Messages.h"
#include "MemoryCard.h"
#include "MemoryHeap.h"
#include "Pad.h"
#include "Pools.h" // rouz edit (ChatGPT)
#include "Particle.h"
#include "ParticleObject.h"
#include "PedRoutes.h"
#include "Phones.h"
#include "Pickups.h"
#include "Plane.h"
#include "PlayerSkin.h"
#include "Population.h"
#include "Radar.h"
#include "Record.h"
#include "References.h"
#include "Renderer.h"
#include "Replay.h"
#include "Restart.h"
#include "RoadBlocks.h"
#include "Rubbish.h"
#include "SceneEdit.h"
#include "Script.h"
#include "Shadows.h"
#include "Skidmarks.h"
#include "SetPieces.h"
#include "SpecialFX.h"
#include "Stats.h"
#include "Streaming.h"
#include "SurfaceTable.h"
#include "TempColModels.h"
#include "Timecycle.h"
#include "TrafficLights.h"
#include "Train.h"
#include "TxdStore.h"
#include "User.h"
#include "VisibilityPlugins.h"
#include "WaterCannon.h"
#include "WaterLevel.h"
#include "Weapon.h"
#include "WeaponEffects.h"
#include "Weather.h"
#include "World.h"
#include "ZoneCull.h"
#include "Zones.h"
#include "Gangs.h"
#include "Occlusion.h"
#include "debugmenu.h"
#include "Ropes.h"
#include "WindModifiers.h"
#include "WaterCreatures.h"
#include "postfx.h"
#include "custompipes.h"
#include "screendroplets.h"
#include "VarConsole.h"
#ifdef USE_TEXTURE_POOL
#include "TexturePools.h"
#endif

eLevelName Game_currLevel;
int32 Game_currArea;
bool Game_bDemoMode = true;
bool Game_nastyGame = true;
bool Game_frenchGame;
bool Game_germanGame;
bool Game_noProstitutes;
bool Game_playingIntro;
char Game_aDatFile[32];
#ifdef MORE_LANGUAGES
bool Game_russianGame = false;
bool Game_japaneseGame = false;
#endif
#ifndef MASTER
CVector Game_PlayerCoords;
bool8 Game_VarUpdatePlayerCoords;
#endif

int gameTxdSlot;

#ifdef SECUROM
uint8 gameProcessPirateCheck = 0;
#endif

bool DoRWStuffStartOfFrame(int16 TopRed, int16 TopGreen, int16 TopBlue, int16 BottomRed, int16 BottomGreen, int16 BottomBlue, int16 Alpha);
void DoRWStuffEndOfFrame(void);
#ifdef PS2_MENU
void MessageScreen(char *msg)
{
	//TODO: stretch_screen
	
	CRect rect = Rect_Make(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	CRGBA color(255, 255, 255, 255);

	DoRWStuffStartOfFrame(50, 50, 50, 0, 0, 0, 255);
	
	// Preserve the original operation through the C sprite interface
	Sprite2d_InitPerFrame();
	Font_InitPerFrame();
	DefinedState();

	Sprite2d *splash = LoadSplash(NULL);
	([](Sprite2d *spriteSelf,const CRect &rect,const CRGBA &c0,const CRGBA &c1,const CRGBA &c2,const CRGBA &c3){
    // Retain actual native reference objects through the complete synchronous C operation
    Sprite2d_DrawRectColours(spriteSelf,&rect,Sprite_ColourRef(&c0.r,&c0.g,&c0.b,&c0.a),Sprite_ColourRef(&c1.r,&c1.g,&c1.b,&c1.a),Sprite_ColourRef(&c2.r,&c2.g,&c2.b,&c2.a),Sprite_ColourRef(&c3.r,&c3.g,&c3.b,&c3.a));
})(splash,rect, color, color, color, color);
	splash->DrawRect(Rect_Make(SCREEN_SCALE_X(20.0f), SCREEN_SCALE_Y(110.0f), SCREEN_SCALE_X(620.0f), SCREEN_SCALE_Y(300.0f)), CRGBA(50, 50, 50, 192));
	
	Font_SetFontStyle(FONT_BANK);
	Font_SetBackgroundOff();
	Font_SetWrapx(SCREEN_SCALE_FROM_RIGHT(190.0f)); // 450.0f
	Font_SetScale(SCREEN_SCALE_X(1.0f), SCREEN_SCALE_Y(1.0f));
	Font_SetCentreOn();
	Font_SetCentreSize(SCREEN_SCALE_X(450.0f));
	Font_SetJustifyOff();
	Font_SetColor(CRGBA(255, 255, 255, 255));
	Font_SetDropColor(CRGBA(32, 32, 32, 255));
	Font_SetDropShadowPosition(3);
	Font_SetPropOn();
	Font_PrintString(SCREEN_SCALE_X(320.0f), SCREEN_SCALE_Y(130.0f), Text_Get(&TheText,msg));
	Font_DrawFonts();
	
	DoRWStuffEndOfFrame();
}
#endif

bool
Game_InitialiseOnceBeforeRW(void)
{
	FileMgr_Initialise();
	CdStreamInit(MAX_CDCHANNELS);
	debug("size of matrix %d\n", sizeof(CMatrix));
	debug("size of placeable %d\n", sizeof(CPlaceable));
	debug("size of entity %d\n", sizeof(CEntity));
	debug("size of building %d\n", sizeof(CBuilding));
	debug("size of dummy %d\n", sizeof(CDummy));
#ifdef EXTENDED_COLOURFILTER
	// Use the complete C post-processing owner and its live shared state
	PostFX_InitOnce();
#endif
#ifdef CUSTOM_FRONTEND_OPTIONS
	// Not needed here but may be needed in future
	// if (numCustomFrontendOptions == 0 && numCustomFrontendScreens == 0)
	CustomFrontendOptionsPopulate();
#endif
	return true;
}

#ifndef LIBRW
#ifdef PS2_MATFX
void ReplaceMatFxCallback();
#endif // PS2_MATFX
#ifdef PS2_ALPHA_TEST
void ReplaceAtomicPipeCallback();
#endif // PS2_ALPHA_TEST
#endif // !LIBRW

bool
Game_InitialiseRenderWare(void)
{
	ValidateVersion();
#ifdef USE_TEXTURE_POOL
	// Use the complete C texture and palette pool owner
	TexturePools_Initialise();
#endif

	// Use the complete C texture dictionary owner with the original live arguments
	TxdStore_Initialise();
	CVisibilityPlugins::Initialise();

#ifdef GTA_PS2
	RpSkySelectTrueTSClipper(TRUE);
	RpSkySelectTrueTLClipper(TRUE);

	// PS2ManagerApplyDirectionalLightingCB() uploads the GTA lights
	// directly without going through RpWorld and all that
	SetupPS2ManagerDefaultLightingCallback();
	PreAllocateRwObjects();
#endif

	/* Create camera */
	// Use the C helper and retain the original typed SDK result
	Scene.camera = ((RwCamera*)CameraCreate(SCREEN_WIDTH, SCREEN_HEIGHT, TRUE));
	ASSERT(Scene.camera != nil);
	if (!Scene.camera)
	{
		return (false);
	}
	
	RwCameraSetFarClipPlane(Scene.camera, 2000.0f);
	RwCameraSetNearClipPlane(Scene.camera, 0.9f);
	
	CameraSize(Scene.camera, nil, DEFAULT_VIEWWINDOW, DEFAULT_ASPECT_RATIO);
	
	/* Create a world */
	RwBBox  bbox;
	
	bbox.sup.x = bbox.sup.y = bbox.sup.z = 10000.0f;
	bbox.inf.x = bbox.inf.y = bbox.inf.z = -10000.0f;

	Scene.world = RpWorldCreate(&bbox);
	ASSERT(Scene.world != nil);
	if (!Scene.world)
	{
		CameraDestroy(Scene.camera);
		Scene.camera = nil;
		return (false);
	}
	
	/* Add the camera to the world */
	RpWorldAddCamera(Scene.world, Scene.camera);
	// Use the complete C lighting owner and its borrowed SDK handles
	Lights_Create(Scene.world);

	CreateDebugFont();

#ifdef LIBRW
#ifdef PS2_MATFX
	rw::MatFX::envMapApplyLight = true;
	rw::MatFX::envMapUseMatColor = true;
	rw::MatFX::envMapFlipU = true;
#else
	rw::MatFX::envMapApplyLight = false;
	rw::MatFX::envMapUseMatColor = false;
	rw::MatFX::envMapFlipU = false;
#endif
	rw::RGBA envcol = { 64, 64, 64, 255 };
	rw::MatFX::envMapColor = envcol;
#else
#ifdef PS2_MATFX
	ReplaceMatFxCallback();
#endif // PS2_MATFX
#ifdef PS2_ALPHA_TEST
	ReplaceAtomicPipeCallback();
#endif // PS2_ALPHA_TEST
#endif // LIBRW

	PUSH_MEMID(MEMID_TEXTURES);
	Font_Initialise();
	Hud_Initialise();
	PlayerSkin_Initialise(); // rouz edit (ChatGPT)
	POP_MEMID();

#ifdef EXTENDED_PIPELINES
	CustomPipes::CustomPipeInit();	// need Scene.world for this
#endif
#ifdef SCREEN_DROPLETS
	ScreenDroplets::InitDraw();
#endif

	return (true);
}

void Game_ShutdownRenderWare(void)
{
#ifdef SCREEN_DROPLETS
	ScreenDroplets::Shutdown();
#endif
#ifdef EXTENDED_PIPELINES
	CustomPipes::CustomPipeShutdown();
#endif

	DestroySplashScreen();
	Hud_Shutdown();
	Font_Shutdown();
	
	for ( int32 i = 0; i < NUMPLAYERS; i++ )
		World_Players[i].DeletePlayerSkin();

	PlayerSkin_Shutdown(); // rouz edit (ChatGPT)
	
	DestroyDebugFont();
	
	/* Destroy world */
	// Use the complete C lighting owner and its borrowed SDK handles
	Lights_Destroy(Scene.world);
	RpWorldRemoveCamera(Scene.world, Scene.camera);
	RpWorldDestroy(Scene.world);
	
	/* destroy camera */
	CameraDestroy(Scene.camera);
	
	Scene.world = nil;
	Scene.camera = nil;
	
	CVisibilityPlugins::Shutdown();
	
#ifdef USE_TEXTURE_POOL
	// Use the complete C texture and palette pool owner
	TexturePools_Shutdown();
#endif
}

bool Game_InitialiseOnceAfterRW(void)
{
	Text_Load(&TheText);
	Timer_Initialise();
	TempColModels_Initialise();
	mod_HandlingManager.Initialise();
	SurfaceTable_Initialise("DATA\\SURFACE.DAT"); // rouz edit (ChatGPT)
	PedStats_Initialise();
	TimeCycle_Initialise();
#ifdef GTA_PS2
	LoadingScreen("Loading the Game", "Initialising audio", GetRandomSplashScreen());
#endif
	DMAudio_Initialise();

#ifndef GTA_PS2
#ifdef EXTERNAL_3D_SOUND
	if ( DMAudio_GetNum3DProvidersAvailable() == 0 )
		FrontEndMenuManager.m_nPrefsAudio3DProviderIndex = NO_AUDIO_PROVIDER;

	if ( FrontEndMenuManager.m_nPrefsAudio3DProviderIndex == AUDIO_PROVIDER_NOT_DETERMINED || FrontEndMenuManager.m_nPrefsAudio3DProviderIndex == -2 )
	{
		FrontEndMenuManager.m_PrefsSpeakers = 0;
		FrontEndMenuManager.m_nPrefsAudio3DProviderIndex = DMAudio_AutoDetect3DProviders();
	}

	DMAudio_SetCurrent3DProvider(FrontEndMenuManager.m_nPrefsAudio3DProviderIndex);
	DMAudio_SetSpeakerConfig(FrontEndMenuManager.m_PrefsSpeakers);
#endif
	DMAudio_SetDynamicAcousticModelingStatus(FrontEndMenuManager.m_PrefsDMA);
	DMAudio_SetMusicMasterVolume(FrontEndMenuManager.m_PrefsMusicVolume);
	DMAudio_SetEffectsMasterVolume(FrontEndMenuManager.m_PrefsSfxVolume);
	DMAudio_SetEffectsFadeVol(127);
	DMAudio_SetMusicFadeVol(127);
#endif
	return true;
}

void
Game_FinalShutdown(void)
{	
	// Use the complete C texture dictionary owner with the original live arguments
	TxdStore_Shutdown();
	PedStats_Shutdown();
	CdStreamShutdown();
}

bool Game_Initialise(const char* datFile)
{
	ResetLoadingScreenBar();
	strcpy(Game_aDatFile, datFile);

#ifdef GTA_PS2
	// TODO: upload VU0 collision code here
#endif

	Pools_Initialise();

#ifndef GTA_PS2
#ifdef PED_CAR_DENSITY_SLIDERS
	// Load density values from gta3.ini only if our reVC.ini have them 0.6f
	if (IniFile_PedNumberMultiplier == 0.6f && IniFile_CarNumberMultiplier == 0.6f)
#endif
		IniFile_LoadIniFile();
#endif
#ifdef USE_TEXTURE_POOL
	_TexturePoolsUnknown(false);
#endif
	Game_currLevel = LEVEL_BEACH;
	Game_currArea = AREA_MAIN_MAP;

	PUSH_MEMID(MEMID_TEXTURES);
	LoadingScreen("Loading the Game", "Loading generic textures", GetRandomSplashScreen());
	// Use the complete C texture dictionary owner with the original live arguments
	gameTxdSlot = TxdStore_AddTxdSlot("generic");
	// Use the complete C texture dictionary owner with the original live arguments
	TxdStore_Create(gameTxdSlot);
	// Use the complete C texture dictionary owner with the original live arguments
	TxdStore_AddRef(gameTxdSlot);

#ifdef EXTENDED_PIPELINES
	// for generic fallback
	CustomPipes::SetTxdFindCallback();
#endif

	LoadingScreen("Loading the Game", "Loading particles", nil);
	// Use the complete C texture dictionary owner with the original live arguments
	int particleTxdSlot = TxdStore_AddTxdSlot("particle");
	// Use the complete C texture dictionary owner with the original live arguments
	TxdStore_LoadFile(particleTxdSlot, "MODELS/PARTICLE.TXD");
	// Use the complete C texture dictionary owner with the original live arguments
	TxdStore_AddRef(particleTxdSlot);
	// Use the complete C texture dictionary owner with the original live arguments
	TxdStore_SetCurrentTxd(gameTxdSlot);
	LoadingScreen("Loading the Game", "Setup game variables", nil);
	POP_MEMID();

#ifdef GTA_PS2
	CDma::SyncChannel(0, true);
#endif

	GameLogic_InitAtStartOfGame();
	References_Init();
	TheCamera.Init();
	TheCamera.SetRwCamera(Scene.camera);
	Debug_DebugInitTextBuffer();
	ThePaths.Init();
	ThePaths.AllocatePathFindInfoMem(4500);
	// Enter the C scripted-path operation at the original call point
	ScriptPaths_Init();
	Weather_Init();
	CullZones_Init();
	Occlusion_Init();
	Collision_Init();
	SetPieces_Init(); // rouz edit (ChatGPT)
	Zones_Init();
	UserDisplay_Init();
	Messages_Init();
	Messages_ClearAllMessagesDisplayedByGame();
	RecordGame_Init(); // rouz edit (ChatGPT)
	Restart_Initialise(); // rouz edit (ChatGPT)

	PUSH_MEMID(MEMID_WORLD);
	World_Initialise();
	POP_MEMID();

	PUSH_MEMID(MEMID_TEXTURES);
	CParticle::Initialise();
	POP_MEMID();

	PUSH_MEMID(MEMID_ANIMATION);
	// Use the complete C animation manager at the original call boundary
	AnimManager_Initialise();
	CutsceneMgr_Initialise();
	POP_MEMID();

	PUSH_MEMID(MEMID_CARS);
	CarCtrl_Init();
	POP_MEMID();

	PUSH_MEMID(MEMID_DEF_MODELS);
	InitModelIndices();
	ModelRegistry_Initialise(); // rouz edit (ChatGPT)
	Pickups_Init();
	TheCarGenerators_Init();

	CdStreamAddImage("MODELS\\GTA3.IMG");

	FileLoader_LoadLevel("DATA\\DEFAULT.DAT");
	FileLoader_LoadLevel(datFile);

	LoadingScreen("Loading the Game", "Add Particles", nil);
	World_AddParticles();
	CVehicleModelInfo::LoadVehicleColours();
	CVehicleModelInfo::LoadEnvironmentMaps();
	Zones_PostZoneCreation();
	POP_MEMID();

	LoadingScreen("Loading the Game", "Setup paths", nil);
	ThePaths.PreparePathData();
	for (int i = 0; i < NUMPLAYERS; i++)
		World_Players[i].Clear();
	World_Players[0].LoadPlayerSkin();
	TestModelIndices();

	LoadingScreen("Loading the Game", "Setup water", nil);
	WaterLevel_Initialise("DATA\\WATER.DAT");
	// Enter the complete C console implementation with the original singleton
	Console_Init(&TheConsole);
	Draw_SetFOV(120.0f);
	Draw_ms_fLODDistance = 500.0f;

	LoadingScreen("Loading the Game", "Setup streaming", nil);
	Streaming_LoadInitialVehicles();
	Streaming_LoadInitialPeds();
	Streaming_RequestBigBuildings(LEVEL_GENERIC);
	Streaming_LoadAllRequestedModels(false);
	Streaming_RemoveIslandsNotUsed(Game_currLevel);
	printf("Streaming uses %zuK of its memory", Streaming_memoryUsed / 1024); // original modifier was %d // rouz edit (ChatGPT)

	LoadingScreen("Loading the Game", "Load animations", GetRandomSplashScreen());
	PUSH_MEMID(MEMID_ANIMATION);
	// Use the complete C animation manager at the original call boundary
	AnimManager_LoadAnimFiles();
	POP_MEMID();

	Streaming_LoadInitialWeapons();
	Streaming_LoadAllRequestedModels(0);
	CPed::Initialise();
	PedRoutes_Initialise();
	Events_Initialise();
#ifdef SCREEN_DROPLETS
	ScreenDroplets::Initialise();
#endif
	LoadingScreen("Loading the Game", "Find big buildings", nil);
	Renderer_Init();

	LoadingScreen("Loading the Game", "Setup game variables", nil);
	Radar_Initialise();
	Radar_LoadTextures();
	CWeapon::InitialiseWeapons();

	LoadingScreen("Loading the Game", "Setup traffic lights", nil);
	TrafficLights_ScanForLightsOnMap();
	RoadBlocks_Init();

	LoadingScreen("Loading the Game", "Setup game variables", nil);
	Population_Initialise(); // rouz edit (ChatGPT)
	World_PlayerInFocus = 0;
	CCoronas::Init();
	CShadows::Init();
	// Enter the complete C weapon effect operation
	WeaponEffects_Init();
	// Enter the C skidmark algorithm with the original owner expressions
	Skidmarks_Init();
	// Initialize the C antenna pool
	Antennas_Init();
	Glass_Init();
	gPhoneInfo.Initialise();
#ifdef GTA_SCENE_EDIT
	SceneEdit_Initialise();
#endif

	LoadingScreen("Loading the Game", "Load scripts", nil);
	PUSH_MEMID(MEMID_SCRIPT);
	TheScripts_Init();
	Gangs_Initialise();
	POP_MEMID();

	LoadingScreen("Loading the Game", "Setup game variables", nil);
	Clock_Initialise(1000);
	CHeli::InitHelis();
	Cranes_InitCranes();
	// Enter the C moving-scenery algorithm at the original call point
	MovingThings_Init();
	Darkel_Init();
	Stats_Init();
	PacManPickups_Init();
	// Enter the C rubbish algorithm with the original owner expressions
	Rubbish_Init();
	// Enter the complete C cloud effect with the original arguments
	Clouds_Init();
	// Enter the C special-effects controller with the original arguments
	SpecialFX_Init();
	// Enter the C rope algorithm with the original owner expressions
	Ropes_Init();
	// Enter the C water-cannon algorithm with the original owner expressions
	WaterCannons_Init();
	Bridge_Init();
	Garages_Init();

	LoadingScreen("Loading the Game", "Position dynamic objects", nil);
	LoadingScreen("Loading the Game", "Initialise vehicle paths", nil);

	CTrain::InitTrains();
	CPlane::InitPlanes();
	// Enter the complete C credits implementation with the original arguments
	Credits_Init();
	RecordChase_Init(); // rouz edit (ChatGPT)
	CReplay::Init();

	LoadingScreen("Loading the Game", "Start script", nil);
#ifdef PS2_MENU
	if ( !TheMemoryCard.m_bWantToLoad )
#endif
	{
		TheScripts_StartTestScript();
		TheScripts_Process();
		TheCamera.Process();
	}

	LoadingScreen("Loading the Game", "Load scene", nil);
	Collision_ms_collisionInMemory = Game_currLevel;
	for (int i = 0; i < MAX_PADS; i++)
		CPad::GetPad(i)->Clear(true);
#ifdef USE_TEXTURE_POOL
	_TexturePoolsUnknown(true);
#endif

#ifndef MASTER
	Game_PlayerCoords = FindPlayerCoors();
	VarConsole_AddFloat(&VarConsole,"X PLAYER COORD", &Game_PlayerCoords.x, 10.0f, -10000.0f, 10000.0f, true); // rouz edit (ChatGPT)
	VarConsole_AddFloat(&VarConsole,"Y PLAYER COORD", &Game_PlayerCoords.y, 10.0f, -10000.0f, 10000.0f, true); // rouz edit (ChatGPT)
	VarConsole_AddFloat(&VarConsole,"Z PLAYER COORD", &Game_PlayerCoords.z, 10.0f, -10000.0f, 10000.0f, true); // rouz edit (ChatGPT)
	VarConsole_AddBool8(&VarConsole,"UPDATE PLAYER COORD", &Game_VarUpdatePlayerCoords, true); // rouz edit (ChatGPT)
#endif


	DMAudio_SetStartingTrackPositions(TRUE);
	DMAudio_ChangeMusicMode(MUSICMODE_GAME);
	return true;
}

bool Game_ShutDown(void)
{
#ifdef USE_TEXTURE_POOL
	_TexturePoolsUnknown(false);
#endif
	CReplay::FinishPlayback();
	CReplay::EmptyReplayBuffer();
	CPlane::Shutdown();
	CTrain::Shutdown();
	// Enter the C scripted-path operation at the original call point
	ScriptPaths_Shutdown();
	WaterCreatures_RemoveAll(); // rouz edit (ChatGPT)
	// Enter the C special-effects controller with the original arguments
	SpecialFX_Shutdown();
	Garages_Shutdown();
	// Enter the C moving-scenery algorithm at the original call point
	MovingThings_Shutdown();
	gPhoneInfo.Shutdown();
	CWeapon::ShutdownWeapons();
	PedType_Shutdown();
	
	for (int32 i = 0; i < NUMPLAYERS; i++)
	{
		if ( World_Players[i].m_pPed )
		{
			World_Remove(World_Players[i].m_pPed);
			// Destroy and release the player ped without invoking C++ delete.
			World_Players[i].m_pPed->~CPlayerPed();
						// Access raw storage through the C store or pool API
			CPool_Delete(Pools_GetPedPool(), World_Players[i].m_pPed);
						World_Players[i].m_pPed = nil;
		}
		
		World_Players[i].Clear();
	}
	
	Renderer_Shutdown();
	World_ShutDown();
	DMAudio_DestroyAllGameCreatedEntities();
	ModelRegistry_Shutdown(); // rouz edit (ChatGPT)
	// Use the complete C animation manager at the original call boundary
	AnimManager_Shutdown();
	CutsceneMgr_Shutdown();
	CVehicleModelInfo::DeleteVehicleColourTextures();
	CVehicleModelInfo::ShutdownEnvironmentMaps();
	Radar_Shutdown();
	Streaming_Shutdown();
	// Use the complete C texture dictionary owner with the original live arguments
	TxdStore_GameShutdown();
	Collision_Shutdown();
	WaterLevel_Shutdown();
	// Enter the C rubbish algorithm with the original owner expressions
	Rubbish_Shutdown();
	// Enter the complete C cloud effect with the original arguments
	Clouds_Shutdown();
	CShadows::Shutdown();
	CCoronas::Shutdown();
	// Enter the C skidmark algorithm with the original owner expressions
	Skidmarks_Shutdown();
	// Enter the complete C weapon effect operation
	WeaponEffects_Shutdown();
	CParticle::Shutdown();
	Pools_ShutDown();
	Hud_ReInitialise();
	// Use the complete C texture dictionary owner with the original live arguments
	TxdStore_RemoveTxdSlot(gameTxdSlot);
	// Use the complete C motion-blur owner and its live shared state
	MBlur_MotionBlurClose();
	CdStreamRemoveImages();
#ifdef USE_TEXTURE_POOL
	_TexturePoolsFinalShutdown();
#endif
	return true;
}

void Game_ReInitGameObjectVariables(void)
{
	GameLogic_InitAtStartOfGame();
#ifdef PS2_MENU
	if ( !TheMemoryCard.m_bWantToLoad )
#endif
	{
		TheCamera.Init();
		TheCamera.SetRwCamera(Scene.camera);
	}
	Debug_DebugInitTextBuffer();
	Weather_Init();
	UserDisplay_Init();
	Messages_Init();
	Restart_Initialise(); // rouz edit (ChatGPT)
	World_bDoingCarCollisions = false;
	Hud_ReInitialise();
	Radar_Initialise();
	CarCtrl_ReInit();
	TimeCycle_Initialise();
	Draw_SetFOV(120.0f);
	Draw_ms_fLODDistance = 500.0f;
	Streaming_RequestBigBuildings(LEVEL_GENERIC);
	Streaming_RemoveIslandsNotUsed(LEVEL_BEACH);
	Streaming_RemoveIslandsNotUsed(LEVEL_MAINLAND);
	Streaming_LoadAllRequestedModels(false);
	Game_currArea = AREA_MAIN_MAP;
	CPed::Initialise();
	Events_Initialise();
#ifdef SCREEN_DROPLETS
	ScreenDroplets::Initialise();
#endif
	CWeapon::InitialiseWeapons();
	Population_Initialise(); // rouz edit (ChatGPT)
	
	for (int i = 0; i < NUMPLAYERS; i++)
		World_Players[i].Clear();
	
	World_PlayerInFocus = 0;
	// Initialize the C antenna pool
	Antennas_Init();
	Glass_Init();
	gPhoneInfo.Initialise();

	PUSH_MEMID(MEMID_SCRIPT);
	TheScripts_Init();
	Gangs_Initialise();
	POP_MEMID();

	Timer_Initialise();
	Clock_Initialise(1000);
	TheCarGenerators_Init();
	CHeli::InitHelis();
	// Enter the C moving-scenery algorithm at the original call point
	MovingThings_Init();
	Darkel_Init();
	Stats_Init();
	Pickups_Init();
	PacManPickups_Init();
	Garages_Init();
	// Enter the C special-effects controller with the original arguments
	SpecialFX_Init();
	// Enter the C rope algorithm with the original owner expressions
	Ropes_Init();
	// Enter the C water-cannon algorithm with the original owner expressions
	WaterCannons_Init();
	// Enter the C scripted-path operation at the original call point
	ScriptPaths_Init();
	CParticle::ReloadConfig();

#ifdef PS2_MENU
	if ( !TheMemoryCard.m_bWantToLoad )
#else
	if ( !FrontEndMenuManager.m_bWantToLoad )
#endif
	{
		Cranes_InitCranes();
		TheScripts_StartTestScript();
		TheScripts_Process();
		TheCamera.Process();
		CTrain::InitTrains();
		CPlane::InitPlanes();
	}
	
	for (int32 i = 0; i < MAX_PADS; i++)
		CPad::GetPad(i)->Clear(true);
}

void Game_ReloadIPLs(void)
{
	// Empty and unused
}

void Game_ShutDownForRestart(void)
{
#ifdef USE_TEXTURE_POOL
	_TexturePoolsUnknown(false);
#endif
	CReplay::FinishPlayback();
	CReplay::EmptyReplayBuffer();
	DMAudio_DestroyAllGameCreatedEntities();
	// Enter the C moving-scenery algorithm at the original call point
	MovingThings_Shutdown();
	
	for (int i = 0; i < NUMPLAYERS; i++)
		World_Players[i].Clear();

	Garages_SetAllDoorsBackToOriginalHeight();
	TheScripts_UndoBuildingSwaps();
	TheScripts_UndoEntityInvisibilitySettings();
	World_ClearForRestart();
	GameLogic_ClearShortCut();
	Timer_Shutdown();
	Streaming_ReInit();
	Radar_RemoveRadarSections();
	FrontEndMenuManager.UnloadTextures();
	CParticleObject::RemoveAllExpireableParticleObjects();
	WaterCreatures_RemoveAll();  // rouz edit (ChatGPT)
	SetPieces_Init(); // rouz edit (ChatGPT)
	PedType_Shutdown();
	// Enter the C special-effects controller with the original arguments
	SpecialFX_Shutdown();
}

void Game_InitialiseWhenRestarting(void)
{
	CRect rect = Rect_Make(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	CRGBA color(255, 255, 255, 255);
	
	Timer_Initialise();
	// Preserve the original operation through the C sprite interface
	Sprite2d_SetRecipNearClip();

	if (b_FoundRecentSavedGameWantToLoad || FrontEndMenuManager.m_bWantToLoad)
	{
		LoadSplash("splash1");
#ifndef XBOX_MESSAGE_SCREEN
		if (FrontEndMenuManager.m_bWantToLoad)
			FrontEndMenuManager.MessageScreen("FELD_WR", true);
#endif
	}

	b_FoundRecentSavedGameWantToLoad = false;
	
	TheCamera.Init();
	
	if ( FrontEndMenuManager.m_bWantToLoad == true )
	{
#ifdef XBOX_MESSAGE_SCREEN
		FrontEndMenuManager.SetDialogTimer(1000);
		DoRWStuffStartOfFrame(0, 0, 0, 0, 0, 0, 0);
		// Preserve the original operation through the C sprite interface
		Sprite2d_InitPerFrame();
		Font_InitPerFrame();
		FrontEndMenuManager.DrawOverlays();
		DoRWStuffEndOfFrame();
#endif
		RestoreForStartLoad();
	}
	
	Game_ReInitGameObjectVariables();
	
	if ( FrontEndMenuManager.m_bWantToLoad == true )
	{
		FrontEndMenuManager.m_bWantToLoad = false;
		InitRadioStationPositionList();
		if ( GenericLoad() == true )
		{
			DMAudio_ResetTimers((*Timer_GetTimeInMilliseconds()));
			CTrain::InitTrains();
			CPlane::InitPlanes();
		}
		else
		{
			for ( int32 i = 0; i < 50; i++ )
			{
				HandleExit();
				FrontEndMenuManager.MessageScreen("FED_LFL", true); // Loading save game has failed. The game will restart now. 
			}
			
			TheCamera.SetFadeColour(0, 0, 0);
			Game_ShutDownForRestart();
			Timer_Stop();
			Timer_Initialise();
			FrontEndMenuManager.m_bWantToLoad = false;
			Game_ReInitGameObjectVariables();
			Game_currLevel = LEVEL_GENERIC;
			Collision_SortOutCollisionAfterLoad();
		}
#ifdef XBOX_MESSAGE_SCREEN
		FrontEndMenuManager.ProcessDialogTimer();
#endif
	}
	
	Timer_Update();
	
	DMAudio_ChangeMusicMode(MUSICMODE_GAME);
#ifdef USE_TEXTURE_POOL
	_TexturePoolsUnknown(true);
#endif
}

void Game_Process(void) 
{
	CPad::UpdatePads();
#ifdef USE_CUSTOM_ALLOCATOR
	Game_ProcessTidyUpMemory();
#endif
#ifdef DEBUGMENU
	DebugMenuProcess();
#endif
	CutsceneMgr_Update();

	if (!Cutscene_IsProcessing() && !Timer_GetIsCodePaused()) // rouz edit (ChatGPT)
		FrontEndMenuManager.Process();

	Zones_Update();
#ifdef SECUROM
	if ((*Timer_GetTimeInMilliseconds()) >= (35 * 60 * 1000) && gameProcessPirateCheck == 0){
		// if game not pirated
		// gameProcessPirateCheck = 1;
		// else
		gameProcessPirateCheck = 2;
	}
#endif
	uint32 startTime = Timer_GetCurrentTimeInCycles() / Timer_GetCyclesPerMillisecond();
	Streaming_Update();
	uint32 processTime = Timer_GetCurrentTimeInCycles() / Timer_GetCyclesPerMillisecond() - startTime;
	WindModifiers_Number = 0;
	rouz_update();
	if (!Timer_GetIsPaused())
	{
#ifndef MASTER
		if (Game_VarUpdatePlayerCoords) {
			FindPlayerPed()->Teleport(Game_PlayerCoords);
			Game_VarUpdatePlayerCoords = false;
		}
#endif
		// Preserve the original operation through the C sprite interface
		Sprite2d_SetRecipNearClip();
		// Preserve the original operation through the C sprite interface
		Sprite2d_InitPerFrame();
		Font_InitPerFrame();
		RecordGame_SaveOrRetrieveDataForThisFrame(); // rouz edit (ChatGPT)
		RecordChase_SaveOrRetrieveDataForThisFrame(); // rouz edit (ChatGPT)
		CPad::DoCheats();
		Clock_Update();
		Weather_Update();

		PUSH_MEMID(MEMID_SCRIPT);
		TheScripts_Process();
		POP_MEMID();

		Collision_Update();
		// Enter the C scripted-path operation at the original call point
		ScriptPaths_Update();
		CTrain::UpdateTrains();
		CPlane::UpdatePlanes();
		CHeli::UpdateHelis();
		Darkel_Update();
		// Enter the C skidmark algorithm with the original owner expressions
		Skidmarks_Update();
		// Advance the C antenna frame state
		Antennas_Update();
		Glass_Update();
#ifdef GTA_SCENE_EDIT
		SceneEdit_Update();
#endif
		SetPieces_Update(); // rouz edit (ChatGPT)
		Events_Update();
		CParticle::Update();
		FireManager_Update(&gFireManager);

		// Otherwise even on 30 fps most probably you won't see any peds around Ocean View Hospital
#if defined FIX_BUGS && !defined SQUEEZE_PERFORMANCE
		if (processTime > 2) {
#else
		if (processTime >= 2) {
#endif
			Population_Update(false); // rouz edit (ChatGPT)
		} else {
			uint32 startTime = Timer_GetCurrentTimeInCycles() / Timer_GetCyclesPerMillisecond();
			Population_Update(true); // rouz edit (ChatGPT)
			processTime = Timer_GetCurrentTimeInCycles() / Timer_GetCyclesPerMillisecond() - startTime;
		}
		CWeapon::UpdateWeapons();
		if (!Cutscene_IsRunning()) // rouz edit (ChatGPT)
			TheCarGenerators_Process();
		if (!Replay_IsPlayingBack()) // rouz edit (ChatGPT)
			Cranes_UpdateCranes();
		// Enter the complete C cloud effect with the original arguments
		Clouds_Update();
		// Enter the C moving-scenery algorithm at the original call point
		MovingThings_Update();
		// Enter the C water-cannon algorithm with the original owner expressions
		WaterCannons_Update();
		UserDisplay_Process();
		CReplay::Update();

		PUSH_MEMID(MEMID_WORLD);
		World_Process();
		POP_MEMID();

		AccidentManager_Update(&gAccidentManager);
		PacManPickups_Update();
		Pickups_Update();
		Garages_Update();
		// Enter the C rubbish algorithm with the original owner expressions
		Rubbish_Update();
		// Enter the C special-effects controller with the original arguments
		SpecialFX_Update();
		// Enter the C rope algorithm with the original owner expressions
		Ropes_Update();
		TimeCycle_Update();
		if (Replay_ShouldStandardCameraBeProcessed()) // rouz edit (ChatGPT)
			TheCamera.Process();
		CullZones_Update();
		if (!Replay_IsPlayingBack()) // rouz edit (ChatGPT)
			GameLogic_Update();
		Bridge_Update();
		CCoronas::DoSunAndMoon();
		CCoronas::Update();
		CShadows::UpdateStaticShadows();
		CShadows::UpdatePermanentShadows();
		gPhoneInfo.Update();
		if (!Replay_IsPlayingBack()) // rouz edit (ChatGPT)
		{
			PUSH_MEMID(MEMID_CARS);
			if (processTime < 2)
				CarCtrl_GenerateRandomCars();
			RoadBlocks_GenerateRoadBlocks();
			CarCtrl_RemoveDistantCars();
			CarCtrl_RemoveCarsIfThePoolGetsFull();
			POP_MEMID();
		}
	}
#ifdef GTA_PS2
	CMemCheck::DoTest();
#endif
}

#ifdef USE_CUSTOM_ALLOCATOR

// TODO(MIAMI)

int32 gNumMemMoved;

bool
MoveMem(void** ptr)
{
	if (*ptr) {
		gNumMemMoved++;
		// Use the complete C heap operation with the original live owner
		void* newPtr = MemoryHeap_MoveMemory(&gMainHeap,*ptr);
		if (*ptr != newPtr) {
			*ptr = newPtr;
			return true;
		}
	}
	return false;
}

// Some convenience structs
struct SkyDataPrefix
{
	uint32 pktSize1;
	uint32 data;	// pointer to data as read from TXD
	uint32 pktSize2;
	uint32 unused;
};

struct DMAGIFUpload
{
	uint32 tag1_qwc, tag1_addr;	// dmaref
	uint32 nop1, vif_direct1;

	uint32 giftag[4];
	uint32 gs_bitbltbuf[4];

	uint32 tag2_qwc, tag2_addr;	// dmaref
	uint32 nop2, vif_direct2;
};

// This is very scary. it depends on the exact memory layout of the DMA chains and whatnot
RwTexture*
MoveTextureMemoryCB(RwTexture* texture, void* pData)
{
#ifdef GTA_PS2
	bool* pRet = (bool*)pData;
	RwRaster* raster = RwTextureGetRaster(texture);
	_SkyRasterExt* rasterExt = RASTEREXTFROMRASTER(raster);
	if (raster->originalPixels == nil ||	// the raw data
		raster->cpPixels == raster->originalPixels ||	// old format, can't handle it
		rasterExt->dmaRefCount != 0 && rasterExt->dmaClrCount != 0)
		return texture;

	// this is the allocated pointer we will move
	SkyDataPrefix* prefix = (SkyDataPrefix*)raster->originalPixels;
	DMAGIFUpload* uploads = (DMAGIFUpload*)(prefix + 1);

	// We have 4qw for each upload,
	// i.e. for each buffer width of mip levels,
	// and the palette if there is one.
	// NB: this code does NOT support mipmaps!
	// so we assume two uploads (pixels and palette)
	//
	// each upload looks like this:
	//    (DMAcnt; NOP; VIF DIRECT(2))
	//     giftag (1, A+D)
	//      GS_BITBLTBUF
	//    (DMAref->pixel data; NOP; VIF DIRECT(5))
	// the DMArefs are what we have to adjust
	uintptr dataDiff, upload1Diff, upload2Diff, pixelDiff, paletteDiff;
	dataDiff = prefix->data - (uintptr)raster->originalPixels;
	upload1Diff = uploads[0].tag2_addr - (uintptr)raster->originalPixels;
	if (raster->palette)
		upload2Diff = uploads[1].tag2_addr - (uintptr)raster->originalPixels;
	pixelDiff = (uintptr)raster->cpPixels - (uintptr)raster->originalPixels;
	if (raster->palette)
		paletteDiff = (uintptr)raster->palette - (uintptr)raster->originalPixels;
	// Use the complete C heap operation with the original live owner
	uint8* newptr = (uint8*)MemoryHeap_MoveMemory(&gMainHeap,raster->originalPixels);
	if (newptr != raster->originalPixels) {
		// adjust everything
		prefix->data = (uintptr)newptr + dataDiff;
		uploads[0].tag2_addr = (uintptr)newptr + upload1Diff;
		if (raster->palette)
			uploads[1].tag2_addr = (uintptr)newptr + upload2Diff;
		raster->originalPixels = newptr;
		raster->cpPixels = newptr + pixelDiff;
		if (raster->palette)
			raster->palette = newptr + paletteDiff;

		if (pRet) {
			*pRet = true;
			return nil;
		}
	}
#else
	// nothing to do here really, everything should be in videomemory
#endif
	return texture;
}

bool
MoveAtomicMemory(RpAtomic* atomic, bool onlyOne)
{
	RpGeometry* geo = RpAtomicGetGeometry(atomic);

#if THIS_IS_COMPATIBLE_WITH_GTA3_RW31
	if (MoveMem((void**)&geo->triangles) && onlyOne)
		return true;
	if (MoveMem((void**)&geo->matList.materials) && onlyOne)
		return true;
	if (MoveMem((void**)&geo->preLitLum) && onlyOne)
		return true;
	if (MoveMem((void**)&geo->texCoords[0]) && onlyOne)
		return true;
	if (MoveMem((void**)&geo->texCoords[1]) && onlyOne)
		return true;

	// verts and normals of morph target are allocated together
	int vertDiff;
	if (geo->morphTarget->normals)
		vertDiff = geo->morphTarget->normals - geo->morphTarget->verts;
	if (MoveMem((void**)&geo->morphTarget->verts)) {
		if (geo->morphTarget->normals)
			geo->morphTarget->normals = geo->morphTarget->verts + vertDiff;
		if (onlyOne)
			return true;
	}

	RpMeshHeader* oldmesh = geo->mesh;
	if (MoveMem((void**)&geo->mesh)) {
		// index pointers are allocated together with meshes,
		// have to relocate those too
		RpMesh* mesh = (RpMesh*)(geo->mesh + 1);
		uintptr reloc = (uintptr)geo->mesh - (uintptr)oldmesh;
		for (int i = 0; i < geo->mesh->numMeshes; i++)
			mesh[i].indices = (RxVertexIndex*)((uintptr)mesh[i].indices + reloc);
		if (onlyOne)
			return true;
	}
#else
	// we could do something in librw here
#endif
	return false;
}

bool
MoveColModelMemory(CColModel& colModel, bool onlyOne)
{
#if GTA_VERSION >= GTA3_PS2_160
	// hm...should probably only do this if ownsCollisionVolumes
	// but it doesn't exist on PS2...
	if (!colModel.ownsCollisionVolumes)
		return false;
#endif

	if (MoveMem((void**)&colModel.spheres) && onlyOne)
		return true;
	if (MoveMem((void**)&colModel.lines) && onlyOne)
		return true;
	if (MoveMem((void**)&colModel.boxes) && onlyOne)
		return true;
	if (MoveMem((void**)&colModel.vertices) && onlyOne)
		return true;
	if (MoveMem((void**)&colModel.triangles) && onlyOne)
		return true;
	if (MoveMem((void**)&colModel.trianglePlanes) && onlyOne)
		return true;
	return false;
}

RpAtomic*
MoveAtomicMemoryCB(RpAtomic* atomic, void* pData)
{
	bool* pRet = (bool*)pData;
	if (pRet == nil)
		MoveAtomicMemory(atomic, false);
	else if (MoveAtomicMemory(atomic, true)) {
		*pRet = true;
		return nil;
	}
	return atomic;
}

bool
TidyUpModelInfo(CBaseModelInfo* modelInfo, bool onlyone)
{
	if (modelInfo->GetColModel() && modelInfo->DoesOwnColModel())
		if (MoveColModelMemory(*modelInfo->GetColModel(), onlyone))
			return true;

	RwObject* rwobj = modelInfo->GetRwObject();
	if (RwObjectGetType(rwobj) == rpATOMIC)
		if (MoveAtomicMemory((RpAtomic*)rwobj, onlyone))
			return true;
	if (RwObjectGetType(rwobj) == rpCLUMP) {
		bool ret = false;
		if (onlyone)
			RpClumpForAllAtomics((RpClump*)rwobj, MoveAtomicMemoryCB, &ret);
		else
			RpClumpForAllAtomics((RpClump*)rwobj, MoveAtomicMemoryCB, nil);
		if (ret)
			return true;
	}

	if (modelInfo->GetModelType() == MITYPE_PED && ((CPedModelInfo*)modelInfo)->m_hitColModel)
		if (MoveColModelMemory(*((CPedModelInfo*)modelInfo)->m_hitColModel, onlyone))
			return true;

	return false;
}
#endif


void Game_DrasticTidyUpMemory(bool flushDraw)
{
#ifdef USE_CUSTOM_ALLOCATOR
	bool removedCol = false;

	Game_TidyUpMemory(true, flushDraw);

	// Use the complete C heap operation with the original live owner
	if (MemoryHeap_GetLargestFreeBlock(&gMainHeap) < 200000 && !Game_playingIntro) {
		Streaming_RemoveIslandsNotUsed(LEVEL_INDUSTRIAL);
		Streaming_RemoveIslandsNotUsed(LEVEL_COMMERCIAL);
		Streaming_RemoveIslandsNotUsed(LEVEL_SUBURBAN);
		Game_TidyUpMemory(true, flushDraw);
	}

	// Use the complete C heap operation with the original live owner
	if (MemoryHeap_GetLargestFreeBlock(&gMainHeap) < 200000 && !Game_playingIntro) {
		CModelInfo::RemoveColModelsFromOtherLevels(LEVEL_GENERIC);
		Game_TidyUpMemory(true, flushDraw);
		removedCol = true;
	}

	// Use the complete C heap operation with the original live owner
	if (MemoryHeap_GetLargestFreeBlock(&gMainHeap) < 200000 && !Game_playingIntro) {
		Streaming_RemoveBigBuildings(LEVEL_INDUSTRIAL);
		Streaming_RemoveBigBuildings(LEVEL_COMMERCIAL);
		Streaming_RemoveBigBuildings(LEVEL_SUBURBAN);
		Game_TidyUpMemory(true, flushDraw);
	}

	if (removedCol) {
		// different on PS2
		CFileLoader::LoadCollisionFromDatFile(Collision_ms_collisionInMemory);
	}

	if (!Game_playingIntro)
		Streaming_RequestBigBuildings(Game_currLevel);

	Streaming_LoadAllRequestedModels(true);
#endif
}

void Game_TidyUpMemory(bool moveTextures, bool flushDraw)
{
#ifdef USE_CUSTOM_ALLOCATOR
	// Use the complete C heap operation with the original live owner
	printf("Largest free block before tidy %d\n", MemoryHeap_GetLargestFreeBlock(&gMainHeap));

	if (moveTextures) {
		if (flushDraw) {
#ifdef GTA_PS2
			for (int i = 0; i < sweMaxFlips + 1; i++) {
#else
			for (int i = 0; i < 5; i++) {	// probably more than needed
#endif
				RwCameraBeginUpdate(Scene.camera);
				RwCameraEndUpdate(Scene.camera);
				RwCameraShowRaster(Scene.camera, nil, 0);
			}
			}
		// Use the complete C texture dictionary owner with the original live arguments
		int fontSlot = TxdStore_FindTxdSlot("fonts");

		for (int i = 0; i < TXDSTORESIZE; i++) {
			if (i == fontSlot ||
				// Use the complete C texture dictionary owner with the original live arguments
				TxdStore_GetSlot(i) == nil)
				continue;
			// Use the complete C texture dictionary owner with the original live arguments
			RwTexDictionary* txd = ((RwTexDictionary*)TxdStore_GetSlot(i)->texDict);
			if (txd)
				RwTexDictionaryForAllTextures(txd, MoveTextureMemoryCB, nil);
		}
		}

	// animations
	for (int i = 0; i < NUMANIMATIONS; i++) {
		// Use the complete C animation manager at the original call boundary
		CAnimBlendHierarchy* anim = AnimManager_GetAnimationById(i);
		if (anim == nil)
			continue;	// cannot happen
		// Use the complete C hierarchy at the original native operation boundary
		AnimHierarchy_MoveMemory(anim,false);
	}

	// model info
	for (int i = 0; i < MODELINFOSIZE; i++) {
		CBaseModelInfo* mi = ((CBaseModelInfo*)ModelRegistry_Get(i)); // rouz edit (ChatGPT)
		if (mi == nil)
			continue;
		TidyUpModelInfo(mi, false);
	}

	// Use the complete C heap operation with the original live owner
	printf("Largest free block after tidy %d\n", MemoryHeap_GetLargestFreeBlock(&gMainHeap));
#endif
	}

void Game_ProcessTidyUpMemory(void)
{
#ifdef USE_CUSTOM_ALLOCATOR
	static int32 modelIndex = 0;
	static int32 animIndex = 0;
	static int32 txdIndex = 0;
	bool txdReturn = false;
	RwTexDictionary* txd = nil;
	gNumMemMoved = 0;

	// model infos
	for (int numCleanedUp = 0; numCleanedUp < 10; numCleanedUp++) {
		CBaseModelInfo* mi;
		do {
			mi = ((CBaseModelInfo*)ModelRegistry_Get(modelIndex)); // rouz edit (ChatGPT)
			modelIndex++;
			if (modelIndex >= MODELINFOSIZE)
				modelIndex = 0;
		} while (mi == nil);

		if (TidyUpModelInfo(mi, true))
			return;
	}

	// tex dicts
	for (int numCleanedUp = 0; numCleanedUp < 3; numCleanedUp++) {
		if (gNumMemMoved > 80)
			break;

		do {
#ifdef FIX_BUGS
			txd = nil;
#endif
			// Use the complete C texture dictionary owner with the original live arguments
			if (TxdStore_GetSlot(txdIndex))
				// Use the complete C texture dictionary owner with the original live arguments
				txd = ((RwTexDictionary*)TxdStore_GetSlot(txdIndex)->texDict);
			txdIndex++;
			if (txdIndex >= TXDSTORESIZE)
				txdIndex = 0;
		} while (txd == nil);

		RwTexDictionaryForAllTextures(txd, MoveTextureMemoryCB, &txdReturn);
		if (txdReturn)
			return;
		}

	// animations
	CAnimBlendHierarchy* anim;
	do {
		// Use the complete C animation manager at the original call boundary
		anim = AnimManager_GetAnimationById(animIndex);
		animIndex++;
		if (animIndex >= NUMANIMATIONS)
			animIndex = 0;
	} while (anim == nil);	// always != nil
	// Use the complete C hierarchy at the original native operation boundary
	AnimHierarchy_MoveMemory(anim,true);
#endif
}

void
Game_InitAfterFocusLoss()
{
	FrontEndMenuManager.m_nPrefsAudio3DProviderIndex = FrontEndMenuManager.m_lastWorking3DAudioProvider;
	DMAudio_SetCurrent3DProvider(FrontEndMenuManager.m_lastWorking3DAudioProvider);

	if (!FrontEndMenuManager.m_bGameNotLoaded && !FrontEndMenuManager.m_bMenuActive)
		FrontEndMenuManager.m_bStartUpFrontEndRequested = true;
}

bool
Game_CanSeeWaterFromCurrArea(void)
{
	return Game_currArea == AREA_MAIN_MAP || Game_currArea == AREA_MANSION
		|| Game_currArea == AREA_HOTEL;
}

bool
Game_CanSeeOutSideFromCurrArea(void)
{
	return Game_currArea == AREA_MAIN_MAP || Game_currArea == AREA_MALL ||
		Game_currArea == AREA_MANSION || Game_currArea == AREA_HOTEL;
}

//- rouz edit (ChatGPT)

//- rouz edit (ChatGPT)

//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
