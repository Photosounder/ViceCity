"""Move the complete 3D-marker controller to C while retaining typed marker/matrix ownership."""
import re
from audio_controls_migration import function
import marker_record_migration as records
ENUM='''enum { MARKERTYPE_0=0,MARKERTYPE_ARROW,MARKERTYPE_2,MARKERTYPE_3,MARKERTYPE_CYLINDER,NUMMARKERTYPES,MARKERTYPE_INVALID=0x101 };
'''
ARGS='uint32_t identifier,uint16_t type,void *pos,float size,uint8_t r,uint8_t g,uint8_t b,uint8_t a,uint16_t pulsePeriod,float pulseFraction,int16_t rotateRate'
INVOKE='identifier,type,pos,size,r,g,b,a,pulsePeriod,pulseFraction,rotateRate'
HEADER='''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
typedef struct Markers3DVector { float x,y,z; } Markers3DVector;
typedef struct Markers3DVector2D { float x,y; } Markers3DVector2D;
'''+ENUM+'''#ifdef __cplusplus
extern "C" {
#endif
extern int32_t Markers3D_NumActiveMarkers;
void Markers3D_Init(void);
void Markers3D_Shutdown(void);
void Markers3D_Render(void);
void Markers3D_RenderForEnvMap(void *camera);
void Markers3D_Update(void);
void *Markers3D_PlaceMarker('''+ARGS.replace('void *pos','Markers3DVector *pos')+''');
void Markers3D_PlaceMarkerSet('''+ARGS.replace('void *pos','Markers3DVector *pos')+''');
#ifdef __cplusplus
}
class C3dMarker;
class CVector;
C3dMarker *Markers3D_PlaceMarkerLegacy('''+ARGS.replace('void *pos','CVector &pos')+''');
void Markers3D_PlaceMarkerSetLegacy('''+ARGS.replace('void *pos','CVector &pos')+''');
#endif
//- rouz edit (ChatGPT)
'''
FIELDS=[('uint16_t','m_nType'),('bool','m_bIsUsed'),('bool','m_bFindZOnNextPlacement'),('uint32_t','m_nIdentifier'),('uint8_t','red'),('uint8_t','green'),('uint8_t','blue'),('uint8_t','alpha'),('uint16_t','m_nPulsePeriod'),('int16_t','m_nRotateRate'),('uint32_t','m_nStartTime'),('float','m_fPulseFraction'),('float','m_fStdSize'),('float','m_fSize'),('float','m_fBrightness'),('float','m_fCameraRange')]
FIELD_HEADER='typedef struct Markers3DFields {\n'+''.join('    '+kind+' *'+name+';\n' for kind,name in FIELDS)+'} Markers3DFields;\n'
READERS=[
('void *','Marker','int32_t index','return &MarkerOwners[index];','Address the actual typed marker pool without assuming its stride'),
('Markers3DFields','Fields','void *marker','C3dMarker &m=*(C3dMarker *)marker;\n    Markers3DFields result={'+','.join('&m.'+('m_Color.'+n if n in ['red','green','blue','alpha'] else n) for t,n in FIELDS)+'};return result;','Borrow correctly typed scalar field addresses without copying live values'),
('void *','Atomic','void *marker','return ((C3dMarker *)marker)->m_pAtomic;','Read the actual typed atomic pointer at its original use'),
('void','SetAtomic','void *marker,void *atomic','((C3dMarker *)marker)->m_pAtomic=(RpAtomic *)atomic;','Write the actual atomic field without a pointer-to-pointer cast'),
('void *','Clump','int32_t index','return MarkerClumps[index];','Read each original live typed resource slot'),
('void','SetClump','int32_t index,void *clump','MarkerClumps[index]=(RpClump *)clump;','Publish a resource through its actual typed pointer slot'),
('void','DestroyClump','void *clump','RpClumpDestroy((RpClump *)clump);','Destroy the original typed clump without clearing its published slot'),
('void','Delete','void *marker','((C3dMarker *)marker)->DeleteMarkerObject();','Enter the original marker deletion and matrix ownership boundary'),
('void','Render','void *marker','((C3dMarker *)marker)->Render();','Enter the original typed marker-render operation'),
('bool','Add','void *marker,uint32_t identifier,uint16_t type,float size,uint8_t r,uint8_t g,uint8_t b,uint8_t a,uint16_t pulsePeriod,float pulseFraction,int16_t rotateRate','return ((C3dMarker *)marker)->AddMarker(identifier,type,size,r,g,b,a,pulsePeriod,pulseFraction,rotateRate);','Enter the original typed marker creation without adding failure handling'),
('Markers3DVector2D','Player','void','CVector2D value=FindPlayerCentreOfWorld(0);\n    Markers3DVector2D result={value.x,value.y};return result;','Retain the original player query and two-component conversion'),
('bool','Collision','Markers3DVector2D pos','return CColStore::HasCollisionLoaded(CVector2D(pos.x,pos.y));','Enter the exact original collision-loaded predicate'),
('float','Ground','float x,float y,float z','return CWorld::FindGroundZFor3DCoord(x,y,z,nil);','Retain the original ground query and null found pointer'),
('float','ReadLegacy','const void *pos,int32_t axis','const CVector &v=*(const CVector *)pos;\n    return axis==0?v.x:axis==1?v.y:v.z;','Read only the original live legacy coordinate component'),
('void','WriteLegacy','void *pos,int32_t axis,float value','CVector &v=*(CVector *)pos;\n    if(axis==0)v.x=value;else if(axis==1)v.y=value;else v.z=value;','Write the original borrowed vector component through its actual type'),
('Markers3DVector','Position','void *marker','const CVector &v=((C3dMarker *)marker)->m_Matrix.GetPosition();\n    Markers3DVector result={v.x,v.y,v.z};return result;','Read actual marker matrix-position fields at their original use'),
('void','SetPosition','void *marker,Markers3DVector pos','((C3dMarker *)marker)->m_Matrix.GetPosition()=CVector(pos.x,pos.y,pos.z);','Retain the original typed matrix-position assignment'),
('void','SetZ','void *marker,float z','((C3dMarker *)marker)->m_Matrix.GetPosition().z=z;','Write only the original live matrix-height component'),
('void','SetTranslate','void *marker,Markers3DVector pos','((C3dMarker *)marker)->m_Matrix.SetTranslate(pos.x,pos.y,pos.z);','Enter the existing typed matrix translation operation'),
('void','RotateZ','void *marker,float angle','((C3dMarker *)marker)->m_Matrix.RotateZ(angle);','Retain the original typed Z rotation'),
('void','RotateX','void *marker,float angle','((C3dMarker *)marker)->m_Matrix.RotateX(angle);','Retain the original typed X rotation'),
('void','UpdateRW','void *marker','((C3dMarker *)marker)->m_Matrix.UpdateRW();','Publish the original matrix to its attached RenderWare owner'),
('void','Directional','void','ActivateDirectional();','Activate the original directional lighting at the same call point'),
('float','LOD','void','return TheCamera.LODDistMultiplier;','Read the original live camera distance multiplier'),
('void','Corona','void *marker,float size,float drawDist','CCoronas::RegisterCorona((uintptr)marker,SPHERE_MARKER_R,SPHERE_MARKER_G,SPHERE_MARKER_B,192,((C3dMarker *)marker)->m_Matrix.GetPosition(),size,drawDist,CCoronas::TYPE_STAR,CCoronas::FLARE_NONE,CCoronas::REFLECTION_OFF,CCoronas::LOSCHECK_OFF,CCoronas::STREAK_ON,0.0f,false);','Retain the exact corona overload, borrowed matrix position, ID narrowing and defaults'),
('int32_t','FindTxd','const char *name','return CTxdStore::FindTxdSlot(name);','Find the original particle texture dictionary before pushing it'),
('void','PushTxd','void','CTxdStore::PushCurrentTxd();','Push the original current dictionary'),
('void','SetTxd','int32_t slot','CTxdStore::SetCurrentTxd(slot);','Set the original selected dictionary'),
('void','PopTxd','void','CTxdStore::PopCurrentTxd();','Restore the original dictionary after both resource loads'),
('void *','Load','const char *name','return CFileLoader::LoadAtomicFile2Return(name);','Enter the original typed atomic-file loader'),
('void *','CameraFrame','void *camera','return RwCameraGetFrame((RwCamera *)camera);','Read the original reflection camera frame'),
('void *','FrameMatrix','void *frame','return RwFrameGetMatrix((RwFrame *)frame);','Retain every original matrix null check and subsequent matrix read'),
('float','NearClip','void *camera','return RwCameraGetNearClipPlane((RwCamera *)camera);','Read the original live near clip plane'),
('float','FarClip','void *camera','return RwCameraGetFarClipPlane((RwCamera *)camera);','Read the original live far clip plane')]
for name,field in [('MatrixPosition','pos'),('MatrixForward','at')]:READERS.append(('Markers3DVector',name,'void *matrix','const RwV3d &v=((RwMatrix *)matrix)->'+field+';\n    Markers3DVector result={v.x,v.y,v.z};return result;','Copy actual camera matrix '+field+' components'))
BACKEND_HEADER='//+ rouz edit (ChatGPT)\n#pragma once\n#include "Markers3D.h"\n'+FIELD_HEADER+'#ifdef __cplusplus\nextern "C" {\n#endif\n'+''.join(result+' Markers3D_Backend'+name+'('+args+');\n' for result,name,args,body,comment in READERS)+''.join(result+' Markers3D_'+name+'LegacyInput('+ARGS+');\n' for name,result in [('PlaceMarker','void *'),('PlaceMarkerSet','void')])+'#ifdef __cplusplus\n}\n#endif\n//- rouz edit (ChatGPT)\n'
HOST='''//+ rouz edit (ChatGPT)
#include "common.h"
#include "SpecialFX.h"
#include "Markers3D_backend.h"
#include "World.h"
#include "ColStore.h"
#include "Camera.h"
#include "TxdStore.h"
#include "FileLoader.h"
#include "Lights.h"
#include "Coronas.h"
#include "Script.h"
static C3dMarker MarkerOwners[NUM3DMARKERS];
static RpClump *MarkerClumps[NUMMARKERTYPES];
'''+''.join(result+' Markers3D_Backend'+name+'('+args+')\n{\n    // '+comment+'\n    '+body+'\n}\n' for result,name,args,body,comment in READERS)+''.join(result+' Markers3D_'+name+'Legacy('+ARGS.replace('void *pos','CVector &pos')+')\n{\n    // Retain the original mutable CVector reference across all placement callbacks\n    '+('return (C3dMarker *)' if result!='void' else '')+'Markers3D_'+name+'LegacyInput('+INVOKE.replace('type,pos,','type,&pos,')+');\n}\n' for name,result in [('PlaceMarker','C3dMarker *'),('PlaceMarkerSet','void')])+'//- rouz edit (ChatGPT)\n'

