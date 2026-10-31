"""Audit all marker-controller methods and compare them with the unchanged original owners."""
from pathlib import Path
import re,json
import markers_3d_migration as m
import text_migration as text
root=Path(__file__).resolve().parents[2];out=root/'build/markers-3d-c-tests'

def normalize(source):
    # Compare every complete statement independently of comments and formatting
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',source,flags=re.S))

for path in json.loads((out/'changes.json').read_text()):
    old=(out/'before'/path).read_text();expected=m.project(old) if path.startswith('build/') else m.calls(old)
    assert normalize(expected)==normalize((root/path).read_text()),path
    assert b'\r' not in (root/path).read_bytes(),path
old=(out/'before/src/renderer/SpecialFX.cpp').read_text();header=(out/'before/src/renderer/SpecialFX.h').read_text()
for name,value in [('Markers3D.c',m.kernel(old)),('Markers3D.cpp',m.records.calls(m.HOST)),('Markers3D.h',m.records.calls(m.HEADER)),('Markers3D_backend.h',m.records.calls(m.BACKEND_HEADER))]:assert (root/'src/renderer'/name).read_text()==value
assert normalize(m.records.calls(m.marker_header(header)))==normalize(m.marker_header((root/'src/renderer/SpecialFX.h').read_text()))
assert len(re.findall(r'\nC3dMarkers::\w+\(',m.section(old)))==7
for path in ['src/core/common.h','src/core/config.h','src/math/Vector.h','src/math/Vector.cpp','src/math/Vector2D.h','src/math/Matrix.h','src/math/Matrix.cpp','src/math/Rect.h','src/math/maths.h','src/control/Script.h','src/renderer/Coronas.h']:
    saved=out/'before'/path
    if not saved.exists():saved.parent.mkdir(parents=True,exist_ok=True);saved.write_bytes((root/path).read_bytes())
    text.verify_dependency(saved,root/path)
    if path.startswith('src/math/') and path.endswith('.h'):(out/Path(path).name).write_text(saved.read_text(),newline='\n')
