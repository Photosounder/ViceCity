"""Relocate complete bullet-trace algorithms, resources and geometry into C11."""
import re
import object_names_migration as objects
import special_fx_migration as specialfx

HEADER='''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
typedef struct BulletTraceVector { float x,y,z; } BulletTraceVector;
typedef struct BulletTrace {
    BulletTraceVector m_vecStartPos,m_vecEndPos;
    bool m_bInUse;
    uint32_t m_nCreationTime,m_nLifeTime;
    float m_fThickness;
    uint8_t m_fVisibility;
} BulletTrace;
#ifdef __cplusplus
extern "C" {
#endif
extern BulletTrace BulletTraces_aTraces[NUMBULLETTRACES];
void BulletTrace_Update(BulletTrace *trace);
void BulletTraces_Init(void);
void BulletTraces_InitGeometry(void);
void BulletTraces_InitTexture(void);
void BulletTraces_ShutdownTexture(void);
void BulletTraces_Update(void);
void BulletTraces_Render(void);
void BulletTraces_Add(const BulletTraceVector *start,const BulletTraceVector *end,float thickness,uint32_t lifeTime,uint8_t visibility);
void BulletTraces_AddLegacy(const void *start,const void *end,float thickness,uint32_t lifeTime,uint8_t visibility);
void BulletTraces_AddWeapon(const BulletTraceVector *start,const BulletTraceVector *end,int32_t weaponType,void *shooter);
void BulletTraces_AddWeaponLegacy(const void *start,const void *end,int32_t weaponType,void *shooter);
#ifdef __cplusplus
}
class CVector;
BulletTraceVector BulletTraces_FromVector(CVector value);
CVector BulletTraces_ToVector(BulletTraceVector value);
#endif
//- rouz edit (ChatGPT)
'''
READERS=[
('BulletTraceVector','ReadVector','const void *value','const CVector &v=*(const CVector *)value;\n    BulletTraceVector result={v.x,v.y,v.z};return result;','Read actual legacy vector components at their original copy point'),
('const void *','CameraPosition','void','return &TheCamera.GetPosition();','Borrow the original live camera position reference'),
('const void *','CameraForward','void','return &TheCamera.GetForward();','Borrow the original live forward-vector reference'),
('const void *','CameraUp','void','return &TheCamera.GetUp();','Borrow the original live up-vector reference'),
('const void *','CameraRight','void','return &TheCamera.GetRight();','Borrow the original live right-vector reference'),
('void *','PlayerPedEntity','void','return (CEntity *)FindPlayerPed();','Preserve the original typed player-to-entity conversion and query'),
('void *','PlayerVehicle','void','return FindPlayerVehicle();','Preserve each original live player-vehicle query'),
('void *','AsVehicle','void *entity','return (CVehicle *)(CEntity *)entity;','Preserve the original typed entity-to-vehicle address conversion'),
('void *','PlayerVehiclePhysical','void','return (CPhysical *)FindPlayerVehicle();','Preserve the original repeated vehicle query and physical-base conversion'),
('void *','PlayerPedPhysical','void','return (CPhysical *)FindPlayerPed();','Preserve the original repeated ped query and physical-base conversion'),
('BulletTraceVector','MoveSpeed','void *physical','return BulletTraces_BackendReadVector(&((CPhysical *)physical)->m_vecMoveSpeed);','Read the actual physical-owner velocity before C computes speed'),
('int16_t','CameraMode','void','return TheCamera.Cams[TheCamera.ActiveCam].Mode;','Read the original active camera mode only after shooter eligibility'),
('bool','EnvMap','void','#if defined(LIBRW) && defined(EXTENDED_PIPELINES)\n    return CustomPipes::bRenderingEnvMap;\n#else\n    return false;\n#endif','Read the original conditional reflection-pass flag'),
('void *','EnvCamera','void','#if defined(LIBRW) && defined(EXTENDED_PIPELINES)\n    return CustomPipes::EnvMapCam;\n#else\n    return NULL;\n#endif','Read the actual auxiliary camera at every original use'),
('void *','CameraFrame','void *camera','return RwCameraGetFrame((RwCamera *)camera);','Enter the original typed camera-frame operation'),
('void *','FrameMatrix','void *frame','return RwFrameGetMatrix((RwFrame *)frame);','Enter each original repeated frame-matrix query'),
('void *','Raster','void *texture','return RwTextureGetRaster((RwTexture *)texture);','Read the actual smoke-trail raster through its texture owner'),
('void','SetRaster','void *raster','RwRenderStateSet(rwRENDERSTATETEXTURERASTER,raster);','Set the original typed texture-raster state'),
('void','ZWrite','bool value','RwRenderStateSet(rwRENDERSTATEZWRITEENABLE,(void *)(uintptr_t)value);','Set the original typed depth-write state'),
('void','SrcBlend','void','RwRenderStateSet(rwRENDERSTATESRCBLEND,(void *)rwBLENDSRCALPHA);','Set the original source blend factor'),
('void','DestBlend','void','RwRenderStateSet(rwRENDERSTATEDESTBLEND,(void *)rwBLENDINVSRCALPHA);','Set the original destination blend factor'),
('bool','Transform','bool uv','return RwIm3DTransform(TraceVertices,ARRAY_SIZE(TraceVertices),nil,uv?rwIM3D_VERTEXUV:1)!=nil;','Transform the actual shared ten-vertex array with the original flags'),
('void','RenderIndexed','uint16_t *indices','RwIm3DRenderIndexedPrimitive(rwPRIMTYPETRILIST,indices,48);','Render the entire original typed triangle table'),
('void','End','void','RwIm3DEnd();','Finish the original successful immediate draw'),
('void','PushTxd','void','CTxdStore::PushCurrentTxd();','Preserve the original texture-dictionary push'),
('int32_t','FindTxd','const char *name','return CTxdStore::FindTxdSlot(name);','Find the original named texture dictionary'),
('void','SetTxd','int32_t slot','CTxdStore::SetCurrentTxd(slot);','Select the original texture dictionary'),
('void *','ReadTexture','const char *name','return RwTextureRead(name,0);','Read the original named smoke texture with its null mask'),
('void','PopTxd','void','CTxdStore::PopCurrentTxd();','Restore the original texture dictionary'),
('void','DestroyTexture','void *texture','RwTextureDestroy((RwTexture *)texture);','Destroy the original typed texture before C clears its published handle')]
for name,field in [('Position','pos'),('Forward','at')]:READERS.append(('BulletTraceVector','Matrix'+name,'void *matrix','const RwV3d &v=((RwMatrix *)matrix)->'+field+';\n    BulletTraceVector result={v.x,v.y,v.z};return result;','Read actual matrix '+field+' components'))
for name,args,values in [('U','float value','value'),('V','float value','value'),('RGBA','int red,int green,int blue,int alpha','red,green,blue,alpha'),('Pos','float x,float y,float z','x,y,z')]:READERS.append(('void','Set'+name,'int index,'+args,'RwIm3DVertexSet'+name+'(&TraceVertices[index],'+values+');','Write actual shared vertex '+name+' fields'))
BACKEND_HEADER='//+ rouz edit (ChatGPT)\n#pragma once\n#include "BulletTraces.h"\n#ifdef __cplusplus\nextern "C" {\n#endif\n'+''.join(result+' BulletTraces_Backend'+name+'('+args+');\n' for result,name,args,body,comment in READERS)+'#ifdef __cplusplus\n}\n#endif\n//- rouz edit (ChatGPT)\n'
HOST='''//+ rouz edit (ChatGPT)
#include "common.h"
#include "Camera.h"
#include "World.h"
#include "PlayerPed.h"
#include "Vehicle.h"
#include "TxdStore.h"
#include "BulletTraces_backend.h"
#if defined(LIBRW) && defined(EXTENDED_PIPELINES)
#include "custompipes.h"
#endif
static RwIm3DVertex TraceVertices[10];
'''+''.join(result+' BulletTraces_Backend'+name+'('+args+')\n{\n    // '+comment+'\n    '+body+'\n}\n' for result,name,args,body,comment in READERS)+'''
BulletTraceVector BulletTraces_FromVector(CVector value)
{
    // Copy replay input coordinates through their actual typed owner
    BulletTraceVector result={value.x,value.y,value.z};return result;
}
CVector BulletTraces_ToVector(BulletTraceVector value)
{
    // Return a typed replay packet value without aliasing a C record
    return CVector(value.x,value.y,value.z);
}
//- rouz edit (ChatGPT)
'''
HELPERS='''#if defined(_MSC_VER)
float fast_absf(float value);
#define Abs fast_absf
#else
#define Abs fabsf
#endif
#define Sqrt sqrtf
#define SQR(a) ((a)*(a))
typedef BulletTraceVector (*BulletTracesRead)(const void *);
typedef const void *(*BulletTracesCamera)(void);
static BulletTraceVector BulletTraces_Read(const void *value)
{
    // Read actual C vector storage at the original live input use
    return *(const BulletTraceVector *)value;
}
static BulletTraceVector BulletTraces_AddVector(BulletTraceVector a,BulletTraceVector b)
{
    // Retain the original component addition order
    BulletTraceVector result={a.x+b.x,a.y+b.y,a.z+b.z};return result;
}
static BulletTraceVector BulletTraces_Subtract(BulletTraceVector a,BulletTraceVector b)
{
    // Retain the original component subtraction order
    BulletTraceVector result={a.x-b.x,a.y-b.y,a.z-b.z};return result;
}
static BulletTraceVector BulletTraces_LeftScale(float a,BulletTraceVector b)
{
    // Retain original scalar-times-vector operand order
    BulletTraceVector result={a*b.x,a*b.y,a*b.z};return result;
}
static BulletTraceVector BulletTraces_Divide(BulletTraceVector a,float b)
{
    // Retain the original three separate divisions
    BulletTraceVector result={a.x/b,a.y/b,a.z/b};return result;
}
static float BulletTraces_Dot(BulletTraceVector a,BulletTraceVector b)
{
    // Retain the original dot products and additions
    return a.x*b.x+a.y*b.y+a.z*b.z;
}
static float BulletTraces_Magnitude(BulletTraceVector a)
{
    // Retain the original float square-root wrapper
    return Sqrt(a.x*a.x+a.y*a.y+a.z*a.z);
}
static void BulletTraces_Normalise(BulletTraceVector *v)
{
    // Retain the original float reciprocal root and x-only fallback
    float square=v->x*v->x+v->y*v->y+v->z*v->z;
    if(square>0.0f){
        // Scale each original component in order
        float scale=1.0f/Sqrt(square);v->x*=scale;v->y*=scale;v->z*=scale;
    }else v->x=1.0f;
}
static float BulletTraces_Project(const void *input,BulletTracesRead read,BulletTracesCamera axis)
{
    // Preserve borrowed camera references and the compiler-selected original argument order
#if defined(__clang__)
    const void *basis=axis();
#endif
    const void *position=BulletTraces_BackendCameraPosition();
    BulletTraceVector delta=BulletTraces_Subtract(read(input),BulletTraces_BackendReadVector(position));
#if !defined(__clang__)
    const void *basis=axis();
#endif
    return BulletTraces_Dot(BulletTraces_BackendReadVector(basis),delta);
}
'''