def section(source):
    # Delimit the seven complete controller methods and its original global declarations
    a=source.index('C3dMarker C3dMarkers::m_aMarkerArray[NUM3DMARKERS];');b=source.index('void\nC3dMarkers::Update()',a)
    b=source.index('\n}',b)+2;return source[a:b]+'\n'

def marker_header(source):
    # Retain the exact existing record and CMatrix owner declarations
    a=re.search(r'(?:class|struct) C3dMarker\n',source).start();b=source.index('\n};',a)+3;return source[a:b]

def controller_header(source):
    # Preserve the whole original controller declaration for the independent oracle
    a=source.index('class C3dMarkers\n');b=source.index('\n};',a)+3;return source[a:b]

def calls(source):
    # Remove only the controller class and route all original call points to the C API
    if 'C3dMarker C3dMarkers::m_aMarkerArray[NUM3DMARKERS];' in source:
        source=source.replace(section(source),'')
        source=source.replace('RpClumpForAllAtomics(C3dMarkers::m_pRpClumpArray[type],','RpClumpForAllAtomics((RpClump *)Markers3D_BackendClump(type),')
        source=source.replace('#include "SpecialFX.h"','#include "SpecialFX.h"\n#include "Markers3D_backend.h"',1)
    if 'class C3dMarkers\n' in source:
        source=source.replace(controller_header(source),'')
        a=source.index('enum\n{\n\tMARKERTYPE_0');b=source.index('\n};',a)+3;source=source[:a]+'#include "Markers3D.h"'+source[b:]
    for name in ['Init','Render','Shutdown']:
        if 'SpecialFX_Backend3dMarkers'+name+'(void)\n{' in source:source=source.replace(function(source,'SpecialFX_Backend3dMarkers',name)+'\n','')
        source=re.sub(r'^void SpecialFX_Backend3dMarkers'+name+r'\(void\);\n','',source,flags=re.M)
        if 'SpecialFX_Backend3dMarkers'+name+'(' in source:
            source=source.replace('SpecialFX_Backend3dMarkers'+name+'(','Markers3D_'+name+'(')
            if '#include "Markers3D.h"' not in source:source=source.replace('#include "SpecialFX_backend.h"','#include "SpecialFX_backend.h"\n#include "Markers3D.h"')
    for name in ['PlaceMarkerSet','PlaceMarker']:source=source.replace('C3dMarkers::'+name+'(','Markers3D_'+name+'Legacy(')
    source=source.replace('C3dMarkers::','Markers3D_')
    lines=[]
    for line in source.splitlines(keepends=True):
        if re.search(r'\bMarkers3D_(?:Init|Render|Shutdown|RenderForEnvMap|PlaceMarkerLegacy|PlaceMarkerSetLegacy)\(',line):lines.append(re.match(r'\s*',line)[0]+'// Enter the C 3D-marker controller with the original arguments\n')
        lines.append(line)
    return records.calls(''.join(lines))

