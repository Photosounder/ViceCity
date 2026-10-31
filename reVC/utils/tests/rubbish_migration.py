"""Convert all sheet/list, simulation, visibility and resource/render algorithms."""
import re
import object_names_migration as objects
import water_cannon_migration as water_cannon

HEADER='''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#ifdef __cplusplus
class CVehicle;
#else
typedef struct CVehicle CVehicle;
#endif
enum {
#ifdef SQUEEZE_PERFORMANCE
	NUM_RUBBISH_SHEETS = 32
#else
	NUM_RUBBISH_SHEETS = 64
#endif
};
typedef struct RubbishVector { float x, y, z; } RubbishVector;
typedef struct RubbishVector2 { float x, y; } RubbishVector2;
typedef struct RubbishSheet {
	RubbishVector m_basePos;
	RubbishVector m_animatedPos;
	float m_targetZ;
	int8_t m_state;
	int8_t m_animationType;
	uint32_t m_moveStart;
	uint32_t m_moveDuration;
	float m_animHeight;
	float m_xDist;
	float m_yDist;
	float m_angle;
	bool m_isVisible;
	bool m_targetIsVisible;
	struct RubbishSheet *m_next;
	struct RubbishSheet *m_prev;
} RubbishSheet;
#ifdef __cplusplus
extern "C" {
#endif
void RubbishSheet_AddToList(RubbishSheet *sheet, RubbishSheet *list);
void RubbishSheet_RemoveFromList(RubbishSheet *sheet);
void Rubbish_Render(void);
void Rubbish_StirUp(CVehicle *veh);
void Rubbish_Update(void);
void Rubbish_SetVisibility(bool visible);
void Rubbish_Init(void);
void Rubbish_Shutdown(void);
extern void *gpRubbishTexture[4];
extern uint16_t RubbishIndexList[6];
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''

# Keep each original owner operation as a separately typed boundary
READERS=[
('RubbishVector','CameraPosition','void','const CVector &v = TheCamera.GetPosition();\n\tRubbishVector result = {v.x, v.y, v.z};\n\treturn result;','Read the original complete typed camera position'),
('float','CameraX','void','return TheCamera.GetPosition().x;','Read only the original live camera X component'),
('float','CameraY','void','return TheCamera.GetPosition().y;','Read only the original live camera Y component'),
('float','CameraZ','void','return TheCamera.GetPosition().z;','Read only the original live camera Z component'),
('float','Orientation','void','return TheCamera.Orientation;','Read the original live camera orientation'),
('bool','EnvMap','void','#if defined(LIBRW) && defined(EXTENDED_PIPELINES)\n\treturn CustomPipes::bRenderingEnvMap;\n#else\n\treturn false;\n#endif','Read the original live environment-pass flag'),
('void *','EnvCamera','void','#if defined(LIBRW) && defined(EXTENDED_PIPELINES)\n\treturn CustomPipes::EnvMapCam;\n#else\n\treturn NULL;\n#endif','Read the original auxiliary camera pointer at each access point'),
('void *','CameraFrame','void *camera','return RwCameraGetFrame((RwCamera *)camera);','Read the actual typed auxiliary camera frame'),
('void *','FrameMatrix','void *frame','return RwFrameGetMatrix((RwFrame *)frame);','Read the actual typed frame matrix at each original query'),
('RubbishVector','MatrixPosition','void *matrix','const RwV3d &v = ((RwMatrix *)matrix)->pos;\n\tRubbishVector result = {v.x, v.y, v.z};\n\treturn result;','Copy the actual typed matrix position components'),
('uint16_t','VehicleSeed','CVehicle *veh','return veh->m_randomSeed;','Read the original entity seed through its actual owner'),
('float','VehicleX','CVehicle *veh','return veh->GetPosition().x;','Read only the original live vehicle X component'),
('float','VehicleY','CVehicle *veh','return veh->GetPosition().y;','Read only the original live vehicle Y component'),
('RubbishVector','VehiclePosition','CVehicle *veh','const CVector &v = veh->GetPosition();\n\tRubbishVector result = {v.x, v.y, v.z};\n\treturn result;','Copy the original complete vehicle position at the subtraction'),
('float','SpeedX','CVehicle *veh','return veh->GetMoveSpeed().x;','Read only the original live vehicle speed X component'),
('float','SpeedY','CVehicle *veh','return veh->GetMoveSpeed().y;','Read only the original live vehicle speed Y component'),
('RubbishVector2','Speed','CVehicle *veh','const CVector &v = veh->GetMoveSpeed();\n\tRubbishVector2 result = {v.x, v.y};\n\treturn result;','Copy only the original speed components used by two-dimensional math'),
('RubbishVector2','Forward','CVehicle *veh','const CVector &v = veh->GetForward();\n\tRubbishVector2 result = {v.x, v.y};\n\treturn result;','Copy the original forward components at the original query'),
('RubbishVector2','Right','CVehicle *veh','const CVector &v = veh->GetRight();\n\tRubbishVector2 result = {v.x, v.y};\n\treturn result;','Copy the original right components at the original query'),
('RubbishVector2','Bounds','CVehicle *veh','const CVector &v = veh->GetColModel()->boundingBox.max;\n\tRubbishVector2 result = {v.x, v.y};\n\treturn result;','Read the actual collision model and its original planar maximum'),
('float','Ground','float x, float y, float z, bool *found','return CWorld::FindGroundZFor3DCoord(x, y, z, found);','Enter the original ground query with its original output pointer'),
('float','Wind','void','return CWeather::Wind;','Read the original live wind scalar at each access point'),
('int32_t','Pamphlet','void','return CStats::PamphletMissionPassed;','Read the original mission flag only when selecting the fourth raster'),
('int32_t *','VertexCount','void','return &TempBufferVerticesStored;','Expose the actual shared vertex counter without copying it'),
('int32_t *','IndexCount','void','return &TempBufferIndicesStored;','Expose the actual shared index counter without copying it'),
('void','SetIndex','int index, uint32_t value','TempBufferRenderIndexList[index] = value;','Retain the original final narrowing into the typed shared index buffer'),
('void','TxdPush','void','CTxdStore::PushCurrentTxd();','Save the original current texture dictionary'),
('int','TxdFind','const char *name','return CTxdStore::FindTxdSlot(name);','Find the original particle dictionary'),
('void','TxdSet','int slot','CTxdStore::SetCurrentTxd(slot);','Select the original texture dictionary slot'),
('void','TxdPop','void','CTxdStore::PopCurrentTxd();','Restore the original current texture dictionary'),
('void *','TextureRead','const char *name, const char *mask','return RwTextureRead(name, mask);','Read the original typed texture resource'),
('void','TextureDestroy','void *texture','RwTextureDestroy((RwTexture *)texture);','Destroy the original typed texture before C clears the handle'),
('void *','Raster','void *texture','return RwTextureGetRaster((RwTexture *)texture);','Read the actual raster for the original texture handle'),
('void','TextureRaster','void *raster','RwRenderStateSet(rwRENDERSTATETEXTURERASTER, raster);','Select the same typed texture raster'),
('void','ZWrite','bool enabled','RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)(uintptr_t)enabled);','Set the original typed depth-write state'),
('void','VertexAlpha','bool enabled','RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)(uintptr_t)enabled);','Set the original typed vertex-alpha state'),
('void','Fog','bool enabled','RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)(uintptr_t)enabled);','Set the original typed fog state'),
('void','SetPos','int index, float x, float y, float z','RwIm3DVertexSetPos(&TempBufferRenderVertices[index], x, y, z);','Write the original typed shared vertex position'),
('void','SetRGBA','int index, uint8_t r, uint8_t g, uint8_t b, uint32_t alpha','RwIm3DVertexSetRGBA(&TempBufferRenderVertices[index], r, g, b, alpha);','Retain the original final alpha narrowing in the typed color setter'),
('void','SetU','int index, float value','RwIm3DVertexSetU(&TempBufferRenderVertices[index], value);','Write the original typed U coordinate'),
('void','SetV','int index, float value','RwIm3DVertexSetV(&TempBufferRenderVertices[index], value);','Write the original typed V coordinate'),
('void','LittleTest','void','LittleTest();','Enter the original hook after shared-buffer generation'),
('bool','Transform','int count','return RwIm3DTransform(TempBufferRenderVertices, count, nil, rwIM3D_VERTEXUV) != nil;','Retain the original shared vertices, null matrix and UV flag'),
('void','RenderIndexed','int count','RwIm3DRenderIndexedPrimitive(rwPRIMTYPETRILIST, TempBufferRenderIndexList, count);','Render the original shared triangle-list index count'),
('void','End','void','RwIm3DEnd();','Finish the original successful draw'),
('void','Push','const char *name','PUSH_RENDERGROUP(name);','Preserve the original debug group boundary'),
('void','Pop','void','POP_RENDERGROUP();','Close the original debug group when enabled')]
BACKEND_HEADER='//+ rouz edit (ChatGPT)\n#pragma once\n#include "Rubbish.h"\n#ifdef __cplusplus\nextern "C" {\n#endif\n'+''.join(t+' Rubbish_Backend'+name+'('+args+');\n' for t,name,args,body,comment in READERS)+'#ifdef __cplusplus\n}\n#endif\n//- rouz edit (ChatGPT)\n'
HOST='''//+ rouz edit (ChatGPT)
#include "common.h"
#include "main.h"
#include "Weather.h"
#include "Camera.h"
#include "World.h"
#include "Vehicle.h"
#include "Stats.h"
#include "TxdStore.h"
#include "RenderBuffer.h"
#include "Rubbish_backend.h"
#if defined(LIBRW) && defined(EXTENDED_PIPELINES)
#include "custompipes.h"
#endif

