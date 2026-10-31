"""Move complete skidmark algorithms while preserving lazy borrowed positions."""
import re
import object_names_migration as objects
import rubbish_migration as rubbish

HEADER='''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
enum { SKIDMARK_LENGTH = 16 };
typedef enum eSkidmarkType {
	SKIDMARK_NORMAL,
	SKIDMARK_MUDDY,
	SKIDMARK_SANDY,
	SKIDMARK_BLOODY
} eSkidmarkType;
typedef struct SkidmarksVector { float x, y, z; } SkidmarksVector;
typedef struct Skidmark {
	SkidmarksVector m_pos[SKIDMARK_LENGTH];
	float m_sideX[SKIDMARK_LENGTH];
	float m_sideY[SKIDMARK_LENGTH];
	uintptr_t m_id;
	uint32_t m_lastUpdate;
	uint32_t m_fadeStart;
	uint32_t m_fadeEnd;
	uint32_t m_type;
	int16_t m_last;
	uint8_t m_state;
	bool m_wasUpdated;
} Skidmark;
#ifdef __cplusplus
extern "C" {
#endif
void Skidmarks_Init(void);
void Skidmarks_Shutdown(void);
void Skidmarks_Clear(void);
void Skidmarks_Update(void);
void Skidmarks_Render(void);
void Skidmarks_RegisterOneType(uintptr_t id, const void *pos, float fwdX, float fwdY, eSkidmarkType type, bool *isBloody);
void Skidmarks_RegisterOneFlags(uintptr_t id, const void *pos, float fwdX, float fwdY, bool *isMuddy, bool *isBloody);
extern uint16_t SkidmarkIndexList[SKIDMARK_LENGTH * 6];
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
BACKEND_HEADER='''//+ rouz edit (ChatGPT)
#pragma once
#include "Skidmarks.h"
#ifdef __cplusplus
class CVector;
const void *Skidmarks_BackendPosition(const CVector &position);
extern "C" {
#endif
SkidmarksVector Skidmarks_BackendReadPosition(const void *position);
void Skidmarks_BackendTxdPush(void);
int Skidmarks_BackendTxdFind(const char *name);
void Skidmarks_BackendTxdSet(int slot);
void Skidmarks_BackendTxdPop(void);
void *Skidmarks_BackendTextureRead(const char *name, const char *mask);
void Skidmarks_BackendTextureDestroy(void *texture);
void *Skidmarks_BackendRaster(void *texture);
void Skidmarks_BackendTextureRaster(void *raster);
void Skidmarks_BackendZWrite(bool enabled);
void Skidmarks_BackendZTest(void);
void Skidmarks_BackendVertexAlpha(bool enabled);
void Skidmarks_BackendSrcBlend(void);
void Skidmarks_BackendDestBlend(void);
void Skidmarks_BackendSetRGBA(int index, uint8_t red, uint8_t green, uint8_t blue, uint32_t alpha);
void Skidmarks_BackendSetPos(int index, float x, float y, float z);
void Skidmarks_BackendSetU(int index, float value);
void Skidmarks_BackendSetV(int index, float value);
void Skidmarks_BackendLittleTest(void);
bool Skidmarks_BackendTransform(int count);
void Skidmarks_BackendRenderIndexed(uint16_t *indices, int count);
void Skidmarks_BackendEnd(void);
void Skidmarks_BackendPush(const char *name);
void Skidmarks_BackendPop(void);
bool Skidmarks_BackendPlayingBack(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
HOST='''//+ rouz edit (ChatGPT)
#include "common.h"
#include "main.h"
#include "TxdStore.h"
#include "Replay.h"
#include "Skidmarks_backend.h"

RwIm3DVertex SkidmarkVertices[SKIDMARK_LENGTH * 2];

const void *Skidmarks_BackendPosition(const CVector &position)
{
	// Borrow the original object without reading its fields before replay checks
	return &position;
}
SkidmarksVector Skidmarks_BackendReadPosition(const void *position)
{
	// Read the actual typed position only at its original assignment point
	const CVector *value = (const CVector *)position;
	SkidmarksVector result = {value->x, value->y, value->z};
	return result;
}
void Skidmarks_BackendTxdPush(void)
{
	// Save the original current texture dictionary
	CTxdStore::PushCurrentTxd();
}
int Skidmarks_BackendTxdFind(const char *name)
{
	// Find the original named texture dictionary
	return CTxdStore::FindTxdSlot(name);
}
void Skidmarks_BackendTxdSet(int slot)
{
	// Select the same original dictionary slot
	CTxdStore::SetCurrentTxd(slot);
}
void Skidmarks_BackendTxdPop(void)
{
	// Restore the original current texture dictionary
	CTxdStore::PopCurrentTxd();
}
void *Skidmarks_BackendTextureRead(const char *name, const char *mask)
{
	// Retain the original typed texture load and names
	return RwTextureRead(name, mask);
}
void Skidmarks_BackendTextureDestroy(void *texture)
{
	// Destroy the original typed texture at the C algorithm's release point
	RwTextureDestroy((RwTexture *)texture);
}
void *Skidmarks_BackendRaster(void *texture)
{
	// Read the raster through the original typed texture API
	return RwTextureGetRaster((RwTexture *)texture);
}
void Skidmarks_BackendTextureRaster(void *raster)
{
	// Select the same raster in the original render state
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, raster);
}
void Skidmarks_BackendZWrite(bool enabled)
{
	// Preserve the original typed depth-write state
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)(uintptr_t)enabled);
}
void Skidmarks_BackendZTest(void)
{
	// Restore the original enabled depth-test state
	RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)TRUE);
}
void Skidmarks_BackendVertexAlpha(bool enabled)
{
	// Preserve the original typed vertex-alpha state
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)(uintptr_t)enabled);
}
void Skidmarks_BackendSrcBlend(void)
{
	// Set the original source blend function
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDSRCALPHA);
}
void Skidmarks_BackendDestBlend(void)
{
	// Set the original destination blend function
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDINVSRCALPHA);
}
void Skidmarks_BackendSetRGBA(int index, uint8_t red, uint8_t green, uint8_t blue, uint32_t alpha)
{
	// Retain the original final alpha narrowing in the typed vertex setter
	RwIm3DVertexSetRGBA(&SkidmarkVertices[index], red, green, blue, alpha);
}
void Skidmarks_BackendSetPos(int index, float x, float y, float z)
{
	// Write the original persistent vertex buffer through its typed position setter
	RwIm3DVertexSetPos(&SkidmarkVertices[index], x, y, z);
}
void Skidmarks_BackendSetU(int index, float value)
{
	// Write the original typed U coordinate
	RwIm3DVertexSetU(&SkidmarkVertices[index], value);
}
void Skidmarks_BackendSetV(int index, float value)
{
	// Write the original typed V coordinate
	RwIm3DVertexSetV(&SkidmarkVertices[index], value);
}
void Skidmarks_BackendLittleTest(void)
{
	// Preserve the original hook after all vertex updates
	LittleTest();
}
bool Skidmarks_BackendTransform(int count)
{
	// Retain the original vertex count, null matrix and UV flag
	return RwIm3DTransform(SkidmarkVertices, count, nil, rwIM3D_VERTEXUV) != nil;
}
void Skidmarks_BackendRenderIndexed(uint16_t *indices, int count)
{
	// Render the same triangle-list index count after a successful transform
	RwIm3DRenderIndexedPrimitive(rwPRIMTYPETRILIST, indices, count);
}
void Skidmarks_BackendEnd(void)
{
	// Finish the original successful draw
	RwIm3DEnd();
}
void Skidmarks_BackendPush(const char *name)
{
	// Preserve the original debug group boundary
	PUSH_RENDERGROUP(name);
}
void Skidmarks_BackendPop(void)
{
	// Close the original debug group when enabled
	POP_RENDERGROUP();
}
bool Skidmarks_BackendPlayingBack(void)
{
	// Query the original live replay owner before any position read
	return CReplay::IsPlayingBack();
}
//- rouz edit (ChatGPT)
'''
HELPERS='''typedef struct SkidmarksVector2 { float x, y; } SkidmarksVector2;
typedef struct SkidmarksColor { uint8_t red, green, blue, alpha; } SkidmarksColor;
#ifdef _MSC_VER
float fast_absf(float x);
#define Skidmarks_Abs fast_absf
#else
#define Skidmarks_Abs fabsf
#endif
static void Skidmarks_Normalise(SkidmarksVector2 *vector)
{
	// Retain the original float reciprocal root and x-only fallback
	float sq = vector->x*vector->x + vector->y*vector->y;
	if (sq > 0.0f) {
		// Scale the original two components in order
		float invsqrt = 1.0f/sqrtf(sq);
		vector->x *= invsqrt;
		vector->y *= invsqrt;
	} else
		vector->x = 1.0f;
}
static float Skidmarks_Dot2(SkidmarksVector2 left, SkidmarksVector2 right)
{
	// Retain the original multiply and add order
	return left.x*right.x + left.y*right.y;
}

'''

def calls(source):
    # Preserve all eight actual typed-overload selections and their borrowed expressions
    if 'class CSkidmark\n' in source:return HEADER
    if 'CSkidmark CSkidmarks::aSkidmarks[NUMSKIDMARKS];' in source:return HOST
    changed='CSkidmarks::' in source
    if 'CSkidmarks::RegisterOne(' in source:
        if '#include "Skidmarks_backend.h"' not in source:
            # Include the typed bridge explicitly even when the public API was inherited through a vehicle header
            anchor='#include "Skidmarks.h"' if '#include "Skidmarks.h"' in source else '#include "common.h"'
            source=source.replace(anchor,anchor+'\n#include "Skidmarks_backend.h"')
        source=re.sub(r'CSkidmarks::RegisterOne\(([^,]+),\s*([^,]+),',lambda m:'Skidmarks_RegisterOneType('+m[1]+', Skidmarks_BackendPosition('+m[2].strip()+'),',source)
    source=re.sub(r'\bCSkidmarks::(\w+)\(',r'Skidmarks_\1(',source)
    if changed:
        lines=[]
        for line in source.splitlines(keepends=True):
            if re.search(r'\bSkidmarks_(?:Init|Shutdown|Clear|Update|Render|RegisterOneType)\(',line):
                comment=re.match(r'\s*',line)[0]+'// Enter the C skidmark algorithm with the original owner expressions\n'
                if not lines or lines[-1]!=comment:lines.append(comment)
            lines.append(line)
        source=''.join(lines)
    return rubbish.calls(source)

def kernel(source):
    # Preserve every original algorithm while making the C record and typed boundaries explicit
    source=source[source.index('CSkidmark CSkidmarks::aSkidmarks'):]
    source=source.replace('CSkidmark CSkidmarks::aSkidmarks[NUMSKIDMARKS];','static Skidmark aSkidmarks[NUMSKIDMARKS];').replace('RwImVertexIndex SkidmarkIndexList','uint16_t SkidmarkIndexList')
    source=source.replace('RwIm3DVertex SkidmarkVertices[SKIDMARK_LENGTH * 2];\n','').replace('RwTexture *gpSkidTex;','static void *gpSkidTex;')
    source=source.replace('CSkidmarks::RegisterOne(uintptr id, const CVector &pos, float fwdX, float fwdY, bool *isMuddy, bool *isBloody)','Skidmarks_RegisterOneFlags(uintptr_t id, const void *pos, float fwdX, float fwdY, bool *isMuddy, bool *isBloody)')
    source=source.replace('CSkidmarks::RegisterOne(uintptr id, const CVector &pos, float fwdX, float fwdY, eSkidmarkType type, bool *isBloody)','Skidmarks_RegisterOneType(uintptr_t id, const void *pos, float fwdX, float fwdY, eSkidmarkType type, bool *isBloody)')
    source=source.replace('RegisterOne(id, pos, fwdX, fwdY, type, isBloody);','Skidmarks_RegisterOneType(id, pos, fwdX, fwdY, type, isBloody);')
    source=re.sub(r'\bCSkidmarks::(\w+)\(',r'Skidmarks_\1(',source)
    source=source.replace('CTxdStore::PushCurrentTxd()','Skidmarks_BackendTxdPush()').replace('CTxdStore::FindTxdSlot(','Skidmarks_BackendTxdFind(').replace('CTxdStore::SetCurrentTxd(','Skidmarks_BackendTxdSet(').replace('CTxdStore::PopCurrentTxd()','Skidmarks_BackendTxdPop()')
    source=source.replace('RwTextureRead(','Skidmarks_BackendTextureRead(').replace('RwTextureDestroy(','Skidmarks_BackendTextureDestroy(').replace('RwTextureGetRaster(','Skidmarks_BackendRaster(')
    source=source.replace('uint32 ','uint32_t ').replace('CReplay::IsPlayingBack()','Skidmarks_BackendPlayingBack()')
    source=source.replace('CRGBA color(0, 0, 0, 255);','SkidmarksColor color = {0, 0, 0, 255};')
    source=re.sub(r'CRGBA\((\d+), (\d+), (\d+), (\d+)\)',r'(SkidmarksColor){\1, \2, \3, \4}',source)
    source=source.replace('CVector2D fwd(fwdX, fwdY);','SkidmarksVector2 fwd = {fwdX, fwdY};')
    source=source.replace('CVector2D right(','SkidmarksVector2 right = {').replace('aSkidmarks[i].m_pos[aSkidmarks[i].m_last].x);','aSkidmarks[i].m_pos[aSkidmarks[i].m_last].x};')
    source=source.replace('right.Normalise();','Skidmarks_Normalise(&right);').replace('fwd.Normalise();','Skidmarks_Normalise(&fwd);').replace('DotProduct2D(','Skidmarks_Dot2(').replace('Abs(','Skidmarks_Abs(')
    source=source.replace('CVector p1','SkidmarksVector p1').replace('CVector p2','SkidmarksVector p2')
    source=source.replace('= pos;','= Skidmarks_BackendReadPosition(pos);')
    for key,name in [('ZWRITEENABLE','ZWrite'),('VERTEXALPHAENABLE','VertexAlpha')]:
        for old,value in [('FALSE','false'),('TRUE','true')]:source=source.replace('RwRenderStateSet(rwRENDERSTATE'+key+', (void*)'+old+');','Skidmarks_Backend'+name+'('+value+');')
    source=source.replace('RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)TRUE);','Skidmarks_BackendZTest();')
    for key,name in [('SRCBLEND','SrcBlend'),('DESTBLEND','DestBlend')]:
        value='rwBLENDSRCALPHA' if key=='SRCBLEND' else 'rwBLENDINVSRCALPHA'
        source=source.replace('RwRenderStateSet(rwRENDERSTATE'+key+', (void*)'+value+');','Skidmarks_Backend'+name+'();')
    source=source.replace('RwRenderStateSet(rwRENDERSTATETEXTURERASTER, Skidmarks_BackendRaster(gpSkidTex));','Skidmarks_BackendTextureRaster(Skidmarks_BackendRaster(gpSkidTex));')
    for operation in ['RGBA','Pos','U','V']:source=re.sub(r'RwIm3DVertexSet'+operation+r'\(&SkidmarkVertices\[([^\]]+)\],',r'Skidmarks_BackendSet'+operation+r'(\1,',source)
    source=source.replace('LittleTest()','Skidmarks_BackendLittleTest()').replace('RwIm3DTransform(SkidmarkVertices, 2*(aSkidmarks[i].m_last+1), nil, rwIM3D_VERTEXUV)','Skidmarks_BackendTransform(2*(aSkidmarks[i].m_last+1))')
    source=source.replace('RwIm3DRenderIndexedPrimitive(rwPRIMTYPETRILIST, SkidmarkIndexList, 6*aSkidmarks[i].m_last)','Skidmarks_BackendRenderIndexed(SkidmarkIndexList, 6*aSkidmarks[i].m_last)').replace('RwIm3DEnd()','Skidmarks_BackendEnd()')
    source=source.replace('PUSH_RENDERGROUP(','Skidmarks_BackendPush(').replace('POP_RENDERGROUP()','Skidmarks_BackendPop()').replace('nil','NULL')
    source=re.sub(r'\t*//[+-] rouz edit \(ChatGPT\)\n','',source)
    comments={'Init':'Preserve texture acquisition, partial record initialization and the entire index table','Shutdown':'Destroy then clear the original texture handle','Clear':'Clear only the original state and updated flags','Update':'Preserve unsigned fade deadlines, strict expiry and updated-flag clearing','Render':'Retain unsigned fade math and every original vertex and render operation','RegisterOneFlags':'Preserve blood priority and lazy muddy-pointer reads before forwarding','RegisterOneType':'Preserve replay rejection, borrowed position timing and original continuation rules'}
    for name,comment in comments.items():source=re.sub(r'(Skidmarks_'+name+r'\([^\n]*\)\n\{)',r'\1\n\t// '+comment,source)
    source=re.sub(r'^(\t+)([^\n]*\{\n)',r'\1\2\1\t// Retain the original skidmark traversal and write order\n',source,flags=re.M)
    assert not re.search(r'\b(?:CVector|CVector2D|CRGBA|CSkidmarks|RwIm3D|SkidmarkVertices)\b',re.sub(r'"[^"\n]*"','',source))
    return '//+ rouz edit (ChatGPT)\n#include "Skidmarks_backend.h"\n#include "Timer.h"\n#include <math.h>\n#include <stddef.h>\n\n'+HELPERS+source.rstrip()+'\n//- rouz edit (ChatGPT)\n'

def project(source):
    # Register C11 algorithms and distinct objects beside the typed C++ readers
    if '..\\src\\renderer\\Skidmarks.c"' not in source:
        entry='<ClCompile Include="..\\src\\renderer\\Skidmarks.cpp" />';addition='<ClCompile Include="..\\src\\renderer\\Skidmarks.c">\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>'
        source=source.replace(entry,entry+'\n    '+addition)
        entry='<ClCompile Include="..\\src\\renderer\\Skidmarks.cpp">\n      <Filter>renderer</Filter>\n    </ClCompile>';source=source.replace(entry,entry+'\n    '+entry.replace('Skidmarks.cpp','Skidmarks.c'))
    if '..\\src\\renderer\\Skidmarks_backend.h"' not in source:
        for entry in ['<ClInclude Include="..\\src\\renderer\\Skidmarks.h" />','<ClInclude Include="..\\src\\renderer\\Skidmarks.h">\n      <Filter>renderer</Filter>\n    </ClInclude>']:source=source.replace(entry,entry+'\n    '+entry.replace('Skidmarks.h','Skidmarks_backend.h'))
    return rubbish.project(objects.project(source))
