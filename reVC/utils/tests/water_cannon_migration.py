"""Move every water-cannon algorithm and preserve typed input/owner boundaries."""
import re
import object_names_migration as objects
import point_lights_migration as point_lights
HEADER='''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#define WATERCANNON_GRAVITY (0.009f)
#define WATERCANNON_LIFETIME (150)
enum { WATERCANNON_SEGMENTS = 16 };
typedef struct WaterCannonVector { float x, y, z; } WaterCannonVector;
typedef struct WaterCannon {
	int32_t m_nId;
	int16_t m_nCur;
	uint32_t m_nTimeCreated;
	WaterCannonVector m_avecPos[WATERCANNON_SEGMENTS];
	WaterCannonVector m_avecVelocity[WATERCANNON_SEGMENTS];
	bool m_abUsed[WATERCANNON_SEGMENTS];
} WaterCannon;
#ifdef __cplusplus
extern "C" {
#endif
extern WaterCannon WaterCannons_aCannons[NUM_WATERCANNONS];
extern uint16_t WaterCannonIndexList[12];
void WaterCannon_Init(WaterCannon *cannon);
void WaterCannon_Update_OncePerFrame(WaterCannon *cannon, int16_t index);
void WaterCannon_Update_NewInput(WaterCannon *cannon, const WaterCannonVector *pos, const WaterCannonVector *dir);
void WaterCannon_Update_NewInputLegacy(WaterCannon *cannon, const void *pos, const void *dir);
void WaterCannon_Render(WaterCannon *cannon);
void WaterCannon_PushPeds(WaterCannon *cannon);
void WaterCannons_Init(void);
void WaterCannons_UpdateOne(uint32_t id, const WaterCannonVector *pos, const WaterCannonVector *dir);
void WaterCannons_UpdateOneLegacy(uint32_t id, const void *pos, const void *dir);
void WaterCannons_Update(void);
void WaterCannons_Render(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
READERS=[
('WaterCannonVector','ReadVector','const void *value','const CVector *v = (const CVector *)value;\n\tWaterCannonVector result = {v->x, v->y, v->z};\n\treturn result;','Read the original typed legacy input only at its copy point'),
('WaterCannonVector','CameraForward','void','const CVector &v = TheCamera.GetForward();\n\tWaterCannonVector result = {v.x, v.y, v.z};\n\treturn result;','Copy the original typed camera forward value'),
('bool','EnvMap','void','#if defined(LIBRW) && defined(EXTENDED_PIPELINES)\n\treturn CustomPipes::bRenderingEnvMap;\n#else\n\treturn false;\n#endif','Read the live environment-pass flag at each original access'),
('void *','EnvCamera','void','#if defined(LIBRW) && defined(EXTENDED_PIPELINES)\n\treturn CustomPipes::EnvMapCam;\n#else\n\treturn NULL;\n#endif','Read the original auxiliary camera pointer'),
('void *','CameraFrame','void *camera','return RwCameraGetFrame((RwCamera *)camera);','Query the actual typed auxiliary camera frame'),
('void *','FrameMatrix','void *frame','return RwFrameGetMatrix((RwFrame *)frame);','Query the actual typed frame matrix'),
('WaterCannonVector','MatrixForward','void *matrix','const RwV3d &v = ((RwMatrix *)matrix)->at;\n\tWaterCannonVector result = {v.x, v.y, v.z};\n\treturn result;','Copy the actual matrix forward vector before C normalization'),
('bool','Extinguish','WaterCannonVector pos, float range','return gFireManager.ExtinguishPointWithWater(CVector(pos.x, pos.y, pos.z), range);','Enter the original fire query at its eligibility gate'),
('CPool *','PedPool','void','return CPools::GetPedPool();','Read the original live ped pool at every query'),
('CPed *','Ped','void *slot','return (CPed *)slot;','Convert actual raw pool storage through the original typed owner'),
('float','PedX','CPed *ped','return ped->GetPosition().x;','Read only the original live ped X component'),
('float','PedY','CPed *ped','return ped->GetPosition().y;','Read only the original live ped Y component'),
('float','PedZ','CPed *ped','return ped->GetPosition().z;','Read only the original live ped Z component'),
('WaterCannonVector','PedPosition','CPed *ped','const CVector &v = ped->GetPosition();\n\tWaterCannonVector result = {v.x, v.y, v.z};\n\treturn result;','Copy the original complete ped position at its use point'),
('int32_t','LocalDirection','CPed *ped','return ped->GetLocalDirection(CVector2D(1.0f, 0.0f));','Enter the original typed local-direction query'),
('void','Standing','CPed *ped','ped->bIsStanding = false;','Clear the original standing bit through its actual owner'),
('void','Force','CPed *ped, float z','ped->ApplyMoveForce(0.0f, 0.0f, z);','Apply the original typed vertical force after C computes its magnitude'),
('float','SpeedX','CPed *ped','return ped->m_vecMoveSpeed.x;','Read the current ped X velocity at its original use'),
('float','SpeedY','CPed *ped','return ped->m_vecMoveSpeed.y;','Read the current ped Y velocity at its original use'),
('void','SetSpeedX','CPed *ped, float value','ped->m_vecMoveSpeed.x = value;','Write the original ped X velocity at its original point'),
('void','SetSpeedY','CPed *ped, float value','ped->m_vecMoveSpeed.y = value;','Write the original ped Y velocity at its original point'),
('WaterCannonVector','Velocity','CPed *ped','const CVector &v = ped->m_vecMoveSpeed;\n\tWaterCannonVector result = {v.x, v.y, v.z};\n\treturn result;','Copy the original full ped velocity for particle direction math'),
('void','Fall','CPed *ped, int32_t animation','ped->SetFall(2000, (AnimationId)animation, 0);','Enter the original typed fall animation after C selects its ID'),
('void','Particle','int32_t kind, WaterCannonVector pos, WaterCannonVector dir, int32_t rotationSpeed, int32_t rotation, int32_t frame','CParticle::AddParticle((tParticleType)kind, CVector(pos.x, pos.y, pos.z), CVector(dir.x, dir.y, dir.z), NULL, 0.5f, rotationSpeed, rotation, frame);','Enter the original typed particle overload with C-selected values'),
('void','SetU','int index, float value','RwIm3DVertexSetU(&WaterCannonVertices[index], value);','Write the original typed shared U coordinate'),
('void','SetV','int index, float value','RwIm3DVertexSetV(&WaterCannonVertices[index], value);','Write the original typed shared V coordinate'),
('void','SetRGBA','int index, int32_t r, int32_t g, int32_t b, int32_t a','RwIm3DVertexSetRGBA(&WaterCannonVertices[index], r, g, b, a);','Retain final narrowing in the actual typed color setter'),
('void','SetPos','int index, float x, float y, float z','RwIm3DVertexSetPos(&WaterCannonVertices[index], x, y, z);','Write the original typed shared vertex position'),
('void','ZWrite','bool enabled','RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)(uintptr_t)enabled);','Set the original typed depth-write state'),
('void','VertexAlpha','bool enabled','RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)(uintptr_t)enabled);','Set the original typed vertex-alpha state'),
('void','Fog','bool enabled','RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)(uintptr_t)enabled);','Set the original typed fog state'),
('void','Raster','void','RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void *)gpWaterRaster);','Select the original live water raster'),
('void','LittleTest','void','LittleTest();','Enter the original hook after the four vertices are generated'),
('bool','Transform','void','return RwIm3DTransform(WaterCannonVertices, 4, NULL, rwIM3D_VERTEXUV) != NULL;','Retain the original buffer, count, null matrix and UV flag'),
('void','RenderIndexed','uint16_t *indices','RwIm3DRenderIndexedPrimitive(rwPRIMTYPETRILIST, indices, 12);','Draw the original twelve indices as a triangle list'),
('void','End','void','RwIm3DEnd();','Finish the original successful draw'),
('void','Push','const char *name','PUSH_RENDERGROUP(name);','Preserve the original debug group boundary'),
('void','Pop','void','POP_RENDERGROUP();','Close the original debug group when enabled')]
BACKEND_HEADER='//+ rouz edit (ChatGPT)\n#pragma once\n#include "WaterCannon.h"\n#include "Pool.h"\n#ifdef __cplusplus\nclass CPed;\nextern "C" {\n#else\ntypedef struct CPed CPed;\n#endif\n'+''.join(t+' WaterCannon_Backend'+name+'('+args+');\n' for t,name,args,body,comment in READERS)+'#ifdef __cplusplus\n}\n#endif\n//- rouz edit (ChatGPT)\n'
HOST='''//+ rouz edit (ChatGPT)
#include "common.h"
#include "main.h"
#include "Pools.h"
#include "Ped.h"
#include "AnimManager.h"
#include "Fire.h"
#include "WaterLevel.h"
#include "Camera.h"
#include "Particle.h"
#include "WaterCannon_backend.h"
#if defined(LIBRW) && defined(EXTENDED_PIPELINES)
#include "custompipes.h"
#endif
RwIm3DVertex WaterCannonVertices[4];