def section(source):
    # Delimit all six methods without altering neighboring marker algorithms
    a=source.index('CBulletTrace CBulletTraces::aTraces[NUMBULLETTRACES];');b=source.index('RpAtomic *\nMarkerAtomicCB(',a);return source[a:b]

def geometry(source):
    # Preserve all fifteen original UV writes including the five uninitialized V fields
    a=source.index('\tRwIm3DVertexSetU(&TraceVertices[0]');b=source.index('\n\t// Initialize streak geometry',a);return source[a:b]

def resource_init(source):
    # Extract the complete original resource setup without dropping dictionary operations
    a=source.index('\tCTxdStore::PushCurrentTxd();');b=source.index('\tCTxdStore::PopCurrentTxd();',a)+len('\tCTxdStore::PopCurrentTxd();');return source[a:b]

def resource_shutdown(source):
    # Extract the entire original conditional destruction block
    a=source.index('\tif (gpSmokeTrailTexture)',source.index('CSpecialFX::Shutdown'));b=source.index('\n\t}',a)+len('\n\t}');return source[a:b]

def camera_enum(source):
    # Extract the complete original ordered camera-mode enum
    a=source.index('\tenum\n',source.index('class CCam\n'));b=source.index('\n\t};',a)+len('\n\t};');return source[a:b]