'''+''.join(t+' Rubbish_Backend'+name+'('+args+')\n{\n\t// '+comment+'\n\t'+body+'\n}\n' for t,name,args,body,comment in READERS)+'//- rouz edit (ChatGPT)\n'
HELPERS='''#define Min(a,b) ((a) < (b) ? (a) : (b))
#define Max(a,b) ((a) > (b) ? (a) : (b))
#define SQR(x) ((x) * (x))
#ifdef _MSC_VER
float fast_absf(float x);
#define Rubbish_Abs fast_absf
#else
#define Rubbish_Abs fabsf
#endif
static RubbishVector Rubbish_Subtract(RubbishVector left, RubbishVector right)
{
	// Preserve all three original vector subtractions before planar conversion
	RubbishVector result = {left.x-right.x, left.y-right.y, left.z-right.z};
	return result;
}
static RubbishVector2 Rubbish_XY(RubbishVector vector)
{
	// Preserve the original planar constructor component copies
	RubbishVector2 result = {vector.x, vector.y};
	return result;
}
static float Rubbish_MagnitudeSqr2(RubbishVector2 vector)
{
	// Retain the original two float products and addition
	return vector.x*vector.x + vector.y*vector.y;
}
static float Rubbish_Magnitude2(RubbishVector2 vector)
{
	// Retain the native float square-root wrapper
	return sqrtf(Rubbish_MagnitudeSqr2(vector));
}
static float Rubbish_Dot2(RubbishVector2 left, RubbishVector2 right)
{
	// Retain the original planar dot-product operation order
	return left.x*right.x + left.y*right.y;
}