HELPERS='''typedef float (*Markers3DRead)(const void *,int32_t);
typedef void (*Markers3DWrite)(void *,int32_t,float);
static float Markers3D_Read(const void *pos,int32_t axis)
{
    // Read only the selected original C coordinate
    const Markers3DVector *v=(const Markers3DVector *)pos;return axis==0?v->x:axis==1?v->y:v->z;
}
static void Markers3D_Write(void *pos,int32_t axis,float value)
{
    // Write the selected original borrowed C coordinate
    Markers3DVector *v=(Markers3DVector *)pos;if(axis==0)v->x=value;else if(axis==1)v->y=value;else v->z=value;
}
static Markers3DVector Markers3D_ReadPosition(const void *pos,Markers3DRead read)
{
    // Retain live component reads at each original whole-position use
    Markers3DVector v={read(pos,0),read(pos,1),read(pos,2)};return v;
}
static Markers3DVector2D Markers3D_ReadPosition2D(const void *pos,Markers3DRead read)
{
    // Avoid reading the height where the original conversion used only x and y
    Markers3DVector2D v={read(pos,0),read(pos,1)};return v;
}
static void Markers3D_AddZ(void *pos,float value,Markers3DRead read,Markers3DWrite write)
{
    // Preserve the original in-place height update and aliasing
    write(pos,2,read(pos,2)+value);
}
static Markers3DVector Markers3D_Subtract(Markers3DVector a,Markers3DVector b)
{
    // Preserve original component subtraction
    Markers3DVector v={a.x-b.x,a.y-b.y,a.z-b.z};return v;
}
static float Markers3D_Squared(Markers3DVector v)
{
    // Preserve the original three-product sum order
    return v.x*v.x+v.y*v.y+v.z*v.z;
}
static float Markers3D_Dot(Markers3DVector a,Markers3DVector b)
{
    // Retain the exact original camera-depth dot product
    return a.x*b.x+a.y*b.y+a.z*b.z;
}
static Markers3DVector2D Markers3D_Subtract2D(Markers3DVector2D a,Markers3DVector2D b)
{
    // Preserve the original two-dimensional displacement
    Markers3DVector2D v={a.x-b.x,a.y-b.y};return v;
}
static float Markers3D_Squared2D(Markers3DVector2D v)
{
    // Preserve the original squared planar distance
    return v.x*v.x+v.y*v.y;
}
static void Markers3D_Normalise(Markers3DVector *v)
{
    // Retain the original reciprocal-square-root normalizer and x-only zero fallback
    float sq=Markers3D_Squared(*v);
    if(sq>0.0f){
        // Scale each component in the original order
        float invsqrt=1.0f/sqrtf(sq);v->x*=invsqrt;v->y*=invsqrt;v->z*=invsqrt;
    }else{
        // Leave the original y and z values untouched
        v->x=1.0f;
    }
}
'''