'''+''.join(t+' WaterCannon_Backend'+name+'('+args+')\n{\n\t// '+comment+'\n\t'+body+'\n}\n' for t,name,args,body,comment in READERS)+'//- rouz edit (ChatGPT)\n'
HELPERS='''#define Min(a,b) ((a) < (b) ? (a) : (b))
#define Max(a,b) ((a) > (b) ? (a) : (b))
void re3_assert(const char *, const char *, unsigned int, const char *);
#ifndef MASTER
#define WC_ASSERT(expr) (void)((!!(expr)) || (re3_assert(#expr, __FILE__, __LINE__, __func__), 0))
#else
#define WC_ASSERT(expr) (expr)
#endif
typedef WaterCannonVector (*WaterCannonRead)(const void *);
static WaterCannonVector WaterCannon_Read(const void *value)
{
	// Read actual C vector storage only at the original ordered copy point
	return *(const WaterCannonVector *)value;
}
static WaterCannonVector WaterCannon_Add(WaterCannonVector a, WaterCannonVector b)
{
	// Retain the original component addition order
	WaterCannonVector result = {a.x+b.x, a.y+b.y, a.z+b.z};
	return result;
}
static WaterCannonVector WaterCannon_Subtract(WaterCannonVector a, WaterCannonVector b)
{
	// Retain all original component subtractions
	WaterCannonVector result = {a.x-b.x, a.y-b.y, a.z-b.z};
	return result;
}
static WaterCannonVector WaterCannon_Scale(WaterCannonVector a, float b)
{
	// Retain original vector-times-scalar operand order
	WaterCannonVector result = {a.x*b, a.y*b, a.z*b};
	return result;
}
static WaterCannonVector WaterCannon_LeftScale(float a, WaterCannonVector b)
{
	// Retain original scalar-times-vector operand order
	WaterCannonVector result = {a*b.x, a*b.y, a*b.z};
	return result;
}
static float WaterCannon_MagnitudeSqr(WaterCannonVector v)
{
	// Retain the original three float products and additions
	return v.x*v.x + v.y*v.y + v.z*v.z;
}
static float WaterCannon_Magnitude(WaterCannonVector v)
{
	// Use the measured original float square root
	return sqrtf(WaterCannon_MagnitudeSqr(v));
}
static float WaterCannon_PedSpeed(CPed *ped)
{
	// Read the original planar speed before computing its float magnitude
	float x = WaterCannon_BackendSpeedX(ped), y = WaterCannon_BackendSpeedY(ped);
	return sqrtf(x*x + y*y);
}
static WaterCannonVector WaterCannon_Cross(WaterCannonVector a, WaterCannonVector b)
{
	// Retain the complete original cross-product expression
	WaterCannonVector result = {a.y*b.z-a.z*b.y, a.z*b.x-a.x*b.z, a.x*b.y-a.y*b.x};
	return result;
}
static void WaterCannon_Normalise(WaterCannonVector *v)
{
	// Retain the original float reciprocal root and x-only fallback
	float sq = WaterCannon_MagnitudeSqr(*v);
	if (sq > 0.0f) {
		// Scale all three original components in order
		float invsqrt = 1.0f/sqrtf(sq);
		v->x *= invsqrt;v->y *= invsqrt;v->z *= invsqrt;
	} else
		v->x = 1.0f;
}