'''

def calls(source):
    # Convert every class caller and the separate typed Fluff texture consumer
    if 'class COneSheet\n' in source:return HEADER
    if 'bool CRubbish::bRubbishInvisible;' in source:return HOST
    changed=bool(re.search(r'\bCRubbish::\w+\(',source))
    if 'RwTextureGetRaster(gpRubbishTexture[2])' in source:
        if '#include "Rubbish_backend.h"' not in source:
            anchor='#include "Rubbish.h"' if '#include "Rubbish.h"' in source else '#include "common.h"'
            source=source.replace(anchor,anchor+'\n#include "Rubbish_backend.h"')
        source=source.replace('RwTextureGetRaster(gpRubbishTexture[2])','Rubbish_BackendRaster(gpRubbishTexture[2])')
        source=re.sub(r'^(\s*)(RwRenderStateSet\(rwRENDERSTATETEXTURERASTER, Rubbish_BackendRaster\(gpRubbishTexture\[2\]\)\);)',r'\1// Read the C-owned rubbish texture through its typed raster boundary\n\1\2',source,flags=re.M)
    source=re.sub(r'\bCRubbish::(\w+)\(',r'Rubbish_\1(',source)
    if changed:
        lines=[]
        for line in source.splitlines(keepends=True):
            if re.search(r'\bRubbish_(?:Render|StirUp|Update|SetVisibility|Init|Shutdown)\(',line):
                comment=re.match(r'\s*',line)[0]+'// Enter the C rubbish algorithm with the original owner expressions\n'
                if not lines or lines[-1]!=comment:lines.append(comment)
            lines.append(line)
        source=''.join(lines)
    return water_cannon.calls(source)

def kernel(source):
    # Relocate both list methods and all six full static-class algorithms
    source=source[source.index('#define RUBBISH_MAX_DIST'):]
    source=source.replace('RwTexture *gpRubbishTexture[4];','void *gpRubbishTexture[4];').replace('RwImVertexIndex RubbishIndexList','uint16_t RubbishIndexList')
    source=re.sub(r'^(bool|int|COneSheet) CRubbish::(\w+)',r'static \1 \2',source,flags=re.M)
    source=source.replace('COneSheet::AddToList(COneSheet *list)','RubbishSheet_AddToList(RubbishSheet *sheet, RubbishSheet *list)')
    source=source.replace('COneSheet::RemoveFromList(void)','RubbishSheet_RemoveFromList(RubbishSheet *sheet)')
    source=source.replace('this->','sheet->').replace('= this;','= sheet;')
    source=source.replace('m_next->m_prev = m_prev;','sheet->m_next->m_prev = sheet->m_prev;').replace('m_prev->m_next = m_next;','sheet->m_prev->m_next = sheet->m_next;')
    source=re.sub(r'\bCOneSheet\b','RubbishSheet',source)
    source=re.sub(r'\bCRubbish::(\w+)\(',r'Rubbish_\1(',source)
    source=re.sub(r'(sheet|aSheets\[i\])->RemoveFromList\(\)',r'RubbishSheet_RemoveFromList(\1)',source)
    source=re.sub(r'sheet->AddToList\(([^)]+)\)',r'RubbishSheet_AddToList(sheet, \1)',source)
    source=source.replace('aSheets[i].RemoveFromList()','RubbishSheet_RemoveFromList(&aSheets[i])')
    source=re.sub(r'aSheets\[i\]\.AddToList\(([^)]+)\)',r'RubbishSheet_AddToList(&aSheets[i], \1)',source)
    source=source.replace('CVector rubbishCameraPosition = TheCamera.GetPosition();','RubbishVector rubbishCameraPosition = Rubbish_BackendCameraPosition();')
    source=source.replace('CustomPipes::bRenderingEnvMap','Rubbish_BackendEnvMap()').replace('CustomPipes::EnvMapCam','Rubbish_BackendEnvCamera()')
    source=source.replace('RwCamera *effectCamera = (RwCamera*)Rubbish_BackendEnvCamera();','void *effectCamera = Rubbish_BackendEnvCamera();')
    source=source.replace('RwFrame *effectFrame = RwCameraGetFrame(effectCamera);','void *effectFrame = Rubbish_BackendCameraFrame(effectCamera);')
    source=source.replace('RwFrameGetMatrix(effectFrame)','Rubbish_BackendFrameMatrix(effectFrame)')
    source=source.replace('CVector(Rubbish_BackendFrameMatrix(effectFrame)->pos)','Rubbish_BackendMatrixPosition(Rubbish_BackendFrameMatrix(effectFrame))')
    source=source.replace('if(RubbishVisibility == 0)\n\t\treturn;','if(RubbishVisibility == 0)\n\t\treturn;\n\n\t// Keep live counter accesses attached to the original shared storage\n\tint32_t *indicesStored = Rubbish_BackendIndexCount();\n\tint32_t *verticesStored = Rubbish_BackendVertexCount();')
    source=source.replace('TempBufferIndicesStored','(*indicesStored)').replace('TempBufferVerticesStored','(*verticesStored)')
    source=source.replace('CStats::PamphletMissionPassed','Rubbish_BackendPamphlet()').replace('CWeather::Wind','Rubbish_BackendWind()')
    source=source.replace('CWorld::FindGroundZFor3DCoord(','Rubbish_BackendGround(')
    source=source.replace('CullZonesPosition_FromVector(sheet->m_basePos)','(CullZonesPosition){sheet->m_basePos.x, sheet->m_basePos.y, sheet->m_basePos.z}')
    source=source.replace('CullZonesPosition_FromVector(CVector(tx, ty, aSheets[i].m_targetZ))','(CullZonesPosition){tx, ty, aSheets[i].m_targetZ}')
    source=source.replace('(pos - rubbishCameraPosition).Magnitude2D()','Rubbish_Magnitude2(Rubbish_XY(Rubbish_Subtract(pos, rubbishCameraPosition)))')
    source=source.replace('(aSheets[i].m_basePos - TheCamera.GetPosition()).MagnitudeSqr2D()','Rubbish_MagnitudeSqr2(Rubbish_XY(Rubbish_Subtract(aSheets[i].m_basePos, Rubbish_BackendCameraPosition())))')
    source=source.replace('veh->GetPosition().x','Rubbish_BackendVehicleX(veh)').replace('veh->GetPosition().y','Rubbish_BackendVehicleY(veh)')
    source=source.replace('TheCamera.GetPosition().x','Rubbish_BackendCameraX()').replace('TheCamera.GetPosition().y','Rubbish_BackendCameraY()').replace('TheCamera.GetPosition().z','Rubbish_BackendCameraZ()').replace('TheCamera.Orientation','Rubbish_BackendOrientation()')
    source=source.replace('veh->m_randomSeed','Rubbish_BackendVehicleSeed(veh)').replace('veh->GetMoveSpeed().x','Rubbish_BackendSpeedX(veh)').replace('veh->GetMoveSpeed().y','Rubbish_BackendSpeedY(veh)')
    source=source.replace('veh->GetMoveSpeed().Magnitude2D()','Rubbish_Magnitude2(Rubbish_BackendSpeed(veh))')
    source=source.replace('CVector2D size = veh->GetColModel()->boundingBox.max;','RubbishVector2 size = Rubbish_BackendBounds(veh);')
    source=source.replace('CVector2D carToSheet = sheet->m_basePos - veh->GetPosition();','RubbishVector2 carToSheet = Rubbish_XY(Rubbish_Subtract(sheet->m_basePos, Rubbish_BackendVehiclePosition(veh)));')
    source=source.replace('veh->GetMoveSpeed()','Rubbish_BackendSpeed(veh)').replace('veh->GetForward()','Rubbish_BackendForward(veh)').replace('veh->GetRight()','Rubbish_BackendRight(veh)')
    source=source.replace('DotProduct2D(','Rubbish_Dot2(').replace('CVector pos;','RubbishVector pos;')
    for before,after in [('uint32 ','uint32_t '),('uint8 ','uint8_t '),('Abs(','Rubbish_Abs('),('Sqrt(','sqrtf('),('Sin(','sinf('),('Cos(','cosf(')]:source=source.replace(before,after)
    for old,name in [('PushCurrentTxd','TxdPush'),('FindTxdSlot','TxdFind'),('SetCurrentTxd','TxdSet'),('PopCurrentTxd','TxdPop')]:source=source.replace('CTxdStore::'+old+'(','Rubbish_Backend'+name+'(')
    source=source.replace('RwTextureRead(','Rubbish_BackendTextureRead(').replace('RwTextureDestroy(','Rubbish_BackendTextureDestroy(').replace('RwTextureGetRaster(','Rubbish_BackendRaster(')
    source=source.replace('RwRenderStateSet(rwRENDERSTATETEXTURERASTER, Rubbish_BackendRaster(gpRubbishTexture[type]));','Rubbish_BackendTextureRaster(Rubbish_BackendRaster(gpRubbishTexture[type]));')
    for key,name in [('ZWRITEENABLE','ZWrite'),('VERTEXALPHAENABLE','VertexAlpha'),('FOGENABLE','Fog')]:
        for old,value in [('TRUE','true'),('FALSE','false')]:source=source.replace('RwRenderStateSet(rwRENDERSTATE'+key+', (void*)'+old+');','Rubbish_Backend'+name+'('+value+');')
    for operation in ['Pos','RGBA','U','V']:source=re.sub(r'RwIm3DVertexSet'+operation+r'\(&TempBufferRenderVertices\[([^\]]+)\],',r'Rubbish_BackendSet'+operation+r'(\1,',source)
    source=re.sub(r'TempBufferRenderIndexList\[([^\]]+)\] = ([^;]+);',r'Rubbish_BackendSetIndex(\1, \2);',source)
    source=source.replace('LittleTest()','Rubbish_BackendLittleTest()').replace('RwIm3DTransform(TempBufferRenderVertices, (*verticesStored), nil, rwIM3D_VERTEXUV)','Rubbish_BackendTransform((*verticesStored))').replace('RwIm3DRenderIndexedPrimitive(rwPRIMTYPETRILIST, TempBufferRenderIndexList, (*indicesStored))','Rubbish_BackendRenderIndexed((*indicesStored))').replace('RwIm3DEnd()','Rubbish_BackendEnd()')
    source=source.replace('PUSH_RENDERGROUP(','Rubbish_BackendPush(').replace('POP_RENDERGROUP()','Rubbish_BackendPop()').replace('nil','NULL')
    source=re.sub(r'\t*//[+-] rouz edit \(ChatGPT\)\n','',source)
    comments={'RubbishSheet_AddToList':'Insert immediately after the original node without clearing other payloads','RubbishSheet_RemoveFromList':'Unlink both neighbors while retaining this node links','Rubbish_Render':'Preserve camera selection, texture groups and the complete shared-buffer draw sequence','Rubbish_StirUp':'Preserve lazy vehicle eligibility and saved-next list traversal','Rubbish_Update':'Preserve random draws, live inputs, animation arithmetic and list transitions','Rubbish_SetVisibility':'Set the original inverted visibility target','Rubbish_Init':'Link every original record and sentinel before acquiring textures','Rubbish_Shutdown':'Destroy and clear each original texture in order'}
    for name,comment in comments.items():source=re.sub(r'('+name+r'\([^\n]*\)\n\{)',r'\1\n\t// '+comment,source)
    source=re.sub(r'^(\t+)([^\n]*\{\n)',r'\1\2\1\t// Retain the original sheet traversal and update order\n',source,flags=re.M)
    assert not re.search(r'\b(?:CRubbish|COneSheet|CVector|CVector2D|RwCamera|RwFrame|RwIm3DVertex|TempBufferRenderVertices)\b',re.sub(r'"[^"\n]*"','',source))
    return '//+ rouz edit (ChatGPT)\n#include "Rubbish_backend.h"\n#include "Timer.h"\n#include "General.h"\n#include "ZoneCull.h"\n#include <stddef.h>\n#include <math.h>\n\n'+HELPERS+source.rstrip()+'\n//- rouz edit (ChatGPT)\n'

def project(source):
    # Compile C11 algorithms beside typed C++ operations with distinct objects
    if '..\\src\\renderer\\Rubbish.c"' not in source:
        entry='<ClCompile Include="..\\src\\renderer\\Rubbish.cpp" />';addition='<ClCompile Include="..\\src\\renderer\\Rubbish.c">\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>'
        source=source.replace(entry,entry+'\n    '+addition)
        entry='<ClCompile Include="..\\src\\renderer\\Rubbish.cpp">\n      <Filter>renderer</Filter>\n    </ClCompile>';source=source.replace(entry,entry+'\n    '+entry.replace('Rubbish.cpp','Rubbish.c'))
    if '..\\src\\renderer\\Rubbish_backend.h"' not in source:
        for entry in ['<ClInclude Include="..\\src\\renderer\\Rubbish.h" />','<ClInclude Include="..\\src\\renderer\\Rubbish.h">\n      <Filter>renderer</Filter>\n    </ClInclude>']:source=source.replace(entry,entry+'\n    '+entry.replace('Rubbish.h','Rubbish_backend.h'))
    return water_cannon.project(objects.project(source))
