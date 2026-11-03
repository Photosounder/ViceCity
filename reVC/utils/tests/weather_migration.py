"""Move all weather decisions, schedules, rain records and billboard arithmetic into C11."""
import re
from audio_controls_migration import function
import object_names_migration as objects
import ped_data_migration as pedestrian
OBSERVER=None
SCALARS=[('int32_t','SoundHandle'),('int32_t','WeatherTypeInList'),('int16_t','OldWeatherType'),('int16_t','NewWeatherType'),('int16_t','ForcedWeatherType'),('bool','LightningFlash'),('bool','LightningBurst'),('uint32_t','LightningStart'),('uint32_t','LightningFlashLastChange'),('uint32_t','WhenToPlayLightningSound'),('uint32_t','LightningDuration'),('float','StreamAfterRainTimer')]+[('float',name) for name in ['ExtraSunnyness','Foggyness','CloudCoverage','Wind','Rain','InterpolationValue','WetRoads','Rainbow','SunGlare','WindClipped','TrafficLightBrightness']]+[('bool','bScriptsForceRain')]
METHODS=['Init','Update','AddHeatHaze','AddBeastie','ForceWeather','ForceWeatherNow','ReleaseWeather','AddSplashesDuringHurricane','AddStreamAfterRain','AddRain','RenderRainStreaks','ForceHurricaneWeather']
HEADER='''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#include "WeatherTypes.h"
typedef struct WeatherVector {float x,y,z;} WeatherVector;
typedef struct WeatherColor {uint8_t red,green,blue,alpha;} WeatherColor;
enum {NUM_RAIN_STREAKS=35};
typedef struct WeatherRainStreak {WeatherVector position,direction;uint32_t timer;} WeatherRainStreak;
#ifdef __cplusplus
extern "C" {
#endif
'''+''.join('extern '+kind+' Weather_'+name+';\n' for kind,name in SCALARS)+'''extern WeatherRainStreak Weather_Streaks[NUM_RAIN_STREAKS];
extern int16_t Weather_Types[64],Weather_HurricaneTypes[64];
extern const float Weather_Windyness[7];
void Weather_Init(void);
void Weather_Update(void);
void Weather_AddHeatHaze(void);
void Weather_AddBeastie(void);
void Weather_ForceWeather(int16_t weather);
void Weather_ForceWeatherNow(int16_t weather);
void Weather_ReleaseWeather(void);
void Weather_AddSplashesDuringHurricane(void);
void Weather_AddStreamAfterRain(void);
void Weather_AddRain(void);
void Weather_RenderRainStreaks(void);
void Weather_RenderOneRainStreak(WeatherVector pos,WeatherVector unused,int32_t intensity,bool scale,float distance,WeatherVector right,WeatherVector up);
#ifdef SECUROM
void Weather_ForceHurricaneWeather(void);
#endif
#ifdef __cplusplus
}
#ifdef LIBRW
namespace rw {struct Texture;}
typedef rw::Texture WeatherTexture;
#else
typedef struct RwTexture WeatherTexture;
#endif
extern WeatherTexture *gpRainDropTex;
#endif
//- rouz edit (ChatGPT)
'''
STATES={'rwRENDERSTATETEXTURERASTER':1,'rwRENDERSTATEZTESTENABLE':6,'rwRENDERSTATEZWRITEENABLE':8,'rwRENDERSTATESRCBLEND':10,'rwRENDERSTATEDESTBLEND':11,'rwRENDERSTATEVERTEXALPHAENABLE':12,'rwRENDERSTATEFOGENABLE':14,'rwRENDERSTATEFOGTYPE':16,'rwFOGTYPELINEAR':1,'rwBLENDONE':2,'rwBLENDSRCALPHA':5,'rwBLENDINVSRCALPHA':6}
READERS=[
('bool','Playback','void','return CReplay::IsPlayingBack();'),
('bool','NoHurricanes','void','return CStats::NoMoreHurricanes;'),
('bool','ShockDown','void','return CPad::GetPad(1)->GetRightShockJustDown();'),
('void','Shake','int16_t duration,uint8_t frequency','CPad::GetPad(0)->StartShake(duration,frequency);'),
('float','SunZ','void','return CTimeCycle::GetSunDirection().z;'),
('bool','Interior','void','return CGame::IsInInterior();'),
('bool','Cutscene','void','return CCutsceneMgr::IsRunning();'),
('bool','PlayerExists','void','return FindPlayerPed()!=nil;'),
('bool','PlayerInAir','void','return FindPlayerPed()->CheckIfInTheAir();'),
('float','PlayerZ','void','return FindPlayerPed()->GetPosition().z;'),
('float','PlayerSpeedX','void','return FindPlayerSpeed().x;'),
('float','PlayerSpeedY','void','return FindPlayerSpeed().y;'),
('CVehicle *','PlayerVehicle','void','return FindPlayerVehicle();'),
('bool','VehicleRoof','CVehicle *car','return car->CarHasRoof();'),
('int16_t','CameraMode','void','return TheCamera.Cams[TheCamera.ActiveCam].Mode;'),
('bool','CameraForwardFirstPerson','void','return TheCamera.GetLookingForwardFirstPerson();'),
('bool','CameraLRBFirstPerson','void','return TheCamera.GetLookingLRBFirstPerson();'),
('float','ScreenWidth','void','return SCREEN_WIDTH;'),
('float','ScreenHeight','void','return SCREEN_HEIGHT;'),
('float','CosValue','int32_t angle','return CParticle::m_CosTable[angle];'),
('float','SinValue','int32_t angle','return CParticle::m_SinTable[angle];'),
('float','CameraOrientation','void','return TheCamera.Orientation;'),
('float','CameraSpeed','void','return TheCamera.m_CameraAverageSpeed;'),
('CEntity *','Target','void','return TheCamera.pTargetEntity;'),
('WeatherVector','EntityPosition','CEntity *entity','const CVector &value=entity->GetPosition();return WeatherVector{value.x,value.y,value.z};'),
('float','GroundZ','float x,float y,float z,bool *found','return CWorld::FindGroundZFor3DCoord(x,y,z,found);'),
('void','RemoveParticles','enum tParticleType type','CParticle::RemovePSystem(type);'),
('void','ParticleSimple','enum tParticleType type,WeatherVector position,WeatherVector direction','CParticle::AddParticle(type,CVector(position.x,position.y,position.z),CVector(direction.x,direction.y,direction.z));'),
('void','ParticleColor','enum tParticleType type,WeatherVector position,WeatherVector direction,float size,WeatherColor value','RwRGBA color={value.red,value.green,value.blue,value.alpha};CParticle::AddParticle(type,CVector(position.x,position.y,position.z),CVector(direction.x,direction.y,direction.z),nil,size,color);'),
('void','ParticleDrop','enum tParticleType type,WeatherVector position,WeatherVector direction,float size,int32_t rotationSpeed,int32_t rotation,int32_t frame,int32_t lifespan','CParticle::AddParticle(type,CVector(position.x,position.y,position.z),CVector(direction.x,direction.y,direction.z),nil,size,rotationSpeed,rotation,frame,lifespan);'),
('bool','VerticalLine','WeatherVector value,float *height','CVector position(value.x,value.y,value.z);CColPoint point;CEntity *entity;bool hit=CWorld::ProcessVerticalLine(position,-40.0f,point,entity,true,false,false,false,true,false,nil);\n    if(hit){\n        // Copy only the valid collision height before the C controller decides whether to add splashes\n        *height=point.point.z;\n    }\n    return hit;'),
('bool','EnvMap','void','#if defined(LIBRW) && defined(EXTENDED_PIPELINES)\n    return CustomPipes::bRenderingEnvMap;\n#else\n    return false;\n#endif'),
('void *','EnvCamera','void','#if defined(LIBRW) && defined(EXTENDED_PIPELINES)\n    return CustomPipes::EnvMapCam;\n#else\n    return nil;\n#endif'),
('void *','CameraFrame','void *camera','return RwCameraGetFrame((RwCamera*)camera);'),
('void *','FrameMatrix','void *frame','return RwFrameGetMatrix((RwFrame*)frame);'),
('void','VertexRGBA','int32_t index,int32_t red,int32_t green,int32_t blue,int32_t alpha','RwIm3DVertexSetRGBA(&TempBufferRenderVertices[index],red,green,blue,alpha);'),
('void','VertexPos','int32_t index,float x,float y,float z','RwIm3DVertexSetPos(&TempBufferRenderVertices[index],x,y,z);'),
('void','VertexU','int32_t index,float u','RwIm3DVertexSetU(&TempBufferRenderVertices[index],u);'),
('void','VertexV','int32_t index,float v','RwIm3DVertexSetV(&TempBufferRenderVertices[index],v);'),
('void','RenderState','int32_t state,uintptr_t value','RwRenderStateSet((RwRenderState)state,(void*)value);'),
('void *','Raster','void','return RwTextureGetRaster(gpRainDropTex);'),
('bool','Transform','int32_t vertices','return RwIm3DTransform(TempBufferRenderVertices,vertices,nil,1)!=nil;'),
('void','RenderIndexed','int32_t indices','RwIm3DRenderIndexedPrimitive(rwPRIMTYPETRILIST,TempBufferRenderIndexList,indices);'),
('void','End','void','RwIm3DEnd();')]
for name,expression in [('CameraPosition','TheCamera.GetPosition()'),('CameraForward','TheCamera.GetForward()'),('CameraRight','TheCamera.GetRight()'),('CameraUp','TheCamera.GetUp()'),('CameraPrevious','TheCamera.m_RealPreviousCameraPosition')]:READERS.append(('WeatherVector',name,'void','const CVector &value='+expression+';return WeatherVector{value.x,value.y,value.z};'))
for name,field in [('MatrixPosition','pos'),('MatrixRight','right'),('MatrixUp','up')]:READERS.append(('WeatherVector',name,'void *matrix','RwMatrix *value=(RwMatrix*)matrix;return WeatherVector{value->'+field+'.x,value->'+field+'.y,value->'+field+'.z};'))
BACKEND='''//+ rouz edit (ChatGPT)
#pragma once
#include "Weather.h"
#include "ParticleType.h"
#ifdef __cplusplus
class CVehicle;class CEntity;
extern "C" {
#else
typedef struct CVehicle CVehicle;typedef struct CEntity CEntity;
#endif
enum {WEATHER_TRIG_TABLE_SIZE=1024,'''+','.join('WEATHER_'+name+'='+str(value) for name,value in STATES.items())+'''};
'''+''.join(ret+' Weather_Backend'+name+'('+args+');\n' for ret,name,args,body in READERS)+'''#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
HOST='''//+ rouz edit (ChatGPT)
#include "common.h"
#include "Weather_backend.h"
#include "Timecycle.h"
#include "Game.h"
#include "Replay.h"
#include "PlayerInfo.h"
#include "Camera.h"
#include "CutsceneMgr.h"
#include "Pad.h"
#include "PlayerPed.h"
#include "Particle.h"
#include "RenderBuffer.h"
#include "Stats.h"
#include "Timecycle.h"
#include "Vehicle.h"
#include "World.h"
#if defined(LIBRW) && defined(EXTENDED_PIPELINES)
#include "custompipes.h"
#endif
static_assert(WEATHER_TRIG_TABLE_SIZE==CParticle::SIN_COS_TABLE_SIZE,"Original particle angle table capacity");
'''+''.join('static_assert(int(WEATHER_'+name+')==int('+name+'),"Original native render constant");\n' for name in STATES)+''.join(ret+' Weather_Backend'+name+'('+args+')\n{\n    // Access the original typed native operation without moving its weather decisions\n    '+body+'\n}\n' for ret,name,args,body in READERS)+'//- rouz edit (ChatGPT)\n'

def rain_helper(source):
    # Extract the complete original helper with its two-line signature and balanced body
    start=source.index('void RenderOneRainStreak(');cursor=source.index('{',start)+1;depth=1
    while depth:depth+=(source[cursor]=='{')-(source[cursor]=='}');cursor+=1
    return source[start:cursor]

def kernel(source):
    # Retain complete data definitions and every class/global operation before replacing native syntax
    data=source[source.index('int32 CWeather::SoundHandle'):source.index('#define MIN_TIME_BETWEEN_LIGHTNING_FLASH_CHANGES')]
    constants=source[source.index('#define MIN_TIME_BETWEEN_LIGHTNING_FLASH_CHANGES'):source.index('static float\nWeatherTimeStep')]
    text=data+constants+function(source,'','WeatherTimeStep')+'\n\n'+'\n\n'.join(function(source,'CWeather::',name) for name in METHODS[:-1])+'\n\n'+rain_helper(source)+'\n\n#ifdef SECUROM\n'+function(source,'CWeather::','ForceHurricaneWeather')+'\n#endif\n'
    text=text.replace('CWeather::','Weather_')
    for name in METHODS:text=re.sub(r'\b'+name+r'\(', 'Weather_'+name+'(',text)
    text=re.sub(r'(?m)^(void Weather_\w+)\(\)\n\{',r'\1(void)\n{',text).replace('WeatherTimeStep()\n{','WeatherTimeStep(void)\n{')
    for kind,name in SCALARS:text=re.sub(r'(?<!Weather_)\b'+name+r'\b','Weather_'+name,text)
    for old,new in [('WeatherTypesList_WithHurricanes','Weather_HurricaneTypes'),('WeatherTypesList','Weather_Types'),('Windyness','Weather_Windyness'),('Streaks','Weather_Streaks'),('tRainStreak','WeatherRainStreak')]:text=re.sub(r'\b'+old+r'\b',new,text)
    text=text.replace('bool Weather_bScriptsForceRain;','bool Weather_bScriptsForceRain;\nstatic int startStreamAfterRain;')
    native={'CReplay::IsPlayingBack()':'Weather_BackendPlayback()','CStats::NoMoreHurricanes':'Weather_BackendNoHurricanes()','CPad::GetPad(1)->GetRightShockJustDown()':'Weather_BackendShockDown()','CPad::GetPad(0)->StartShake(':'Weather_BackendShake(','CTimeCycle::GetSunDirection().z':'Weather_BackendSunZ()','CGame::IsInInterior()':'Weather_BackendInterior()','CCutsceneMgr::IsRunning()':'Weather_BackendCutscene()','FindPlayerPed()->CheckIfInTheAir()':'Weather_BackendPlayerInAir()','FindPlayerPed()->GetPosition().z':'Weather_BackendPlayerZ()','FindPlayerPed()':'Weather_BackendPlayerExists()','FindPlayerVehicle()':'Weather_BackendPlayerVehicle()','pVehicle->CarHasRoof()':'Weather_BackendVehicleRoof(pVehicle)','TheCamera.Cams[TheCamera.ActiveCam].Mode':'Weather_BackendCameraMode()','TheCamera.GetLookingForwardFirstPerson()':'Weather_BackendCameraForwardFirstPerson()','TheCamera.GetLookingLRBFirstPerson()':'Weather_BackendCameraLRBFirstPerson()','SCREEN_WIDTH':'Weather_BackendScreenWidth()','SCREEN_HEIGHT':'Weather_BackendScreenHeight()','CParticle::SIN_COS_TABLE_SIZE':'WEATHER_TRIG_TABLE_SIZE','CParticle::m_CosTable[angle]':'Weather_BackendCosValue(angle)','CParticle::m_SinTable[angle]':'Weather_BackendSinValue(angle)','TheCamera.Orientation':'Weather_BackendCameraOrientation()','TheCamera.m_CameraAverageSpeed':'Weather_BackendCameraSpeed()','TheCamera.pTargetEntity->GetPosition()':'Weather_BackendEntityPosition(Weather_BackendTarget())','TheCamera.pTargetEntity':'Weather_BackendTarget()','CWorld::FindGroundZFor3DCoord(':'Weather_BackendGroundZ(','CParticle::RemovePSystem(':'Weather_BackendRemoveParticles(','TheCamera.GetPosition()':'Weather_BackendCameraPosition()','TheCamera.GetForward()':'Weather_BackendCameraForward()','TheCamera.GetRight()':'Weather_BackendCameraRight()','TheCamera.GetUp()':'Weather_BackendCameraUp()','TheCamera.m_RealPreviousCameraPosition':'Weather_BackendCameraPrevious()','CTimeCycle::GetFogReduction()':'TimeCycle_GetFogReduction()','CustomPipes::bRenderingEnvMap':'Weather_BackendEnvMap()','CustomPipes::EnvMapCam':'Weather_BackendEnvCamera()'}
    for old,new in native.items():text=text.replace(old,new)
    text=text.replace('FindPlayerSpeed().x','Weather_BackendPlayerSpeedX()').replace('FindPlayerSpeed().y','Weather_BackendPlayerSpeedY()')
    text=text.replace('CCam::MODE_TOPDOWN','CAMERA_MODE_TOPDOWN').replace('CCam::MODE_TOP_DOWN_PED','CAMERA_MODE_TOP_DOWN_PED')
    text=re.sub(r'\b(int32|int)\(([^()]*)\)',lambda match:'(' + ('int32_t' if match[1]=='int32' else 'int')+')('+match[2]+')',text)
    text=text.replace('CVector(', 'Weather_MakeVector(').replace('CVector','WeatherVector').replace('RwRGBA','WeatherColor')
    text=text.replace('CParticle::AddParticle(PARTICLE_HEATHAZE_IN_DIST','Weather_BackendParticleSimple(PARTICLE_HEATHAZE_IN_DIST').replace('CParticle::AddParticle(PARTICLE_BEASTIE','Weather_BackendParticleSimple(PARTICLE_BEASTIE')
    text=text.replace('CParticle::AddParticle(PARTICLE_RAINDROP_2D','Weather_BackendParticleDrop(PARTICLE_RAINDROP_2D')
    text=text.replace('CParticle::AddParticle(','Weather_BackendParticleColor(')
    text=re.sub(r'Weather_BackendParticle(?:Color|Drop)\([^\n]*',lambda match:match[0].replace(', nil,',','),text)
    text=text.replace('pos+Weather_MakeVector(0.0f, 0.0f, 40.0f)','Weather_Add(pos,Weather_MakeVector(0.0f,0.0f,40.0f))')
    text=text.replace('CColPoint point;\n\t\t\tCEntity *ent;','float hitHeight;')
    text=text.replace('CWorld::ProcessVerticalLine(Weather_Add(pos,Weather_MakeVector(0.0f,0.0f,40.0f)), -40.0f, point, ent, true, false, false, false, true, false, nil)','Weather_BackendVerticalLine(Weather_Add(pos,Weather_MakeVector(0.0f,0.0f,40.0f)),&hitHeight)').replace('point.point.z','hitHeight')
    text=text.replace('RenderOneRainStreak(', 'Weather_RenderOneRainStreak(').replace('const WeatherVector &cameraRight','WeatherVector cameraRight').replace('const WeatherVector &cameraUp','WeatherVector cameraUp')
    for name in ['dir','streakCameraRight','streakCameraUp']:text=text.replace(name+'.Normalise()','Weather_Normalise(&'+name+')')
    text=text.replace('Weather_Streaks[i].position + secondsElapsed * Weather_Streaks[i].direction','Weather_Add(Weather_Streaks[i].position,Weather_Scale(secondsElapsed,Weather_Streaks[i].direction))').replace('(pos - streakCameraPosition).Magnitude()','Weather_Magnitude(Weather_Sub(pos,streakCameraPosition))')
    text=text.replace('6.0f * Weather_BackendCameraForward() + Weather_BackendCameraPosition() + Weather_MakeVector(-1.8f * Weather_Streaks[i].direction.x, -1.8f * Weather_Streaks[i].direction.y, 8.0f)','Weather_Add(Weather_Add(Weather_Scale(6.0f,Weather_BackendCameraForward()),Weather_BackendCameraPosition()),Weather_MakeVector(-1.8f*Weather_Streaks[i].direction.x,-1.8f*Weather_Streaks[i].direction.y,8.0f))')
    text=text.replace('Weather_Streaks[i].position += (Weather_BackendCameraPosition() - Weather_BackendCameraPrevious()) * 20.0f','Weather_Streaks[i].position=Weather_Add(Weather_Streaks[i].position,Weather_ScaleRight(Weather_Sub(Weather_BackendCameraPosition(),Weather_BackendCameraPrevious()),20.0f))')
    text=text.replace('RwCamera *effectCamera = (RwCamera*)Weather_BackendEnvCamera()','void *effectCamera = Weather_BackendEnvCamera()').replace('RwFrame *effectFrame = RwCameraGetFrame(effectCamera)','void *effectFrame = Weather_BackendCameraFrame(effectCamera)').replace('RwFrameGetMatrix(effectFrame)','Weather_BackendFrameMatrix(effectFrame)').replace('RwMatrix *effectMatrix','void *effectMatrix')
    for field,name in [('pos','MatrixPosition'),('right','MatrixRight'),('up','MatrixUp')]:text=text.replace('Weather_MakeVector(effectMatrix->'+field+')','Weather_Backend'+name+'(effectMatrix)')
    text=re.sub(r'RwIm3DVertexSet(RGBA|Pos|U|V)\(&TempBufferRenderVertices\[([^\]]+)\],',r'Weather_BackendVertex\1(\2,',text)
    text=text.replace('RwTextureGetRaster(gpRainDropTex)','Weather_BackendRaster()')
    text=re.sub(r'RwRenderStateSet\((\w+), \(void\*\)(\w+)\)',r'Weather_BackendRenderState(\1,(uintptr_t)\2)',text).replace('RwRenderStateSet(rwRENDERSTATETEXTURERASTER, Weather_BackendRaster())','Weather_BackendRenderState(rwRENDERSTATETEXTURERASTER,(uintptr_t)Weather_BackendRaster())')
    text=text.replace('RwIm3DTransform(TempBufferRenderVertices, TempBufferVerticesStored, nil, 1)','Weather_BackendTransform(TempBufferVerticesStored)').replace('RwIm3DRenderIndexedPrimitive(rwPRIMTYPETRILIST, TempBufferRenderIndexList, TempBufferIndicesStored)','Weather_BackendRenderIndexed(TempBufferIndicesStored)').replace('RwIm3DEnd()','Weather_BackendEnd()')
    for name in STATES:text=re.sub(r'\b'+name+r'\b','WEATHER_'+name,text)
    for old,new in [('uint8','uint8_t'),('uint16','uint16_t'),('uint32','uint32_t'),('int16','int16_t'),('int32','int32_t'),('nil','NULL'),('TRUE','true'),('FALSE','false'),('ARRAY_SIZE','WEATHER_ARRAY_SIZE'),('Abs','fabsf'),('Sin','sinf'),('Cos','cosf')]:text=re.sub(r'\b'+old+r'\b',new,text)
    text=re.sub(r'(?m)^\{\n','{\n    // Preserve the complete original weather sequence and every live state access\n',text)
    code=re.sub(r'//[^\n]*|"(?:\\.|[^"\\])*"','',text)
    remaining=list(re.finditer(r'::|\b(?:CWeather|CVector|CColPoint|TheCamera|CParticle|RwIm3D|RwRenderStateSet)\b',code))
    assert not remaining,[code[max(0,match.start()-40):match.end()+100] for match in remaining]
    return '''//+ rouz edit (ChatGPT)