def camera_modes(source):
    # Make one plain C definition authoritative for all original mode values
    block=camera_enum(source).replace('\tenum\n','enum CameraMode\n',1);block=re.sub(r'\bMODE_', 'CAMERA_MODE_',block)
    return '//+ rouz edit (ChatGPT)\n#pragma once\n'+block+'\n//- rouz edit (ChatGPT)\n'

def camera_header(source):
    # Retain scoped compatibility names as aliases to the shared C constants
    block=camera_enum(source);names=re.findall(r'\bMODE_\w+',block)
    aliases='\tenum\n\t{\n'+''.join('\t\t'+name+' = CAMERA_'+name+',\n' for name in names)+'\t};'
    return source.replace('#pragma once','#pragma once\n#include "CameraModes.h"',1).replace(block,aliases)

def calls(source):
    # Replace the owners, both resolved overloads and explicit replay value boundaries
    if 'class CCam\n' in source and 'MODE_NONE = 0,' in source:source=camera_header(source)
    if 'CBulletTrace' not in source and 'CBulletTraces::' not in source:return specialfx.calls(source)
    if 'struct CBulletTrace\n' in source:
        a=source.index('struct CBulletTrace\n');b=source.index('\nenum\n',a);source=source[:a]+'#include "BulletTraces.h"\n'+source[b:]
    if 'CBulletTrace CBulletTraces::aTraces[NUMBULLETTRACES];' in source:
        setup=geometry(source);source=source.replace(section(source),'').replace(setup,'\t// Initialize trace UV fields at their original shared-effect setup point\n\tBulletTraces_InitGeometry();\n')
        source=source.replace('RwIm3DVertex TraceVertices[10];\n','');source=re.sub(r'static RwImVertexIndex TraceIndexList\[48\] = \{.*?\};\n','',source,flags=re.S)
        source=source.replace('static RwTexture* gpSmokeTrailTexture;\n','')
        source=source.replace(resource_init(source),'\t// Acquire the trace texture at its original point in shared effect initialization\n\tBulletTraces_InitTexture();')
        source=source.replace(resource_shutdown(source),'\t// Release the trace texture at its original shutdown point\n\tBulletTraces_ShutdownTexture();')
        assert 'gpSmokeTrailTexture' not in source
    source=source.replace('CBulletTraces::AddTrace(','BulletTraces_AddWeaponLegacy(').replace('CBulletTraces::aTraces','BulletTraces_aTraces').replace('CBulletTraces::','BulletTraces_')
    for field,packet in [('m_vecStartPos','inf'),('m_vecEndPos','sup')]:
        source=source.replace('bt->'+packet+' = BulletTraces_aTraces[i].'+field+';','bt->'+packet+' = BulletTraces_ToVector(BulletTraces_aTraces[i].'+field+');')
        source=source.replace('BulletTraces_aTraces[pb->index].'+field+' = pb->'+packet+';','BulletTraces_aTraces[pb->index].'+field+' = BulletTraces_FromVector(pb->'+packet+');')
    lines=[]
    for line in source.splitlines(keepends=True):
        if re.search(r'\bBulletTraces_(?:Init|Update|Render|AddWeaponLegacy)\(',line):lines.append(re.match(r'\s*',line)[0]+'// Enter the C bullet-trace algorithm with the original owner expressions\n')
        if 'BulletTraces_ToVector(' in line or ' = BulletTraces_FromVector(' in line:lines.append(re.match(r'\s*',line)[0]+'// Copy replay coordinates through the explicit typed vector boundary\n')
        lines.append(line)
    return specialfx.calls(''.join(lines))