common=(root/'build/special-fx-c-tests/common.h').read_text();saved=out/'before/fixture-common.h'
if not saved.exists():saved.write_text(common,newline='\n')
assert saved.read_text()==common
extra='''
#ifndef NATIVE_PROJECT_HEADERS
struct RpClump;struct RpGeometry;struct RpMaterial;
typedef RpAtomic *(*RpAtomicCallBack)(RpAtomic *,void *);
typedef RpAtomic *(*RpAtomicCallBackRender)(RpAtomic *);
enum { rpGEOMETRYMODULATEMATERIALCOLOR=0x40 };
RpClump *RpClumpForAllAtomics(RpClump *,RpAtomicCallBack,void *);
RpAtomic *RpAtomicClone(RpAtomic *);
RpAtomic *RpAtomicSetFrame(RpAtomic *,RwFrame *);
RpGeometry *RpAtomicGetGeometry(const RpAtomic *);
RwUInt32 RpGeometryGetFlags(const RpGeometry *);
RpGeometry *RpGeometrySetFlags(RpGeometry *,RwUInt32);
RpMaterial *RpGeometryGetMaterial(const RpGeometry *,RwInt32);
RpMaterial *RpMaterialSetColor(RpMaterial *,const RwRGBA *);
RpAtomic *RpAtomicRender(RpAtomic *);
RwBool RpAtomicDestroy(RpAtomic *);
RwBool RpClumpDestroy(RpClump *);
RwFrame *RwFrameCreate(void);
RwBool RwFrameDestroy(RwFrame *);
RwFrame *RwFrameUpdateObjects(RwFrame *);
RwFrame *RwCameraGetFrame(const RwCamera *);
RwMatrix *RwFrameGetMatrix(RwFrame *);
RwReal RwCameraGetNearClipPlane(const RwCamera *);
RwReal RwCameraGetFarClipPlane(const RwCamera *);
#endif
'''
(out/'common.h').write_text(common+extra,newline='\n')
oldenum=header[header.index('enum\n{\n\tMARKERTYPE_0'):header.index('class C3dMarker\n')]
enumprobe='namespace OriginalMarkerEnum {\n'+oldenum+'}\n'
for name in ['MARKERTYPE_0','MARKERTYPE_ARROW','MARKERTYPE_2','MARKERTYPE_3','MARKERTYPE_CYLINDER','NUMMARKERTYPES','MARKERTYPE_INVALID']:enumprobe+='static_assert((int)OriginalMarkerEnum::'+name+'==(int)'+name+',"Original '+name+'");\n'
(out/'SpecialFX.h').write_text('#pragma once\n#include "Markers3D.h"\n'+enumprobe+'#ifdef TEST_ORIGINAL\n'+m.marker_header(header)+'\n#else\n'+m.records.calls(m.marker_header(header))+'\n#endif\n',newline='\n')
(out/'original-controller.h').write_text('#pragma once\n#include "SpecialFX.h"\n'+m.controller_header(header)+'\n',newline='\n')
coronas=(out/'before/src/renderer/Coronas.h').read_text();enums='\n'.join(re.findall(r'\tenum \{[\s\S]*?\n\t\};',coronas))
owners='''#pragma once
#include "SpecialFX.h"
#include "Timer.h"
#include "FileMgr.h"
class CCamera { public:float LODDistMultiplier; };
extern CCamera TheCamera;
CVector FindPlayerCentreOfWorld(int32);
class CColStore { public:static bool HasCollisionLoaded(const CVector2D &); };
class CWorld { public:static float FindGroundZFor3DCoord(float,float,float,bool *); };
class CTxdStore { public:static int FindTxdSlot(const char *);static void PushCurrentTxd(void);static void SetCurrentTxd(int);static void PopCurrentTxd(void); };
class CFileLoader { public:static RpClump *LoadAtomicFile2Return(const char *); };
class CVisibilityPlugins { public:static void SetAtomicRenderCallback(RpAtomic *,RpAtomicCallBackRender); };
class CCoronas { public:
'''+enums+'''
static void RegisterCorona(uint32,uint8,uint8,uint8,uint8,const CVector &,float,float,uint8,int8,uint8,uint8,uint8,float,bool=false,float=1.5f);
};
void ActivateDirectional(void);
void SetBrightMarkerColours(float);
void ReSetAmbientAndDirectionalColours(void);
'''
(out/'owners.h').write_text(owners,newline='\n')
for name in ['World.h','ColStore.h','Camera.h','TxdStore.h','FileLoader.h','Lights.h','Coronas.h','VisibilityPlugins.h']:(out/name).write_text('#pragma once\n#include "owners.h"\n',newline='\n')
constants='\n'.join(line for line in (out/'before/src/control/Script.h').read_text().splitlines() if line.startswith('#define SPHERE_MARKER_'))
(out/'Script.h').write_text('#pragma once\n'+constants+'\n',newline='\n')
prototypes='''void *TestMk_Marker(int32_t index);
void *TestMk_Clump(int32_t index);
void TestMk_SetClump(int32_t index,void *value);
int32_t TestMk_Active(void);
void TestMk_SetActive(int32_t value);
int32_t TestMk_CacheCount(void);
int32_t TestMk_CacheIndex(int32_t index);
void TestMk_SetCache(int32_t count);
void TestMk_Init(void);
void TestMk_Shutdown(void);
void TestMk_Render(void);
void TestMk_Env(void *camera);
void TestMk_Update(void);
void *TestMk_PlaceC(uint32_t id,uint16_t type,Markers3DVector *pos,float size,uint16_t period,float fraction,int16_t rate,bool set);
void *TestMk_PlaceLegacy(uint32_t id,uint16_t type,void *pos,float size,uint16_t period,float fraction,int16_t rate,bool set);
void *TestMk_Borrowed(bool *legacy);
void TestMk_SetBorrowed(void *pos,bool legacy);
'''
(out/'caller.h').write_text('#pragma once\n#include "Markers3D.h"\n#ifdef __cplusplus\nextern "C" {\n#endif\n'+prototypes+'#ifdef __cplusplus\n}\n#endif\n',newline='\n')
prefix='#include "common.h"\n#include "owners.h"\n#include "Script.h"\n#include "caller.h"\n'
oldobserver=''
for name,result,args,body in [('Marker','void *','int32_t index','return &C3dMarkers::m_aMarkerArray[index];'),('Clump','void *','int32_t index','return C3dMarkers::m_pRpClumpArray[index];'),('SetClump','void','int32_t index,void *value','C3dMarkers::m_pRpClumpArray[index]=(RpClump *)value;'),('Active','int32_t','void','return C3dMarkers::NumActiveMarkers;'),('SetActive','void','int32_t value','C3dMarkers::NumActiveMarkers=value;'),('CacheCount','int32_t','void','return EnvMapMarkerCount;'),('CacheIndex','int32_t','int32_t index','return EnvMapMarkerIndices[index];'),('SetCache','void','int32_t count','EnvMapMarkerCount=count;for(int i=0;i<NUM3DMARKERS;i++)EnvMapMarkerIndices[i]=i;')]:oldobserver+='extern "C" '+result+' TestMk_'+name+'('+args+')\n{\n    // Observe or seed actual original storage without changing its algorithm\n    '+body+'\n}\n'
for name,args,invoke in [('Init','void','Init()'),('Shutdown','void','Shutdown()'),('Render','void','Render()'),('Env','void *camera','RenderForEnvMap((RwCamera *)camera)'),('Update','void','Update()')]:oldobserver+='extern "C" void TestMk_'+name+'('+args+')\n{\n    // Enter the complete original controller method\n    C3dMarkers::'+invoke+';\n}\n'
(out/'original.cpp').write_text(prefix+'#include "original-controller.h"\n'+m.section(old)+'\n'+oldobserver,newline='\n')
record=old[old.index('RpAtomic *\nMarkerAtomicCB('):old.index('C3dMarker C3dMarkers::m_aMarkerArray')]
record=record.replace('C3dMarkers::m_pRpClumpArray[type]','(RpClump *)TestMk_Clump(type)')
(out/'record-owner.cpp').write_text(prefix+record,newline='\n')
newhost=m.records.calls(m.HOST)+'\n#include "caller.h"\n'
for name,result,args,body in [('Marker','void *','int32_t index','return &MarkerOwners[index];'),('Clump','void *','int32_t index','return MarkerClumps[index];'),('SetClump','void','int32_t index,void *value','MarkerClumps[index]=(RpClump *)value;'),('Active','int32_t','void','return Markers3D_NumActiveMarkers;'),('SetActive','void','int32_t value','Markers3D_NumActiveMarkers=value;')]:newhost+='extern "C" '+result+' TestMk_'+name+'('+args+')\n{\n    // Observe or seed the same actual typed marker and clump objects\n    '+body+'\n}\n'
(out/'host.cpp').write_text(newhost,newline='\n')
observer='#include "caller.h"\n_Static_assert(sizeof(Markers3DFields)==16*sizeof(void *),"Typed scalar-pointer descriptor");\n'
for name,args,result,body in [('CacheCount','void','int32_t','return EnvMapMarkerCount;'),('CacheIndex','int32_t index','int32_t','return EnvMapMarkerIndices[index];'),('SetCache','int32_t count','void','EnvMapMarkerCount=count;for(int i=0;i<NUM3DMARKERS;i++)EnvMapMarkerIndices[i]=i;')]:observer+=result+' TestMk_'+name+'('+args+')\n{\n    // Observe the actual C-owned reflection snapshot without changing a decision\n    '+body+'\n}\n'
(out/'observed-kernel.c').write_text('#include "'+(root/'src/renderer/Markers3D.c').as_posix()+'"\n'+observer,newline='\n')
caller='#include "caller.h"\n'
for name,args,invoke in [('Init','void','Init()'),('Shutdown','void','Shutdown()'),('Render','void','Render()'),('Env','void *camera','RenderForEnvMap(camera)'),('Update','void','Update()')]:caller+='void TestMk_'+name+'('+args+')\n{\n    // Enter the actual public controller from a separate C11 unit\n    Markers3D_'+invoke+';\n}\n'
cargs='uint32_t id,uint16_t type,Markers3DVector *pos,float size,uint16_t period,float fraction,int16_t rate,bool set'
caller+='void *TestMk_PlaceC('+cargs+')\n{\n    // Keep the same logical borrowed input visible to owner callbacks\n    bool previousLegacy;void *previous=TestMk_Borrowed(&previousLegacy);TestMk_SetBorrowed(pos,false);void *result=NULL;\n    if(set)Markers3D_PlaceMarkerSet(id,type,pos,size,23,71,199,228,period,fraction,rate);else result=Markers3D_PlaceMarker(id,type,pos,size,23,71,199,228,period,fraction,rate);\n    TestMk_SetBorrowed(previous,previousLegacy);return result;\n}\n'
(out/'caller.c').write_text(caller,newline='\n')
oldadapter=prefix+'#include "original-controller.h"\n';newadapter=prefix
for original in [True,False]:
    text=oldadapter if original else newadapter
    args=cargs.replace('Markers3DVector *pos','void *input')
    text+='extern "C" void *TestMk_PlaceLegacy('+args+')\n{\n    // Use an actual mutable CVector and preserve nested callback bindings\n    CVector &pos=*(CVector *)input;bool previousLegacy;void *previous=TestMk_Borrowed(&previousLegacy);TestMk_SetBorrowed(&pos,true);void *result=NULL;\n'
    owner='C3dMarkers::' if original else 'Markers3D_';suffix='' if original else 'Legacy'
    text+='    if(set)'+owner+'PlaceMarkerSet'+suffix+'(id,type,pos,size,23,71,199,228,period,fraction,rate);else result='+owner+'PlaceMarker'+suffix+'(id,type,pos,size,23,71,199,228,period,fraction,rate);\n    TestMk_SetBorrowed(previous,previousLegacy);return result;\n}\n'
    if original:text+='extern "C" void *TestMk_PlaceC('+cargs+')\n{\n    // Supply a legal original CVector reference to the same logical native-C input\n    CVector value(pos->x,pos->y,pos->z);void *result=TestMk_PlaceLegacy(id,type,&value,size,period,fraction,rate,set);pos->x=value.x;pos->y=value.y;pos->z=value.z;return result;\n}\n'
    (out/('old-adapter.cpp' if original else 'legacy-caller.cpp')).write_text(text,newline='\n')
matrix=(out/'before/src/math/Matrix.cpp').read_text();parts=[]
for name in ['CMatrix','~CMatrix','SetUnity','Attach','Update','UpdateRW','SetTranslate','RotateX','RotateZ']:parts.append(m.function(matrix,'CMatrix::',name))
vector=(out/'before/src/math/Vector.cpp').read_text();a=vector.index('void\nCVector::Normalise');b=vector.index('\nCVector\nCrossProduct',a)
(out/'original-math.cpp').write_text('#include "common.h"\n'+'\n\n'.join(parts)+'\n'+vector[a:b],newline='\n')
(out/'test.cpp').write_text((root/'utils/tests/markers_3d.cpp.in').read_text(),newline='\n')
print('Audited all seven controller methods, unchanged record layout, typed live-field access and eleven complete source/project files')