'''
WRAPPERS='''void WaterCannon_Update_NewInput(WaterCannon *cannon, const WaterCannonVector *pos, const WaterCannonVector *dir)
{
	// Preserve assertions and ordered reads even when input pointers alias cannon fields
	WaterCannon_Input(cannon, pos, dir, WaterCannon_Read);
}
void WaterCannon_Update_NewInputLegacy(WaterCannon *cannon, const void *pos, const void *dir)
{
	// Read each legacy typed input only after the preceding original write
	WaterCannon_Input(cannon, pos, dir, WaterCannon_BackendReadVector);
}
void WaterCannons_UpdateOne(uint32_t id, const WaterCannonVector *pos, const WaterCannonVector *dir)
{
	// Enter the complete C manager with actual C vector inputs
	WaterCannons_Input(id, pos, dir, WaterCannon_Read);
}
void WaterCannons_UpdateOneLegacy(uint32_t id, const void *pos, const void *dir)
{
	// Enter the same C manager without eagerly copying legacy objects
	WaterCannons_Input(id, pos, dir, WaterCannon_BackendReadVector);
}
'''

def calls(source):
    # Convert the removed classes and preserve the existing cross-record audio indexing
    if 'class CWaterCannon\n' in source:return HEADER
    if 'CWaterCannon CWaterCannons::aCannons[NUM_WATERCANNONS];' in source:return HOST
    source=source.replace('*position = AudioSoundPosition_FromVector(CWaterCannons::aCannons[0].m_avecPos[CWaterCannons::aCannons[index].m_nCur]);','const WaterCannonVector *source = &WaterCannons_aCannons[0].m_avecPos[WaterCannons_aCannons[index].m_nCur];\n    AudioSoundPosition result = {source->x, source->y, source->z};\n    *position = result;')
    source=source.replace('CWaterCannons::aCannons','WaterCannons_aCannons')
    changed=bool(re.search(r'\bCWaterCannons::\w+\(',source))
    source=source.replace('CWaterCannons::UpdateOne(','WaterCannons_UpdateOneLegacy(')
    source=re.sub(r'\bCWaterCannons::(\w+)\(',r'WaterCannons_\1(',source)
    if changed:
        lines=[]
        for line in source.splitlines(keepends=True):
            if re.search(r'\bWaterCannons_(?:Init|Update|Render|UpdateOneLegacy)\(',line):
                comment=re.match(r'\s*',line)[0]+'// Enter the C water-cannon algorithm with the original owner expressions\n'
                if not lines or lines[-1]!=comment:lines.append(comment)
            lines.append(line)
        source=''.join(lines)
    return point_lights.calls(source)

def kernel(source):
    # Relocate all nine complete bodies and use one reader-aware implementation for both input APIs
    source=source[source.index('#define WATERCANNONVERTS'):]
    source=source.replace('RwIm3DVertex WaterCannonVertices[WATERCANNONVERTS];\n','').replace('RwImVertexIndex WaterCannonIndexList','uint16_t WaterCannonIndexList')
    source=source.replace('CWaterCannon CWaterCannons::aCannons[NUM_WATERCANNONS];','WaterCannon WaterCannons_aCannons[NUM_WATERCANNONS];')
    source=source.replace('void CWaterCannon::Update_NewInput(CVector *pos, CVector *dir)','static void WaterCannon_Input(WaterCannon *cannon, const void *pos, const void *dir, WaterCannonRead read)')
    source=source.replace('void CWaterCannon::Update_OncePerFrame(int16 index)','void WaterCannon_Update_OncePerFrame(WaterCannon *cannon, int16_t index)')
    for name in ['Init','Render','PushPeds']:source=source.replace('void CWaterCannon::'+name+'(void)','void WaterCannon_'+name+'(WaterCannon *cannon)')
    a=source.index('void WaterCannon_Init(');b=source.index('void CWaterCannons::Init(');body=source[a:b]
    body=re.sub(r'(?<![\w.>])\b(m_nId|m_nCur|m_nTimeCreated|m_avecPos|m_avecVelocity|m_abUsed)\b',r'cannon->\1',body)
    body=body.replace('= *pos;','= read(pos);').replace('= *dir;','= read(dir);')
    body=body.replace('PushPeds();','WaterCannon_PushPeds(cannon);')
    body=body.replace('cannon->m_avecPos[i]        += cannon->m_avecVelocity[i]    * (*Timer_GetTimeStep());','cannon->m_avecPos[i] = WaterCannon_Add(cannon->m_avecPos[i], WaterCannon_Scale(cannon->m_avecVelocity[i], (*Timer_GetTimeStep())));')
    body=body.replace('gFireManager.ExtinguishPointWithWater(','WaterCannon_BackendExtinguish(')
    body=body.replace('CVector cameraForward = TheCamera.GetForward();','WaterCannonVector cameraForward = WaterCannon_BackendCameraForward();')
    body=body.replace('CustomPipes::bRenderingEnvMap','WaterCannon_BackendEnvMap()').replace('CustomPipes::EnvMapCam','WaterCannon_BackendEnvCamera()')
    body=body.replace('RwFrame *cameraFrame = RwCameraGetFrame(WaterCannon_BackendEnvCamera());','void *cameraFrame = WaterCannon_BackendCameraFrame(WaterCannon_BackendEnvCamera());').replace('RwMatrix *cameraMatrix = RwFrameGetMatrix(cameraFrame);','void *cameraMatrix = WaterCannon_BackendFrameMatrix(cameraFrame);')
    body=body.replace('CVector(cameraMatrix->at)','WaterCannon_BackendMatrixForward(cameraMatrix)').replace('cameraForward.Normalise();','WaterCannon_Normalise(&cameraForward);')
    body=body.replace('CVector cp = CrossProduct(cannon->m_avecPos[pointB] - cannon->m_avecPos[pointA], cameraForward);','WaterCannonVector cp = WaterCannon_Cross(WaterCannon_Subtract(cannon->m_avecPos[pointB], cannon->m_avecPos[pointA]), cameraForward);')
    body=body.replace('cp * (0.05f / cp.Magnitude())','WaterCannon_Scale(cp, (0.05f / WaterCannon_Magnitude(cp)))').replace('CVector norm;','WaterCannonVector norm;').replace('CVector offset = dist * norm;','WaterCannonVector offset = WaterCannon_LeftScale(dist, norm);')
    body=body.replace('float(((*Timer_GetFrameCounter()) + cannon->m_nId) & 255)','(float)(((*Timer_GetFrameCounter()) + cannon->m_nId) & 255)').replace('float(General_GetRandomNumber() & 255)','(float)(General_GetRandomNumber() & 255)').replace('float(i*i*i)','(float)(i*i*i)').replace('float(i)','(float)(i)')
    body=body.replace('CPools::GetPedPool()','WaterCannon_BackendPedPool()').replace('((CPed*)CPool_GetSlot(WaterCannon_BackendPedPool(), i))','WaterCannon_BackendPed(CPool_GetSlot(WaterCannon_BackendPedPool(), i))')
    body=body.replace('ped->GetPosition().x','WaterCannon_BackendPedX(ped)').replace('ped->GetPosition().y','WaterCannon_BackendPedY(ped)').replace('ped->GetPosition().z','WaterCannon_BackendPedZ(ped)')
    body=body.replace('CVector dist = cannon->m_avecPos[j] - ped->GetPosition();','WaterCannonVector dist = WaterCannon_Subtract(cannon->m_avecPos[j], WaterCannon_BackendPedPosition(ped));').replace('dist.MagnitudeSqr()','WaterCannon_MagnitudeSqr(dist)')
    body=body.replace('ped->GetLocalDirection(CVector2D(1.0f, 0.0f))','WaterCannon_BackendLocalDirection(ped)').replace('ped->bIsStanding = false;','WaterCannon_BackendStanding(ped);')
    body=body.replace('ped->ApplyMoveForce(0.0f, 0.0f, 2.0f * (*Timer_GetTimeStep()));','WaterCannon_BackendForce(ped, 2.0f * (*Timer_GetTimeStep()));')
    for axis in ['x','y']:
        getter='WaterCannon_BackendSpeed'+axis.upper()+'(ped)'
        body=body.replace('ped->m_vecMoveSpeed.'+axis+' = (0.6f * cannon->m_avecVelocity[j].'+axis+' + ped->m_vecMoveSpeed.'+axis+') * 0.5f;','WaterCannon_BackendSetSpeed'+axis.upper()+'(ped, (0.6f * cannon->m_avecVelocity[j].'+axis+' + '+getter+') * 0.5f);')
        body=body.replace('ped->m_vecMoveSpeed.'+axis+' *= (0.2f / pedSpeed2D);','WaterCannon_BackendSetSpeed'+axis.upper()+'(ped, '+getter+' * (0.2f / pedSpeed2D));')
    body=body.replace('ped->m_vecMoveSpeed.Magnitude2D()','WaterCannon_PedSpeed(ped)').replace('ped->SetFall(2000, (AnimationId)(localDir + ANIM_STD_HIGHIMPACT_FRONT), 0);','WaterCannon_BackendFall(ped, localDir + ANIM_STD_HIGHIMPACT_FRONT);')
    body=body.replace('CParticle::AddParticle(PARTICLE_STEAM_NY_SLOWMOTION, ped->GetPosition(), ped->m_vecMoveSpeed * 0.3f, 0, 0.5f);','WaterCannon_BackendParticle(PARTICLE_STEAM_NY_SLOWMOTION, WaterCannon_BackendPedPosition(ped), WaterCannon_Scale(WaterCannon_BackendVelocity(ped), 0.3f), 0, 0, 0);')
    body=body.replace('CParticle::AddParticle(PARTICLE_CAR_SPLASH, ped->GetPosition(), ped->m_vecMoveSpeed * -0.3f + CVector(0.f, 0.f, 0.5f), 0, 0.5f,','WaterCannon_BackendParticle(PARTICLE_CAR_SPLASH, WaterCannon_BackendPedPosition(ped), WaterCannon_Add(WaterCannon_Scale(WaterCannon_BackendVelocity(ped), -0.3f), (WaterCannonVector){0.f, 0.f, 0.5f}),')
    # Retain measured MSVC C++ argument order when C evaluates aggregate arguments first
    splash='WaterCannon_BackendParticle(PARTICLE_CAR_SPLASH, WaterCannon_BackendPedPosition(ped), WaterCannon_Add(WaterCannon_Scale(WaterCannon_BackendVelocity(ped), -0.3f), (WaterCannonVector){0.f, 0.f, 0.5f}),\n\t\t\t\t\t\t\t\tGeneral_GetRandomNumberInRangeFloat(0.f, 10.f), General_GetRandomNumberInRangeFloat(0.f, 90.f), 1);'
    assert splash in body
    native='''#if defined(_MSC_VER) && !defined(__clang__)
                            // Preserve native C++ random draws and narrowing before the position query
                            int32_t rotation = (int32_t)General_GetRandomNumberInRangeFloat(0.f, 90.f);
                            int32_t rotationSpeed = (int32_t)General_GetRandomNumberInRangeFloat(0.f, 10.f);
                            WaterCannon_BackendParticle(PARTICLE_CAR_SPLASH, WaterCannon_BackendPedPosition(ped), WaterCannon_Add(WaterCannon_Scale(WaterCannon_BackendVelocity(ped), -0.3f), (WaterCannonVector){0.f, 0.f, 0.5f}), rotationSpeed, rotation, 1);