def kernel(source):
    # Translate all seven complete methods while borrowing actual stable scalar storage
    body=section(source)
    body=body.replace('C3dMarker C3dMarkers::m_aMarkerArray[NUM3DMARKERS];\n','').replace('RpClump* C3dMarkers::m_pRpClumpArray[NUMMARKERTYPES];\n','').replace('int32 C3dMarkers::NumActiveMarkers;','int32_t Markers3D_NumActiveMarkers;')
    body=body.replace('C3dMarkers::','Markers3D_');body=re.sub(r'\bNumActiveMarkers\b','Markers3D_NumActiveMarkers',body)
    for name in ['Init','Shutdown','Render','Update']:body=body.replace('Markers3D_'+name+'()','Markers3D_'+name+'(void)')
    body=body.replace('RwCamera *camera','void *camera').replace('RwFrame *cameraFrame','void *cameraFrame').replace('RwMatrix *cameraMatrix','void *cameraMatrix')
    body=body.replace('RwCameraGetFrame(','Markers3D_BackendCameraFrame(').replace('RwFrameGetMatrix(','Markers3D_BackendFrameMatrix(').replace('RwCameraGetNearClipPlane(','Markers3D_BackendNearClip(').replace('RwCameraGetFarClipPlane(','Markers3D_BackendFarClip(')
    body=body.replace('CVector cameraPosition(cameraMatrix->pos);','Markers3DVector cameraPosition=Markers3D_BackendMatrixPosition(cameraMatrix);').replace('CVector cameraForward(cameraMatrix->at);','Markers3DVector cameraForward=Markers3D_BackendMatrixForward(cameraMatrix);').replace('cameraForward.Normalise()','Markers3D_Normalise(&cameraForward)')
    body=body.replace('CVector toMarker = marker->m_Matrix.GetPosition() - cameraPosition','Markers3DVector toMarker = Markers3D_Subtract(Markers3D_BackendPosition(marker),cameraPosition)').replace('toMarker.MagnitudeSqr()','Markers3D_Squared(toMarker)').replace('DotProduct(toMarker, cameraForward)','Markers3D_Dot(toMarker,cameraForward)')
    body=body.replace('C3dMarker *marker = &m_aMarkerArray[EnvMapMarkerIndices[i]]','void *marker = Markers3D_BackendMarker(EnvMapMarkerIndices[i])').replace('marker->Render()','Markers3D_BackendRender(marker)')
    a=body.index('for(int i = 0; i < NUM3DMARKERS; i++)');b=body.index('Markers3D_NumActiveMarkers = 0;',a)
    fragment=body[a:b];body=body[:a]+fragment.replace('for(int i = 0; i < NUM3DMARKERS; i++)','for(int i = 0; i < NUM3DMARKERS; i++){\n        void *marker=Markers3D_BackendMarker(i);Markers3DFields fields=Markers3D_BackendFields(marker);',1)+'\n    }\n    '+body[b:]
    body=body.replace('for (int i = 0; i < NUM3DMARKERS; i++) {','for (int i = 0; i < NUM3DMARKERS; i++) {\n        void *marker=Markers3D_BackendMarker(i);Markers3DFields fields=Markers3D_BackendFields(marker);')
    body=re.sub(r'CCoronas::RegisterCorona\([\s\S]*?0.0f, false\)',r'Markers3D_BackendCorona(marker,1.2f * (*fields.m_fSize),50.0f * Markers3D_BackendLOD())',body,count=1)
    body=body.replace('m_aMarkerArray[i].m_pAtomic = nil','Markers3D_BackendSetAtomic(marker,NULL)').replace('m_aMarkerArray[i].m_pAtomic','Markers3D_BackendAtomic(marker)').replace('m_aMarkerArray[i].DeleteMarkerObject()','Markers3D_BackendDelete(marker)').replace('m_aMarkerArray[i].Render()','Markers3D_BackendRender(marker)').replace('&m_aMarkerArray[i]','marker')
    for t,n in FIELDS:
        original='m_Color.'+n if n in ['red','green','blue','alpha'] else n
        body=body.replace('m_aMarkerArray[i].'+original,'(*fields.'+n+')')
    body=body.replace('m_pRpClumpArray[MARKERTYPE_ARROW] = CFileLoader::LoadAtomicFile2Return("models/generic/arrow.dff")','Markers3D_BackendSetClump(MARKERTYPE_ARROW,Markers3D_BackendLoad("models/generic/arrow.dff"))').replace('m_pRpClumpArray[MARKERTYPE_CYLINDER] = CFileLoader::LoadAtomicFile2Return("models/generic/zonecylb.dff")','Markers3D_BackendSetClump(MARKERTYPE_CYLINDER,Markers3D_BackendLoad("models/generic/zonecylb.dff"))')
    body=body.replace('m_pRpClumpArray[i]','Markers3D_BackendClump(i)').replace('RpClumpDestroy(','Markers3D_BackendDestroyClump(')
    for a,b in [('CTxdStore::FindTxdSlot','FindTxd'),('CTxdStore::PushCurrentTxd','PushTxd'),('CTxdStore::SetCurrentTxd','SetTxd'),('CTxdStore::PopCurrentTxd','PopTxd'),('ActivateDirectional','Directional')]:body=body.replace(a+'(','Markers3D_Backend'+b+'(')
    oldsig='C3dMarker *\nMarkers3D_PlaceMarker(uint32 identifier, uint16 type, CVector &pos, float size, uint8 r, uint8 g, uint8 b, uint8 a, uint16 pulsePeriod, float pulseFraction, int16 rotateRate)'
    assert oldsig in body
    body=body.replace(oldsig,'static void *\nMarkers3D_PlaceInput('+ARGS+',Markers3DRead read,Markers3DWrite write)')
    body=body.replace('C3dMarker *pMarker;','void *pMarker;').replace('CVector2D playerPos = FindPlayerCentreOfWorld(0)','Markers3DVector2D playerPos = Markers3D_BackendPlayer()').replace('((CVector2D)pos - playerPos).Magnitude()','sqrtf(Markers3D_Squared2D(Markers3D_Subtract2D(Markers3D_ReadPosition2D(pos,read),playerPos)))')
    rotation='''if (pMarker->m_nRotateRate != 0) {
			CVector pos = pMarker->m_Matrix.GetPosition();
			pMarker->m_Matrix.RotateZ(DEGTORAD(pMarker->m_nRotateRate * (*Timer_GetTimeStep())));
			pMarker->m_Matrix.GetPosition() = pos;
		}'''
    assert rotation in body
    body=body.replace(rotation,'''if (pMarker->m_nRotateRate != 0) {
            Markers3DVector savedPosition=Markers3D_BackendPosition(pMarker);
            Markers3D_BackendRotateZ(pMarker,DEGTORAD(pMarker->m_nRotateRate * (*Timer_GetTimeStep())));
            Markers3D_BackendSetPosition(pMarker,savedPosition);
        }''')
    body=re.sub(r'pMarker->m_Matrix.GetPosition\(\).z = ([^;]+);',r'Markers3D_BackendSetZ(pMarker,\1);',body)
    body=body.replace('pMarker->m_Matrix.GetPosition() = pos','Markers3D_BackendSetPosition(pMarker,Markers3D_ReadPosition(pos,read))').replace('pMarker->m_Matrix.SetTranslate(pos.x, pos.y, pos.z)','Markers3D_BackendSetTranslate(pMarker,Markers3D_ReadPosition(pos,read))').replace('pMarker->m_Matrix.RotateX(PI)','Markers3D_BackendRotateX(pMarker,PI)').replace('pMarker->m_Matrix.UpdateRW()','Markers3D_BackendUpdateRW(pMarker)')
    body=body.replace('(playerPos - pos).MagnitudeSqr()','Markers3D_Squared2D(Markers3D_Subtract2D(playerPos,Markers3D_ReadPosition2D(pos,read)))').replace('CColStore::HasCollisionLoaded(CVector2D(pos))','Markers3D_BackendCollision(Markers3D_ReadPosition2D(pos,read))').replace('CWorld::FindGroundZFor3DCoord(pos.x, pos.y, pos.z + 1.0f, nil)','Markers3D_BackendGround(read(pos,0),read(pos,1),read(pos,2)+1.0f)')
    body=re.sub(r'pos.z \+= ([^;]+);',r'Markers3D_AddZ(pos,\1,read,write);',body);body=re.sub(r'pos.z = ([^;]+);',r'write(pos,2,\1);',body)
    body=body.replace('pMarker->DeleteMarkerObject()','Markers3D_BackendDelete(pMarker)').replace('pMarker->AddMarker(','Markers3D_BackendAdd(pMarker,')
    for t,n in FIELDS:
        original='m_Color.'+n if n in ['red','green','blue','alpha'] else n
        body=body.replace('pMarker->'+original,'(*Markers3D_BackendFields(pMarker).'+n+')')
    a=body.index('if (pMarker == nil) return pMarker;');b=body.index('void\nMarkers3D_PlaceMarkerSet',a)
    segment=body[a:b];segment=segment.replace('if (pMarker == nil) return pMarker;','if (pMarker == nil) return pMarker;\n    Markers3DFields placed=Markers3D_BackendFields(pMarker);')
    segment=segment.replace('Markers3D_BackendFields(pMarker).','placed.');body=body[:a]+segment+body[b:]
    setsig='void\nMarkers3D_PlaceMarkerSet(uint32 id, uint16 type, CVector &pos, float size, uint8 r, uint8 g, uint8 b, uint8 a, uint16 pulsePeriod, float pulseFraction, int16 rotateRate)'
    assert setsig in body
    body=body.replace(setsig,'static void\nMarkers3D_SetInput('+ARGS.replace('identifier','id')+',Markers3DRead read,Markers3DWrite write)')
    body=re.sub(r'(?<!\w)PlaceMarker\(([^;]+)\);',r'Markers3D_PlaceInput(\1,read,write);',body)
    body=body.replace('Sin(','sinf(').replace('sq(100.f)','(100.f*100.f)').replace('nil','NULL')
    for a,b in [('uint32','uint32_t'),('uint16','uint16_t'),('uint8','uint8_t'),('int16','int16_t')]:body=re.sub(r'\b'+a+r'\b',b,body)
    body=body.replace('\n{\n','\n{\n    // Retain the complete original controller operation and live data-write order\n');body=re.sub(r'^(\t+)([^\n]*\{\n)',r'\1\2\1\t// Preserve original conditions, resource calls and borrowed-position mutations\n',body,flags=re.M)
    wrappers=''
    for name,result,invoke in [('PlaceMarker','void *','Markers3D_PlaceInput'),('PlaceMarkerSet','void','Markers3D_SetInput')]:
        for suffix,position,reader,writer in [('', 'Markers3DVector *pos','Markers3D_Read','Markers3D_Write'),('LegacyInput','void *pos','Markers3D_BackendReadLegacy','Markers3D_BackendWriteLegacy')]:wrappers+=result+' Markers3D_'+name+suffix+'('+ARGS.replace('void *pos',position)+')\n{\n    // Preserve all original lazy reads and in-place reference writes across complete placement calls\n    '+('return ' if result!='void' else '')+invoke+'('+INVOKE+','+reader+','+writer+');\n}\n'
    assert not re.search(r'\b(?:C3dMarkers|C3dMarker|CVector|CVector2D|CCoronas|CColStore|CWorld|CTxdStore|RwCamera|RwFrame|RwMatrix|m_aMarkerArray|m_pRpClumpArray|CFileLoader)\b',body)
    return records.kernel('//+ rouz edit (ChatGPT)\n#include "Markers3D_backend.h"\n#include "Timer.h"\n#include "FileMgr.h"\n#include <math.h>\n#include <stddef.h>\n#ifndef M_PI\n#define M_PI 3.14159265358979323846\n#endif\n#include "Draw_constants.h"\n\n'+HELPERS+body.rstrip()+'\n\n'+wrappers+'//- rouz edit (ChatGPT)\n',source)

def project(source):
    # Compile the full C controller and its typed owner access in separate paired objects
    if '..\\src\\renderer\\Markers3D.c"' not in source:
        match=re.search(r'<ClCompile Include="\.\.\\src\\renderer\\BoatFoam\.c">[\s\S]*?</ClCompile>',source)
        if match:
            entry=match[0]
            if '<Filter>' in entry:source=source.replace(entry,entry+'\n    '+entry.replace('BoatFoam.c','Markers3D.cpp')+'\n    '+entry.replace('BoatFoam.c','Markers3D.c'))
            else:source=source.replace(entry,entry+'\n    <ClCompile Include="..\\src\\renderer\\Markers3D.cpp" />\n    '+entry.replace('BoatFoam.c','Markers3D.c'))
    for name in ['Markers3D.h','Markers3D_backend.h']:
        if '..\\src\\renderer\\'+name+'"' in source:continue
        for entry in ['<ClInclude Include="..\\src\\renderer\\SpecialFX.h" />','<ClInclude Include="..\\src\\renderer\\SpecialFX.h">\n      <Filter>renderer</Filter>\n    </ClInclude>']:source=source.replace(entry,entry+'\n    '+entry.replace('SpecialFX.h',name))
    return records.project(source)