#include "Weather_backend.h"
#include "Timecycle.h"
#include "Timer.h"
#include "Clock.h"
#include "General.h"
#include "ZoneCull.h"
#include "SpecialFX_control.h"
#include "RenderBuffer.h"
#include "CameraModes.h"
#include "DMAudio.h"
#include <stddef.h>
#include <math.h>
#define WEATHER_ARRAY_SIZE(array) (sizeof(array)/sizeof((array)[0]))
#define Min(a,b) ((a)<(b)?(a):(b))
#define Max(a,b) ((a)>(b)?(a):(b))
#define Clamp(v,low,high) ((v)<(low)?(low):(v)>(high)?(high):(v))
#define TWOPI (((float)3.14159265358979323846)*2)
static WeatherVector Weather_MakeVector(float x,float y,float z)
{
    // Materialize the original scalar coordinates as a plain C value packet
    WeatherVector value={x,y,z};return value;
}
static WeatherVector Weather_Add(WeatherVector a,WeatherVector b)
{
    // Preserve the original component-wise addition order
    return Weather_MakeVector(a.x+b.x,a.y+b.y,a.z+b.z);
}
static WeatherVector Weather_Sub(WeatherVector a,WeatherVector b)
{
    // Preserve the original component-wise subtraction order
    return Weather_MakeVector(a.x-b.x,a.y-b.y,a.z-b.z);
}
static WeatherVector Weather_Scale(float scale,WeatherVector value)
{
    // Retain the original scalar-left multiplication for all three components
    return Weather_MakeVector(scale*value.x,scale*value.y,scale*value.z);
}
static WeatherVector Weather_ScaleRight(WeatherVector value,float scale)
{
    // Retain the original vector-left multiplication for all three components
    return Weather_MakeVector(value.x*scale,value.y*scale,value.z*scale);
}
static float Weather_Magnitude(WeatherVector value)
{
    // Match the original ordered three-component squared magnitude and square root
    return sqrtf(value.x*value.x+value.y*value.y+value.z*value.z);
}
static void Weather_Normalise(WeatherVector *value)
{
    // Preserve the original native vector normalization including its fallback
    float sq=value->x*value->x+value->y*value->y+value->z*value->z;
    if(sq>0.0f){
        // Scale all original components by the reciprocal square root
        float inverse=1.0f/sqrtf(sq);value->x*=inverse;value->y*=inverse;value->z*=inverse;
    }else{
        // Leave the other components unchanged in the original zero-direction branch
        value->x=1.0f;
    }
}
'''+text+'\n//- rouz edit (ChatGPT)\n'

def weather_calls(source):
    # Replace the static class and every complete caller without breaking macro continuations
    if 'int32 CWeather::SoundHandle' in source:
        if OBSERVER:OBSERVER(source)
        return HOST
    if 'class CWeather\n' in source:return HEADER
    if 'CWeather::' not in source:return source
    changed=source.replace('CWeather::','Weather_')
    if source.lstrip().startswith('//+ rouz edit (ChatGPT)') and source.rstrip().endswith('//- rouz edit (ChatGPT)'):return changed
    return '//+ rouz edit (ChatGPT)\n'+changed.rstrip('\n')+'\n//- rouz edit (ChatGPT)\n'

def calls(source):
    # Forward every complete historical caller through the separately verified pedestrian conversion
    return pedestrian.calls(weather_calls(source))
def project(source):
    # Register the uniquely named C11 weather controller and native boundary declarations
    entry='<ClCompile Include="..\\src\\renderer\\Weather.cpp" />'
    if '..\\src\\renderer\\WeatherSystem.c"' not in source:source=source.replace(entry,entry+'\n    <ClCompile Include="..\\src\\renderer\\WeatherSystem.c">\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>')
    entry='<ClInclude Include="..\\src\\renderer\\Weather.h" />'
    if '..\\src\\renderer\\Weather_backend.h"' not in source:source=source.replace(entry,entry+'\n    <ClInclude Include="..\\src\\renderer\\Weather_backend.h" />')
    return pedestrian.project(source)

def verify_legacy(source,transform,root):
    # Retain complete earlier weather snapshots as proofs of both relocated C algorithms and native bindings
    global OBSERVER
    captured=[];saved=OBSERVER;OBSERVER=captured.append
    try:expected=transform(source)
    finally:OBSERVER=saved
    assert len(captured)==1
    normalize=lambda value:re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*[\s\S]*?\*/','',value))
    assert normalize(expected)==normalize((root/'src/renderer/Weather.cpp').read_text())
    assert normalize(kernel(captured[0]))==normalize((root/'src/renderer/WeatherSystem.c').read_text())

def verify_dependency(saved,current):
    # Keep byte guards unless the complete immutable source differs by the explicit weather migration
    if saved.read_bytes()==current.read_bytes():return
    original=saved.read_text();expected=calls(original)
    assert expected!=original and expected==current.read_text(),str(current)

def legacy_member_spelling(source):
    # Restore only known field spellings while an older body translator binds its original host calls
    for kind,name in SCALARS:source=re.sub(r'\bWeather_'+name+r'\b','CWeather::'+name,source)
    return source