def kernel(source):
    # Relocate every method, resource decision, vector operation and shared geometry write
    body=section(source);table=re.search(r'static RwImVertexIndex TraceIndexList\[48\] = \{.*?\};',source,re.S)[0].replace('RwImVertexIndex','uint16_t')
    body=body.replace('CBulletTrace CBulletTraces::aTraces[NUMBULLETTRACES];','BulletTrace BulletTraces_aTraces[NUMBULLETTRACES];')
    body=body.replace('void CBulletTraces::AddTrace(CVector* start, CVector* end, float thickness, uint32 lifeTime, uint8 visibility)','static void BulletTraces_AddInput(const void *start,const void *end,float thickness,uint32_t lifeTime,uint8_t visibility,BulletTracesRead read)')
    body=body.replace('void CBulletTraces::AddTrace(CVector* start, CVector* end, int32 weaponType, class CEntity* shooter)','static void BulletTraces_AddWeaponInput(const void *start,const void *end,int32_t weaponType,void *shooter,BulletTracesRead read)')
    body=body.replace('void CBulletTrace::Update(void)','void BulletTrace_Update(BulletTrace *trace)')
    a=body.index('void BulletTrace_Update');body=body[:a]+re.sub(r'\b(m_nCreationTime|m_nLifeTime|m_bInUse)\b',r'trace->\1',body[a:])
    for expression in ['0.7f, 1000, 200','1.0f, 2000, 220','0.4f, 750, 150']:body=body.replace('CBulletTraces::AddTrace(start, end, '+expression+');','BulletTraces_AddInput(start,end,'+expression+',read);')
    body=body.replace('CBulletTraces::','BulletTraces_');body=re.sub(r'\baTraces\b','BulletTraces_aTraces',body)
    body=body.replace('BulletTraces_aTraces[i].Update()','BulletTrace_Update(&BulletTraces_aTraces[i])')
    for axis in ['Forward','Up','Right']:
        for pointer in ['start','end']:body=body.replace('DotProduct(TheCamera.Get'+axis+'(), *'+pointer+' - TheCamera.GetPosition())','BulletTraces_Project('+pointer+',read,BulletTraces_BackendCamera'+axis+')')
    body=body.replace('= *start;','= read(start);').replace('= *end;','= read(end);')
    body=body.replace('(CEntity*)FindPlayerPed()','BulletTraces_BackendPlayerPedEntity()').replace('(CVehicle*)shooter','BulletTraces_BackendAsVehicle(shooter)')
    body=body.replace('(CPhysical*)FindPlayerVehicle()','BulletTraces_BackendPlayerVehiclePhysical()').replace('(CPhysical*)FindPlayerPed()','BulletTraces_BackendPlayerPedPhysical()').replace('FindPlayerVehicle()','BulletTraces_BackendPlayerVehicle()')
    body=body.replace('CPhysical* player;','void *player;').replace('TheCamera.Cams[TheCamera.ActiveCam].Mode','BulletTraces_BackendCameraMode()').replace('CCam::MODE_','CAMERA_MODE_').replace('player->m_vecMoveSpeed.Magnitude()','BulletTraces_Magnitude(BulletTraces_BackendMoveSpeed(player))')
    body=body.replace('TheCamera.GetPosition()','BulletTraces_BackendReadVector(BulletTraces_BackendCameraPosition())').replace('TheCamera.GetForward()','BulletTraces_BackendReadVector(BulletTraces_BackendCameraForward())')
    body=body.replace('CustomPipes::bRenderingEnvMap','BulletTraces_BackendEnvMap()').replace('(RwCamera*)CustomPipes::EnvMapCam','BulletTraces_BackendEnvCamera()').replace('CustomPipes::EnvMapCam','BulletTraces_BackendEnvCamera()')
    body=body.replace('RwCamera *effectCamera','void *effectCamera').replace('RwFrame *effectFrame','void *effectFrame').replace('RwMatrix *effectMatrix','void *effectMatrix')
    body=body.replace('RwCameraGetFrame(effectCamera)','BulletTraces_BackendCameraFrame(effectCamera)').replace('RwFrameGetMatrix(effectFrame)','BulletTraces_BackendFrameMatrix(effectFrame)')
    body=body.replace('CVector(effectMatrix->pos)','BulletTraces_BackendMatrixPosition(effectMatrix)').replace('CVector(effectMatrix->at)','BulletTraces_BackendMatrixForward(effectMatrix)')
    body=body.replace('traceCameraForward.Normalise();','BulletTraces_Normalise(&traceCameraForward);')
    body=body.replace('CVector horizontalOffset = BulletTraces_aTraces[i].m_vecEndPos - BulletTraces_aTraces[i].m_vecStartPos;','BulletTraceVector horizontalOffset = BulletTraces_Subtract(BulletTraces_aTraces[i].m_vecEndPos,BulletTraces_aTraces[i].m_vecStartPos);')
    body=body.replace('horizontalOffset.Normalise();','BulletTraces_Normalise(&horizontalOffset);').replace('horizontalOffset *= traceThickness;','horizontalOffset.x *= traceThickness;horizontalOffset.y *= traceThickness;horizontalOffset.z *= traceThickness;')
    body=body.replace('DotProduct(start - traceCameraPosition, traceCameraForward)','BulletTraces_Dot(BulletTraces_Subtract(start,traceCameraPosition),traceCameraForward)').replace('DotProduct(end - traceCameraPosition, traceCameraForward)','BulletTraces_Dot(BulletTraces_Subtract(end,traceCameraPosition),traceCameraForward)')
    body=body.replace('(absEndProj * start + absStartProj * end) / (absStartProj + absEndProj)','BulletTraces_Divide(BulletTraces_AddVector(BulletTraces_LeftScale(absEndProj,start),BulletTraces_LeftScale(absStartProj,end)),(absStartProj + absEndProj))')
    body=body.replace('(7.0f * start + end) / 8','BulletTraces_Divide(BulletTraces_AddVector(BulletTraces_LeftScale(7.0f,start),end),8)').replace('(7.0f * end + start) / 8','BulletTraces_Divide(BulletTraces_AddVector(BulletTraces_LeftScale(7.0f,end),start),8)')
    body=body.replace('CVector ','BulletTraceVector ')
    body=body.replace('RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)FALSE)','BulletTraces_BackendZWrite(false)').replace('RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)TRUE)','BulletTraces_BackendZWrite(true)')
    body=body.replace('RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA)','BulletTraces_BackendSrcBlend()').replace('RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA)','BulletTraces_BackendDestBlend()').replace('RwRenderStateSet(rwRENDERSTATETEXTURERASTER, RwTextureGetRaster(gpSmokeTrailTexture))','BulletTraces_BackendSetRaster(BulletTraces_BackendRaster(gpSmokeTrailTexture))')
    body=body.replace('RwIm3DTransform(TraceVertices, ARRAY_SIZE(TraceVertices), nil, 1)','BulletTraces_BackendTransform(false)').replace('RwIm3DTransform(TraceVertices, ARRAY_SIZE(TraceVertices), nil, rwIM3D_VERTEXUV)','BulletTraces_BackendTransform(true)').replace('RwIm3DRenderIndexedPrimitive(rwPRIMTYPETRILIST, TraceIndexList, ARRAY_SIZE(TraceIndexList))','BulletTraces_BackendRenderIndexed(TraceIndexList)').replace('RwIm3DEnd()','BulletTraces_BackendEnd()')
    for a,b in [('int32','int32_t'),('int16','int16_t'),('uint32','uint32_t'),('uint8','uint8_t')]:body=re.sub(r'\b'+a+r'\b',b,body)
    body=body.replace('nil','NULL')
    setup=geometry(source)
    wrappers=''
    for name,args,invoke in [('Add','float thickness,uint32_t lifeTime,uint8_t visibility','thickness,lifeTime,visibility'),('AddWeapon','int32_t weaponType,void *shooter','weaponType,shooter')]:
        for suffix,kind,reader in [('','const BulletTraceVector *','BulletTraces_Read'),('Legacy','const void *','BulletTraces_BackendReadVector')]:
            wrappers+='void BulletTraces_'+name+suffix+'('+kind+'start,'+kind+'end,'+args+')\n{\n    // Preserve all original lazy input reads and overload-specific eligibility\n    BulletTraces_'+name+'Input(start,end,'+invoke+','+reader+');\n}\n'
    resources='static void *gpSmokeTrailTexture;\nvoid BulletTraces_InitTexture(void)\n{\n'+resource_init(source)+'\n}\nvoid BulletTraces_ShutdownTexture(void)\n{\n'+resource_shutdown(source)+'\n}\n'
    resources=resources.replace('CTxdStore::PushCurrentTxd()','BulletTraces_BackendPushTxd()').replace('CTxdStore::SetCurrentTxd(','BulletTraces_BackendSetTxd(').replace('CTxdStore::FindTxdSlot(','BulletTraces_BackendFindTxd(').replace('CTxdStore::PopCurrentTxd()','BulletTraces_BackendPopTxd()').replace('RwTextureRead("smoketrail", 0)','BulletTraces_BackendReadTexture("smoketrail")').replace('RwTextureDestroy(gpSmokeTrailTexture)','BulletTraces_BackendDestroyTexture(gpSmokeTrailTexture)').replace('nil','NULL')
    combined=table+'\n\n'+resources+'\nvoid BulletTraces_InitGeometry(void)\n{\n'+setup+'}\n\n'+body
    combined=re.sub(r'RwIm3DVertexSet(U|V|RGBA|Pos)\(&TraceVertices\[([^\]]+)\],',r'BulletTraces_BackendSet\1(\2,',combined)
    combined=combined.replace('\n{\n','\n{\n\t// Preserve the complete original trace, clipping and shared-geometry operation order\n')
    combined=re.sub(r'^(\t+)([^\n]*\{[^\n]*\n)',r'\1\2\1\t// Retain original eligibility, arithmetic and write order\n',combined,flags=re.M)
    assert not re.search(r'\b(?:CBulletTrace|CBulletTraces|CVector|CPhysical|CTxdStore|RwIm3DVertex|DotProduct|TheCamera|CustomPipes)\b',combined)
    return '//+ rouz edit (ChatGPT)\n#include "BulletTraces_backend.h"\n#include "Timer.h"\n#include "DMAudio.h"\n#include "WeaponType.h"\n#include "CameraModes.h"\n#include <math.h>\n#include <stddef.h>\n\n'+HELPERS+combined.rstrip()+'\n\n'+wrappers+'//- rouz edit (ChatGPT)\n'