#else
                            // Retain the original expression order selected by the other C and C++ front ends
                            '''+splash+'''
#endif'''
    body=body.replace(splash,native)
    source=source[:a]+body+source[b:]
    source=source.replace('void CWaterCannons::UpdateOne(uint32 id, CVector *pos, CVector *dir)','static void WaterCannons_Input(uint32_t id, const void *pos, const void *dir, WaterCannonRead read)')
    source=re.sub(r'\bCWaterCannons::(\w+)\(',r'WaterCannons_\1(',source)
    source=re.sub(r'\baCannons\b','WaterCannons_aCannons',source)
    source=source.replace('WaterCannons_aCannons[i].Init()','WaterCannon_Init(&WaterCannons_aCannons[i])').replace('WaterCannons_aCannons[n].Init()','WaterCannon_Init(&WaterCannons_aCannons[n])')
    source=source.replace('WaterCannons_aCannons[n].Update_NewInput(pos, dir)','WaterCannon_Input(&WaterCannons_aCannons[n], pos, dir, read)')
    source=source.replace('WaterCannons_aCannons[i].Update_OncePerFrame(i)','WaterCannon_Update_OncePerFrame(&WaterCannons_aCannons[i], i)').replace('WaterCannons_aCannons[i].Render()','WaterCannon_Render(&WaterCannons_aCannons[i])')
    source=re.sub(r'\bNUM_SEGMENTPOINTS\b','WATERCANNON_SEGMENTS',source)
    source=re.sub(r'\bint32\b','int32_t',source);source=re.sub(r'\bint16\b','int16_t',source)
    source=source.replace('ASSERT(','WC_ASSERT(')
    source=re.sub(r'RwIm3DVertexSet(Pos|RGBA|U|V)\s*\(&WaterCannonVertices\[([^\]]+)\],',r'WaterCannon_BackendSet\1(\2,',source)
    for key,name in [('ZWRITEENABLE','ZWrite'),('VERTEXALPHAENABLE','VertexAlpha'),('FOGENABLE','Fog')]:
        source=re.sub(r'RwRenderStateSet\(rwRENDERSTATE'+key+r',\s*\(void \*\)(TRUE|FALSE)\);',lambda m:'WaterCannon_Backend'+name+'('+('true' if m[1]=='TRUE' else 'false')+');',source)
    source=re.sub(r'RwRenderStateSet\(rwRENDERSTATETEXTURERASTER,\s*\(void \*\)gpWaterRaster\);','WaterCannon_BackendRaster();',source)
    source=source.replace('LittleTest()','WaterCannon_BackendLittleTest()').replace('RwIm3DTransform(WaterCannonVertices, WATERCANNONVERTS, NULL, rwIM3D_VERTEXUV)','WaterCannon_BackendTransform()').replace('RwIm3DRenderIndexedPrimitive(rwPRIMTYPETRILIST, WaterCannonIndexList, WATERCANNONINDEXES)','WaterCannon_BackendRenderIndexed(WaterCannonIndexList)').replace('RwIm3DEnd()','WaterCannon_BackendEnd()')
    source=source.replace('PUSH_RENDERGROUP(','WaterCannon_BackendPush(').replace('POP_RENDERGROUP()','WaterCannon_BackendPop()')
    source=re.sub(r'\t*//[+-] rouz edit \(ChatGPT\)\n','',source)
    comments={'WaterCannon_Init':'Preserve partial state initialization and every shared UV and index write','WaterCannon_Update_OncePerFrame':'Preserve timeout, integration, first fire hit and frame-gated ped updates','WaterCannon_Input':'Assert both pointers before copying position then velocity and marking the slot','WaterCannon_Render':'Preserve camera selection, random behavior and circular ribbon traversal','WaterCannon_PushPeds':'Preserve bounds, descending live pool traversal and first qualifying segment effects','WaterCannons_Init':'Initialize every original cannon in array order','WaterCannons_Input':'Preserve assertions, signed and unsigned ID comparisons and first-free selection','WaterCannons_Update':'Update each nonzero-ID cannon at its original array index','WaterCannons_Render':'Render each nonzero-ID cannon within the original debug group'}
    for name,comment in comments.items():source=re.sub(r'('+name+r'\([^\n]*\)\n\{)',r'\1\n\t// '+comment,source)
    source=re.sub(r'^(\t+)(\{\n)',r'\1\2\1\t// Retain the original traversal and write order\n',source,flags=re.M)
    assert not re.search(r'\b(?:CWaterCannons|CWaterCannon|CVector|CVector2D|RwFrame|RwMatrix|WaterCannonVertices)\b',re.sub(r'"[^"\n]*"','',source))
    return '//+ rouz edit (ChatGPT)\n#include "WaterCannon_backend.h"\n#include "Timer.h"\n#include "General.h"\n#include "AnimationId.h"\n#include "ParticleType.h"\n#include <math.h>\n#include <stddef.h>\n\n'+HELPERS+source.rstrip()+'\n\n'+WRAPPERS+'//- rouz edit (ChatGPT)\n'

def project(source):
    # Compile all C11 algorithms beside typed C++ operations with separate objects
    if '..\\src\\renderer\\WaterCannon.c"' not in source:
        entry='<ClCompile Include="..\\src\\renderer\\WaterCannon.cpp" />';addition='<ClCompile Include="..\\src\\renderer\\WaterCannon.c">\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>'
        source=source.replace(entry,entry+'\n    '+addition)
        entry='<ClCompile Include="..\\src\\renderer\\WaterCannon.cpp">\n      <Filter>renderer</Filter>\n    </ClCompile>';source=source.replace(entry,entry+'\n    '+entry.replace('WaterCannon.cpp','WaterCannon.c'))
    if '..\\src\\renderer\\WaterCannon_backend.h"' not in source:
        for entry in ['<ClInclude Include="..\\src\\renderer\\WaterCannon.h" />','<ClInclude Include="..\\src\\renderer\\WaterCannon.h">\n      <Filter>renderer</Filter>\n    </ClInclude>']:source=source.replace(entry,entry+'\n    '+entry.replace('WaterCannon.h','WaterCannon_backend.h'))
    return point_lights.project(objects.project(source))
