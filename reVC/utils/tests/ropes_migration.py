"""Convert complete rope physics, visibility, rendering and SWAT orchestration."""
import re
import object_names_migration as objects
import skidmarks_migration as skidmarks

HEADER='''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"

typedef struct RopesVector {
	float x, y, z;
} RopesVector;
typedef struct Rope {
	bool m_bActive;
	bool m_bWasRegistered;
	bool m_unk;
	uintptr_t m_id;
	uint32_t m_updateTimer;
	RopesVector m_pos[32];
	RopesVector m_speed[32];
} Rope;
#ifdef __cplusplus
extern "C" {
#endif
void Rope_Update(Rope *rope);
void Rope_Render(Rope *rope);
void Ropes_Init(void);
void Ropes_Update(void);
void Ropes_Render(void);
bool Ropes_RegisterRope(uintptr_t id, RopesVector pos, bool setUpdateTimer);
void Ropes_SetSpeedOfTopNode(uintptr_t id, RopesVector speed);
bool Ropes_FindCoorsAlongRope(uintptr_t id, float t, RopesVector *coors);
bool Ropes_FindCoorsAlongRopeToVector(uintptr_t id, float t, void *coors);
bool Ropes_CreateRopeWithSwatComingDown(RopesVector pos);
extern uint16_t RopeIndices[64];
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
BACKEND_HEADER='''//+ rouz edit (ChatGPT)
#pragma once
#include "Ropes.h"
#ifdef __cplusplus
class CVector;
class CCopPed;
RopesVector Ropes_BackendCopyVector(const CVector &vector);
extern "C" {
#else
typedef struct CCopPed CCopPed;
#endif
void Ropes_BackendStoreVector(void *destination, RopesVector vector);
bool Ropes_BackendEnvMap(void);
bool Ropes_BackendSphereVisible(RopesVector center, float radius);
void Ropes_BackendPush(const char *name);
void Ropes_BackendPop(void);
void Ropes_BackendSetRGBA(int index, int red, int green, int blue, int alpha);
void Ropes_BackendSetPos(int index, float x, float y, float z);
void Ropes_BackendVertexAlpha(void);
void Ropes_BackendSrcBlend(void);
void Ropes_BackendDestBlend(void);
void Ropes_BackendTextureRaster(void);
bool Ropes_BackendTransform(int count);
void Ropes_BackendRenderIndexed(bool lineList, uint16_t *indices, int count);
void Ropes_BackendEnd(void);
bool Ropes_BackendSwatLoaded(void);
CCopPed *Ropes_BackendAddSwat(RopesVector pos);
void Ropes_BackendDisableCollision(CCopPed *ped);
void Ropes_BackendRopeSentinel(CCopPed *ped);
void Ropes_BackendRopeID(CCopPed *ped, uint32_t id);
void Ropes_BackendBlendSwat(CCopPed *ped);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
HOST='''//+ rouz edit (ChatGPT)
#include "common.h"
#include "main.h"
#include "ModelIndices.h"
#include "Streaming.h"
#include "CopPed.h"
#include "Population.h"
#include "RenderBuffer.h"
#include "Camera.h"
#include "Ropes_backend.h"
#if defined(LIBRW) && defined(EXTENDED_PIPELINES)
#include "custompipes.h"
#endif

RopesVector Ropes_BackendCopyVector(const CVector &vector)
{
	// Copy the original typed value including temporary caller expressions
	RopesVector result = {vector.x, vector.y, vector.z};
	return result;
}
void Ropes_BackendStoreVector(void *destination, RopesVector vector)
{
	// Write the original typed output object only after a successful lookup
	*(CVector *)destination = CVector(vector.x, vector.y, vector.z);
}
bool Ropes_BackendEnvMap(void)
{
	// Read the live environment-pass state through its original owner
#if defined(LIBRW) && defined(EXTENDED_PIPELINES)
	return CustomPipes::bRenderingEnvMap;
#else
	return false;
#endif
}
bool Ropes_BackendSphereVisible(RopesVector center, float radius)
{
	// Query the actual camera with the original typed sphere value
	return TheCamera.IsSphereVisible(CVector(center.x, center.y, center.z), radius);
}
void Ropes_BackendPush(const char *name)
{
	// Preserve the original debug render-group boundary
	PUSH_RENDERGROUP(name);
}
void Ropes_BackendPop(void)
{
	// Close the original render group when enabled
	POP_RENDERGROUP();
}
void Ropes_BackendSetRGBA(int index, int red, int green, int blue, int alpha)
{
	// Write the existing shared render buffer through the typed color setter
	RwIm3DVertexSetRGBA(&TempBufferRenderVertices[index], red, green, blue, alpha);
}
void Ropes_BackendSetPos(int index, float x, float y, float z)
{
	// Write the same shared render buffer through the typed position setter
	RwIm3DVertexSetPos(&TempBufferRenderVertices[index], x, y, z);
}
void Ropes_BackendVertexAlpha(void)
{
	// Enable the original vertex-alpha state
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
}
void Ropes_BackendSrcBlend(void)
{
	// Select the original source blend mode
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
}
void Ropes_BackendDestBlend(void)
{
	// Select the original destination blend mode
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
}
void Ropes_BackendTextureRaster(void)
{
	// Clear the original texture-raster state
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, nil);
}
bool Ropes_BackendTransform(int count)
{
	// Retain the original shared vertices, null matrix and XYZ/RGBA flags
	return RwIm3DTransform(TempBufferRenderVertices, count, nil, rwIM3D_VERTEXXYZ|rwIM3D_VERTEXRGBA) != nil;
}
void Ropes_BackendRenderIndexed(bool lineList, uint16_t *indices, int count)
{
	// Map the C algorithm's original primitive choice to the actual typed enum
	RwIm3DRenderIndexedPrimitive(lineList ? rwPRIMTYPELINELIST : rwPRIMTYPEPOLYLINE, indices, count);
}
void Ropes_BackendEnd(void)
{
	// End the original successful immediate-mode draw
	RwIm3DEnd();
}
bool Ropes_BackendSwatLoaded(void)
{
	// Query the original live SWAT model state
	return CStreaming::HasModelLoaded(MI_SWAT);
}
CCopPed *Ropes_BackendAddSwat(RopesVector pos)
{
	// Retain the original population call and actual derived-pointer conversion
	return (CCopPed *)CPopulation::AddPed(PEDTYPE_COP, COP_HELI_SWAT, CVector(pos.x, pos.y, pos.z));
}
void Ropes_BackendDisableCollision(CCopPed *ped)
{
	// Write the original collision bit through its typed owner
	ped->bUsesCollision = false;
}
void Ropes_BackendRopeSentinel(CCopPed *ped)
{
	// Preserve the original entity-pointer sentinel exactly
	ped->m_pRopeEntity = (CEntity *)1;
}
void Ropes_BackendRopeID(CCopPed *ped, uint32_t id)
{
	// Assign the original wrapped 32-bit counter result to its pointer-width field
	ped->m_nRopeID = id;
}
void Ropes_BackendBlendSwat(CCopPed *ped)
{
	// Enter the original typed clump access and animation blend operation
	CAnimManager::BlendAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_STD_ABSEIL, 4.0f);
}
//- rouz edit (ChatGPT)
'''
HELPERS='''#define ARRAY_SIZE(array) (sizeof(array)/sizeof((array)[0]))
#define ROPES_CLAMP(v, low, high) ((v)<(low) ? (low) : (v)>(high) ? (high) : (v))
static RopesVector Ropes_Add(RopesVector left, RopesVector right)
{
	// Preserve the original componentwise vector addition
	RopesVector result = {left.x+right.x, left.y+right.y, left.z+right.z};
	return result;
}
static RopesVector Ropes_Subtract(RopesVector left, RopesVector right)
{
	// Preserve the original componentwise vector subtraction
	RopesVector result = {left.x-right.x, left.y-right.y, left.z-right.z};
	return result;
}
static RopesVector Ropes_Scale(RopesVector vector, float scalar)
{
	// Multiply components in the same operand order as vector times scalar
	RopesVector result = {vector.x*scalar, vector.y*scalar, vector.z*scalar};
	return result;
}
static RopesVector Ropes_LeftScale(float scalar, RopesVector vector)
{
	// Multiply components in the same operand order as scalar times vector
	RopesVector result = {scalar*vector.x, scalar*vector.y, scalar*vector.z};
	return result;
}
static RopesVector Ropes_Divide(RopesVector vector, float scalar)
{
	// Divide every original component without folding the timestep expression
	RopesVector result = {vector.x/scalar, vector.y/scalar, vector.z/scalar};
	return result;
}
static float Ropes_Magnitude(RopesVector vector)
{
	// Use the probed original float sum and sqrtf wrapper
	return sqrtf(vector.x*vector.x + vector.y*vector.y + vector.z*vector.z);
}

'''
ADAPTER='''bool Ropes_FindCoorsAlongRopeToVector(uintptr_t id, float t, void *coors)
{
	// Preserve the legacy output object's contents on every failed lookup
	RopesVector result;
	if (!Ropes_FindCoorsAlongRope(id, t, &result))
		return false;
	Ropes_BackendStoreVector(coors, result);
	return true;
}
'''

def calls(source):
    # Keep legacy vector temporaries typed while removing all former class calls
    if 'class CRope\n' in source:return HEADER
    if 'CRope CRopes::aRopes[8];' in source:return HOST
    changed=bool(re.search(r'\bCRopes::\w+\(',source))
    vectors=bool(re.search(r'CRopes::(?:RegisterRope|SetSpeedOfTopNode|CreateRopeWithSwatComingDown)\(',source))
    if vectors and '#include "Ropes_backend.h"' not in source:source=source.replace('#include "Ropes.h"','#include "Ropes.h"\n#include "Ropes_backend.h"')
    source=source.replace('CRopes::RegisterRope((uintptr)this + i, GetMatrix()*FindSwatPositionRelativeToHeli(i), false)','Ropes_RegisterRope((uintptr)this + i, Ropes_BackendCopyVector(GetMatrix()*FindSwatPositionRelativeToHeli(i)), false)')
    source=source.replace('CRopes::SetSpeedOfTopNode((uintptr)this + i, speed)','Ropes_SetSpeedOfTopNode((uintptr)this + i, Ropes_BackendCopyVector(speed))')
    source=source.replace('CRopes::RegisterRope((uintptr)this + m_numSwat-1, pos, false)','Ropes_RegisterRope((uintptr)this + m_numSwat-1, Ropes_BackendCopyVector(pos), false)')
    source=source.replace('CRopes::CreateRopeWithSwatComingDown(*(CVector*)&ScriptParams[0])','Ropes_CreateRopeWithSwatComingDown(Ropes_BackendCopyVector(*(CVector*)&ScriptParams[0]))')
    source=source.replace('CRopes::FindCoorsAlongRope(','Ropes_FindCoorsAlongRopeToVector(')
    source=re.sub(r'\bCRopes::(\w+)\(',r'Ropes_\1(',source)
    if changed:
        lines=[]
        for line in source.splitlines(keepends=True):
            if re.search(r'\bRopes_(?:Init|Update|Render|RegisterRope|SetSpeedOfTopNode|FindCoorsAlongRopeToVector|CreateRopeWithSwatComingDown)\(',line):
                indent=re.match(r'\s*',line)[0];comment=indent+'// Enter the C rope algorithm with the original owner expressions\n'
                if not lines or lines[-1]!=comment:lines.append(comment)
            lines.append(line)
        source=''.join(lines)
    return skidmarks.calls(source)

def kernel(source):
    # Relocate all nine bodies and the complete conditional visibility decision
    source=source[source.index('CRope CRopes::aRopes'):]
    source=source.replace('CRope CRopes::aRopes[8];','static Rope aRopes[8];').replace('RwImVertexIndex RopeIndices','uint16_t RopeIndices')
    source=source.replace('IsRopeVisibleToActiveCamera(const CVector &center, float radius)','IsRopeVisibleToActiveCamera(RopesVector center, float radius)')
    source=source.replace('CustomPipes::bRenderingEnvMap','Ropes_BackendEnvMap()').replace('TheCamera.IsSphereVisible(center, radius)','Ropes_BackendSphereVisible(center, radius)')
    source=source.replace('CRope::Update(void)','Rope_Update(Rope *rope)').replace('CRope::Render(void)','Rope_Render(Rope *rope)')
    first=source.index('Rope_Update(Rope *rope)');last=source.index('CRopes::Init(void)');body=source[first:last]
    body=re.sub(r'(?<![\w.>])\b(m_bActive|m_bWasRegistered|m_updateTimer|m_pos|m_speed)\b',r'rope->\1',body)
    body=body.replace('Pow(','powf(')
    body=body.replace('rope->m_pos[0] += rope->m_speed[0]*(*Timer_GetTimeStep());','rope->m_pos[0] = Ropes_Add(rope->m_pos[0], Ropes_Scale(rope->m_speed[0], (*Timer_GetTimeStep())));')
    body=body.replace('rope->m_pos[i] += rope->m_speed[i]*step*(*Timer_GetTimeStep());','rope->m_pos[i] = Ropes_Add(rope->m_pos[i], Ropes_Scale(Ropes_Scale(rope->m_speed[i], step), (*Timer_GetTimeStep())));')
    body=body.replace('rope->m_pos[i] - rope->m_pos[i-1]','Ropes_Subtract(rope->m_pos[i], rope->m_pos[i-1])')
    body=body.replace('rope->m_pos[i-1] + (0.625f/dist.Magnitude())*dist','Ropes_Add(rope->m_pos[i-1], Ropes_LeftScale(0.625f/Ropes_Magnitude(dist), dist))')
    body=body.replace('(rope->m_pos[i] - prevPos)/(*Timer_GetTimeStep())','Ropes_Divide(Ropes_Subtract(rope->m_pos[i], prevPos), (*Timer_GetTimeStep()))')
    body=re.sub(r'RwIm3DVertexSetRGBA\(&TempBufferRenderVertices\[i\],',r'Ropes_BackendSetRGBA(i,',body)
    body=re.sub(r'RwIm3DVertexSetPos\(&TempBufferRenderVertices\[i\],',r'Ropes_BackendSetPos(i,',body)
    body=body.replace('RwIm3DTransform(TempBufferRenderVertices, ARRAY_SIZE(rope->m_pos), nil, rwIM3D_VERTEXXYZ|rwIM3D_VERTEXRGBA)','Ropes_BackendTransform((int)ARRAY_SIZE(rope->m_pos))')
    body=body.replace('RwIm3DRenderIndexedPrimitive(rwPRIMTYPELINELIST, RopeIndices, 2*(ARRAY_SIZE(rope->m_pos)-1))','Ropes_BackendRenderIndexed(true, RopeIndices, (int)(2*(ARRAY_SIZE(rope->m_pos)-1)))')
    body=body.replace('RwIm3DRenderIndexedPrimitive(rwPRIMTYPEPOLYLINE, RopeIndices, 2*(ARRAY_SIZE(rope->m_pos)-1))','Ropes_BackendRenderIndexed(false, RopeIndices, (int)(2*(ARRAY_SIZE(rope->m_pos)-1)))')
    source=source[:first]+body+source[last:]
    source=re.sub(r'\bCRopes::(\w+)\(',r'Ropes_\1(',source)
    source=source.replace('aRopes[i].Update()','Rope_Update(&aRopes[i])').replace('aRopes[i].Render()','Rope_Render(&aRopes[i])')
    source=source.replace('uintptr id','uintptr_t id').replace('uint32 ropeId','uint32_t ropeId').replace('CVector','RopesVector')
    source=source.replace('RopesVector(0.0f, 0.0f, 0.0f)','(RopesVector){0.0f, 0.0f, 0.0f}')
    source=source.replace('aRopes[i].m_pos[j-1] + RopesVector(0.0f, 0.0f, 0.625f)','Ropes_Add(aRopes[i].m_pos[j-1], (RopesVector){0.0f, 0.0f, 0.625f})')
    source=source.replace('aRopes[i].m_pos[j-1] - RopesVector(0.0f, 0.0f, 0.625f)','Ropes_Subtract(aRopes[i].m_pos[j-1], (RopesVector){0.0f, 0.0f, 0.625f})')
    source=source.replace('Clamp(','ROPES_CLAMP(').replace('(1.0f-f)*aRopes[i].m_pos[j] + f*aRopes[i].m_pos[j+1]','Ropes_Add(Ropes_LeftScale(1.0f-f, aRopes[i].m_pos[j]), Ropes_LeftScale(f, aRopes[i].m_pos[j+1]))')
    source=source.replace('CStreaming::HasModelLoaded(MI_SWAT)','Ropes_BackendSwatLoaded()').replace('!RegisterRope(ropeId+100, pos, true)','!Ropes_RegisterRope(ropeId+100, pos, true)')
    source=source.replace('CCopPed *swat = (CCopPed*)CPopulation::AddPed(PEDTYPE_COP, COP_HELI_SWAT, pos);','CCopPed *swat = Ropes_BackendAddSwat(pos);')
    source=source.replace('swat->bUsesCollision = false;','Ropes_BackendDisableCollision(swat);').replace('swat->m_pRopeEntity = (CEntity*)1;','Ropes_BackendRopeSentinel(swat);').replace('swat->m_nRopeID = 100 + ropeId;','Ropes_BackendRopeID(swat, 100 + ropeId);')
    source=source.replace('CAnimManager::BlendAnimation(swat->GetClump(), ASSOCGRP_STD, ANIM_STD_ABSEIL, 4.0f);','Ropes_BackendBlendSwat(swat);')
    for name,expression in [('VertexAlpha','rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE'),('SrcBlend','rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA'),('DestBlend','rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA'),('TextureRaster','rwRENDERSTATETEXTURERASTER, nil')]:source=source.replace('RwRenderStateSet('+expression+');','Ropes_Backend'+name+'();')
    source=source.replace('PUSH_RENDERGROUP(','Ropes_BackendPush(').replace('POP_RENDERGROUP()','Ropes_BackendPop()').replace('RwIm3DEnd()','Ropes_BackendEnd()')
    source=re.sub(r'\t*//[+-] rouz edit \(ChatGPT\)\n','',source)
    comments={'Rope_Update':'Preserve the original live timer reads and sequential node physics','Rope_Render':'Retain sphere visibility and the shared vertex and index draw protocol','Ropes_Init':'Clear only the original active flags','Ropes_Update':'Update active records in the original array order','Ropes_Render':'Render active records inside the original debug group','Ropes_RegisterRope':'Retain first-match updates and first-free record initialization','Ropes_SetSpeedOfTopNode':'Write only the first matching active rope top speed','Ropes_FindCoorsAlongRope':'Preserve clamp, interpolation order and failure output behavior','Ropes_CreateRopeWithSwatComingDown':'Preserve lazy loading, rope allocation and ordered SWAT setup'}
    for name,comment in comments.items():source=re.sub(r'('+name+r'\([^\n]*\)\n\{)',r'\1\n\t// '+comment,source)
    source=re.sub(r'^(\t+)([^\n]*\{\n)',r'\1\2\1\t// Preserve the original rope traversal and write order\n',source,flags=re.M)
    assert 'CVector' not in source and 'RwIm3D' not in source and 'TempBufferRenderVertices' not in source and '->m_pos[i] +=' not in source
    return '//+ rouz edit (ChatGPT)\n#include "Ropes_backend.h"\n#include "Timer.h"\n#include <math.h>\n\n'+HELPERS+source.rstrip()+'\n\n'+ADAPTER+'//- rouz edit (ChatGPT)\n'

def project(source):
    # Add the complete C11 implementation before disambiguating its paired objects
    if '..\\src\\core\\Ropes.c"' not in source:
        entry='<ClCompile Include="..\\src\\core\\Ropes.cpp" />';addition='<ClCompile Include="..\\src\\core\\Ropes.c">\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>'
        source=source.replace(entry,entry+'\n    '+addition)
        entry='<ClCompile Include="..\\src\\core\\Ropes.cpp">\n      <Filter>core</Filter>\n    </ClCompile>';source=source.replace(entry,entry+'\n    '+entry.replace('Ropes.cpp','Ropes.c'))
    if '..\\src\\core\\Ropes_backend.h"' not in source:
        for entry in ['<ClInclude Include="..\\src\\core\\Ropes.h" />','<ClInclude Include="..\\src\\core\\Ropes.h">\n      <Filter>core</Filter>\n    </ClInclude>']:source=source.replace(entry,entry+'\n    '+entry.replace('Ropes.h','Ropes_backend.h'))
    return skidmarks.project(objects.project(source))