def project(source):
    # Register complete C11 logic, typed backend and authoritative camera constants
    if '..\\src\\renderer\\BulletTraces.c"' not in source:
        entry='<ClCompile Include="..\\src\\renderer\\SpecialFX.cpp" />'
        source=source.replace(entry,entry+'\n    <ClCompile Include="..\\src\\renderer\\BulletTraces.cpp" />\n    <ClCompile Include="..\\src\\renderer\\BulletTraces.c">\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>')
        entry='<ClCompile Include="..\\src\\renderer\\SpecialFX.cpp">\n      <Filter>renderer</Filter>\n    </ClCompile>';source=source.replace(entry,entry+'\n    '+entry.replace('SpecialFX.cpp','BulletTraces.cpp')+'\n    '+entry.replace('SpecialFX.cpp','BulletTraces.c'))
    for name in ['BulletTraces.h','BulletTraces_backend.h']:
        if '..\\src\\renderer\\'+name+'"' in source:continue
        for entry in ['<ClInclude Include="..\\src\\renderer\\SpecialFX.h" />','<ClInclude Include="..\\src\\renderer\\SpecialFX.h">\n      <Filter>renderer</Filter>\n    </ClInclude>']:source=source.replace(entry,entry+'\n    '+entry.replace('SpecialFX.h',name))
    if '..\\src\\core\\CameraModes.h"' not in source:
        for entry in ['<ClInclude Include="..\\src\\core\\Camera.h" />','<ClInclude Include="..\\src\\core\\Camera.h">\n      <Filter>core</Filter>\n    </ClInclude>']:source=source.replace(entry,entry+'\n    '+entry.replace('Camera.h','CameraModes.h'))
    return specialfx.project(source)
