//+ rouz edit (ChatGPT)
#include "common.h"
#include "Weather_backend.h"
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
static_assert(int(WEATHER_rwRENDERSTATETEXTURERASTER)==int(rwRENDERSTATETEXTURERASTER),"Original native render constant");
static_assert(int(WEATHER_rwRENDERSTATEZTESTENABLE)==int(rwRENDERSTATEZTESTENABLE),"Original native render constant");
static_assert(int(WEATHER_rwRENDERSTATEZWRITEENABLE)==int(rwRENDERSTATEZWRITEENABLE),"Original native render constant");
static_assert(int(WEATHER_rwRENDERSTATESRCBLEND)==int(rwRENDERSTATESRCBLEND),"Original native render constant");
static_assert(int(WEATHER_rwRENDERSTATEDESTBLEND)==int(rwRENDERSTATEDESTBLEND),"Original native render constant");
static_assert(int(WEATHER_rwRENDERSTATEVERTEXALPHAENABLE)==int(rwRENDERSTATEVERTEXALPHAENABLE),"Original native render constant");
static_assert(int(WEATHER_rwRENDERSTATEFOGENABLE)==int(rwRENDERSTATEFOGENABLE),"Original native render constant");
static_assert(int(WEATHER_rwRENDERSTATEFOGTYPE)==int(rwRENDERSTATEFOGTYPE),"Original native render constant");
static_assert(int(WEATHER_rwFOGTYPELINEAR)==int(rwFOGTYPELINEAR),"Original native render constant");
static_assert(int(WEATHER_rwBLENDONE)==int(rwBLENDONE),"Original native render constant");
static_assert(int(WEATHER_rwBLENDSRCALPHA)==int(rwBLENDSRCALPHA),"Original native render constant");
static_assert(int(WEATHER_rwBLENDINVSRCALPHA)==int(rwBLENDINVSRCALPHA),"Original native render constant");

bool Weather_BackendNoHurricanes(void)
{
    // Access the original typed native operation without moving its weather decisions
    return Stats_NoMoreHurricanes;
}
bool Weather_BackendShockDown(void)
{
    // Access the original typed native operation without moving its weather decisions
    return CPad::GetPad(1)->GetRightShockJustDown();
}
void Weather_BackendShake(int16_t duration,uint8_t frequency)
{
    // Access the original typed native operation without moving its weather decisions
    CPad::GetPad(0)->StartShake(duration,frequency);
}
float Weather_BackendSunZ(void)
{
    // Access the original typed native operation without moving its weather decisions
    return TimeCycle_GetSunDirection()->z;
}
bool Weather_BackendInterior(void)
{
    // Access the original typed native operation without moving its weather decisions
    return CGame::IsInInterior();
}
bool Weather_BackendCutscene(void)
{
    // Access the original typed native operation without moving its weather decisions
    return Cutscene_IsRunning(); // rouz edit (ChatGPT)
}
bool Weather_BackendPlayerExists(void)
{
    // Access the original typed native operation without moving its weather decisions
    return FindPlayerPed()!=nil;
}
bool Weather_BackendPlayerInAir(void)
{
    // Access the original typed native operation without moving its weather decisions
    return FindPlayerPed()->CheckIfInTheAir();
}
float Weather_BackendPlayerZ(void)
{
    // Access the original typed native operation without moving its weather decisions
    return FindPlayerPed()->GetPosition().z;
}
float Weather_BackendPlayerSpeedX(void)
{
    // Access the original typed native operation without moving its weather decisions
    return FindPlayerSpeed().x;
}
float Weather_BackendPlayerSpeedY(void)
{
    // Access the original typed native operation without moving its weather decisions
    return FindPlayerSpeed().y;
}
CVehicle * Weather_BackendPlayerVehicle(void)
{
    // Access the original typed native operation without moving its weather decisions
    return FindPlayerVehicle();
}
bool Weather_BackendVehicleRoof(CVehicle *car)
{
    // Access the original typed native operation without moving its weather decisions
    return car->CarHasRoof();
}
int16_t Weather_BackendCameraMode(void)
{
    // Access the original typed native operation without moving its weather decisions
    return TheCamera.Cams[TheCamera.ActiveCam].Mode;
}
bool Weather_BackendCameraForwardFirstPerson(void)
{
    // Access the original typed native operation without moving its weather decisions
    return TheCamera.GetLookingForwardFirstPerson();
}
bool Weather_BackendCameraLRBFirstPerson(void)
{
    // Access the original typed native operation without moving its weather decisions
    return TheCamera.GetLookingLRBFirstPerson();
}
float Weather_BackendScreenWidth(void)
{
    // Access the original typed native operation without moving its weather decisions
    return SCREEN_WIDTH;
}
float Weather_BackendScreenHeight(void)
{
    // Access the original typed native operation without moving its weather decisions
    return SCREEN_HEIGHT;
}
float Weather_BackendCosValue(int32_t angle)
{
    // Access the original typed native operation without moving its weather decisions
    return CParticle::m_CosTable[angle];
}
float Weather_BackendSinValue(int32_t angle)
{
    // Access the original typed native operation without moving its weather decisions
    return CParticle::m_SinTable[angle];
}
float Weather_BackendCameraOrientation(void)
{
    // Access the original typed native operation without moving its weather decisions
    return TheCamera.Orientation;
}
float Weather_BackendCameraSpeed(void)
{
    // Access the original typed native operation without moving its weather decisions
    return TheCamera.m_CameraAverageSpeed;
}
CEntity * Weather_BackendTarget(void)
{
    // Access the original typed native operation without moving its weather decisions
    return TheCamera.pTargetEntity;
}
WeatherVector Weather_BackendEntityPosition(CEntity *entity)
{
    // Access the original typed native operation without moving its weather decisions
    const CVector &value=entity->GetPosition();return WeatherVector{value.x,value.y,value.z};
}
float Weather_BackendGroundZ(float x,float y,float z,bool *found)
{
    // Access the original typed native operation without moving its weather decisions
    return CWorld::FindGroundZFor3DCoord(x,y,z,found);
}
void Weather_BackendRemoveParticles(enum tParticleType type)
{
    // Access the original typed native operation without moving its weather decisions
    CParticle::RemovePSystem(type);
}
void Weather_BackendParticleSimple(enum tParticleType type,WeatherVector position,WeatherVector direction)
{
    // Access the original typed native operation without moving its weather decisions
    CParticle::AddParticle(type,CVector(position.x,position.y,position.z),CVector(direction.x,direction.y,direction.z));
}
void Weather_BackendParticleColor(enum tParticleType type,WeatherVector position,WeatherVector direction,float size,WeatherColor value)
{
    // Access the original typed native operation without moving its weather decisions
    RwRGBA color={value.red,value.green,value.blue,value.alpha};CParticle::AddParticle(type,CVector(position.x,position.y,position.z),CVector(direction.x,direction.y,direction.z),nil,size,color);
}
void Weather_BackendParticleDrop(enum tParticleType type,WeatherVector position,WeatherVector direction,float size,int32_t rotationSpeed,int32_t rotation,int32_t frame,int32_t lifespan)
{
    // Access the original typed native operation without moving its weather decisions
    CParticle::AddParticle(type,CVector(position.x,position.y,position.z),CVector(direction.x,direction.y,direction.z),nil,size,rotationSpeed,rotation,frame,lifespan);
}
bool Weather_BackendVerticalLine(WeatherVector value,float *height)
{
    // Access the original typed native operation without moving its weather decisions
    CVector position(value.x,value.y,value.z);CColPoint point;CEntity *entity;bool hit=CWorld::ProcessVerticalLine(position,-40.0f,point,entity,true,false,false,false,true,false,nil);
    if(hit){
        // Copy only the valid collision height before the C controller decides whether to add splashes
        *height=point.point.z;
    }
    return hit;
}
bool Weather_BackendEnvMap(void)
{
    // Access the original typed native operation without moving its weather decisions
    #if defined(LIBRW) && defined(EXTENDED_PIPELINES)
    return CustomPipes::bRenderingEnvMap;
#else
    return false;
#endif
}
void * Weather_BackendEnvCamera(void)
{
    // Access the original typed native operation without moving its weather decisions
    #if defined(LIBRW) && defined(EXTENDED_PIPELINES)
    return CustomPipes::EnvMapCam;
#else
    return nil;
#endif
}
void * Weather_BackendCameraFrame(void *camera)
{
    // Access the original typed native operation without moving its weather decisions
    return RwCameraGetFrame((RwCamera*)camera);
}
void * Weather_BackendFrameMatrix(void *frame)
{
    // Access the original typed native operation without moving its weather decisions
    return RwFrameGetMatrix((RwFrame*)frame);
}
void Weather_BackendVertexRGBA(int32_t index,int32_t red,int32_t green,int32_t blue,int32_t alpha)
{
    // Access the original typed native operation without moving its weather decisions
    RwIm3DVertexSetRGBA(&TempBufferRenderVertices[index],red,green,blue,alpha);
}
void Weather_BackendVertexPos(int32_t index,float x,float y,float z)
{
    // Access the original typed native operation without moving its weather decisions
    RwIm3DVertexSetPos(&TempBufferRenderVertices[index],x,y,z);
}
void Weather_BackendVertexU(int32_t index,float u)
{
    // Access the original typed native operation without moving its weather decisions
    RwIm3DVertexSetU(&TempBufferRenderVertices[index],u);
}
void Weather_BackendVertexV(int32_t index,float v)
{
    // Access the original typed native operation without moving its weather decisions
    RwIm3DVertexSetV(&TempBufferRenderVertices[index],v);
}
void Weather_BackendRenderState(int32_t state,uintptr_t value)
{
    // Access the original typed native operation without moving its weather decisions
    RwRenderStateSet((RwRenderState)state,(void*)value);
}
void * Weather_BackendRaster(void)
{
    // Access the original typed native operation without moving its weather decisions
    return RwTextureGetRaster(gpRainDropTex);
}
bool Weather_BackendTransform(int32_t vertices)
{
    // Access the original typed native operation without moving its weather decisions
    return RwIm3DTransform(TempBufferRenderVertices,vertices,nil,1)!=nil;
}
void Weather_BackendRenderIndexed(int32_t indices)
{
    // Access the original typed native operation without moving its weather decisions
    RwIm3DRenderIndexedPrimitive(rwPRIMTYPETRILIST,TempBufferRenderIndexList,indices);
}
void Weather_BackendEnd(void)
{
    // Access the original typed native operation without moving its weather decisions
    RwIm3DEnd();
}
WeatherVector Weather_BackendCameraPosition(void)
{
    // Access the original typed native operation without moving its weather decisions
    const CVector &value=TheCamera.GetPosition();return WeatherVector{value.x,value.y,value.z};
}
WeatherVector Weather_BackendCameraForward(void)
{
    // Access the original typed native operation without moving its weather decisions
    const CVector &value=TheCamera.GetForward();return WeatherVector{value.x,value.y,value.z};
}
WeatherVector Weather_BackendCameraRight(void)
{
    // Access the original typed native operation without moving its weather decisions
    const CVector &value=TheCamera.GetRight();return WeatherVector{value.x,value.y,value.z};
}
WeatherVector Weather_BackendCameraUp(void)
{
    // Access the original typed native operation without moving its weather decisions
    const CVector &value=TheCamera.GetUp();return WeatherVector{value.x,value.y,value.z};
}
WeatherVector Weather_BackendCameraPrevious(void)
{
    // Access the original typed native operation without moving its weather decisions
    const CVector &value=TheCamera.m_RealPreviousCameraPosition;return WeatherVector{value.x,value.y,value.z};
}
WeatherVector Weather_BackendMatrixPosition(void *matrix)
{
    // Access the original typed native operation without moving its weather decisions
    RwMatrix *value=(RwMatrix*)matrix;return WeatherVector{value->pos.x,value->pos.y,value->pos.z};
}
WeatherVector Weather_BackendMatrixRight(void *matrix)
{
    // Access the original typed native operation without moving its weather decisions
    RwMatrix *value=(RwMatrix*)matrix;return WeatherVector{value->right.x,value->right.y,value->right.z};
}
WeatherVector Weather_BackendMatrixUp(void *matrix)
{
    // Access the original typed native operation without moving its weather decisions
    RwMatrix *value=(RwMatrix*)matrix;return WeatherVector{value->up.x,value->up.y,value->up.z};
}
//- rouz edit (ChatGPT)
