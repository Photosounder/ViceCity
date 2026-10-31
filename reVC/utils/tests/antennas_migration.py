"""Move both antenna classes' complete algorithms and preserve typed RW storage."""
import re
import object_names_migration as objects
import ropes_migration as ropes

HEADER='''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"

typedef struct AntennasVector {
	float x, y, z;
} AntennasVector;
typedef struct Antenna {
	bool active;
	bool updatedLastFrame;
	uint32_t id;
	float segmentLength;
	AntennasVector pos[6];
	AntennasVector speed[6];
} Antenna;
#ifdef __cplusplus
extern "C" {
#endif
void Antennas_Init(void);
void Antennas_Update(void);
void Antennas_RegisterOne(uint32_t id, AntennasVector dir, AntennasVector position, float length);
void Antennas_Render(void);
void Antenna_Update(Antenna *antenna, AntennasVector dir, AntennasVector basepos);
#ifdef REVC_SOFTWARE_POLYGONS
void Antennas_RegisterOneForEnvMap(uint32_t id, AntennasVector dir, AntennasVector position, float length);
void Antennas_RenderForEnvMap(void);
#endif
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''

BACKEND_HEADER='''//+ rouz edit (ChatGPT)
#pragma once
#include "Antennas.h"
#ifdef __cplusplus
extern "C" {
#endif
AntennasVector Antennas_BackendVector(const void *vector);
void Antennas_BackendPush(const char *name);
void Antennas_BackendPop(void);
void Antennas_BackendZWrite(void);
void Antennas_BackendVertexAlpha(bool enabled);
void Antennas_BackendSrcBlend(void);
void Antennas_BackendDestBlend(void);
void Antennas_BackendTextureRaster(void);
void Antennas_BackendSetRGBA(int index, int red, int green, int blue, int alpha);
void Antennas_BackendSetPos(int index, float x, float y, float z);
bool Antennas_BackendTransform(void);
void Antennas_BackendRenderLine(void);
void Antennas_BackendEnd(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''

HOST='''//+ rouz edit (ChatGPT)
#include "common.h"
#include "main.h"
#include "Antennas_backend.h"

static RwIm3DVertex vertexbufferA[2];

AntennasVector Antennas_BackendVector(const void *vector)
{
	// Read the actual vector components without assuming its C++ layout
	const CVector *value = (const CVector *)vector;
	AntennasVector result = {value->x, value->y, value->z};
	return result;
}
void Antennas_BackendPush(const char *name)
{
	// Preserve the original debug group boundary and FINAL configuration
	PUSH_RENDERGROUP(name);
}
void Antennas_BackendPop(void)
{
	// Close the original debug group when enabled
	POP_RENDERGROUP();
}
void Antennas_BackendZWrite(void)
{
	// Set the original typed depth-write state
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)TRUE);
}
void Antennas_BackendVertexAlpha(bool enabled)
{
	// Set the original typed vertex-alpha state
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)(uintptr_t)enabled);
}
void Antennas_BackendSrcBlend(void)
{
	// Set the original typed source blend mode
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
}
void Antennas_BackendDestBlend(void)
{
	// Set the original typed destination blend mode
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
}
void Antennas_BackendTextureRaster(void)
{
	// Clear the original typed texture-raster state
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, nil);
}
void Antennas_BackendSetRGBA(int index, int red, int green, int blue, int alpha)
{
	// Write the original private vertex buffer through its typed color setter
	RwIm3DVertexSetRGBA(&vertexbufferA[index], red, green, blue, alpha);
}
void Antennas_BackendSetPos(int index, float x, float y, float z)
{
	// Write the same vertex storage through its typed position setter
	RwIm3DVertexSetPos(&vertexbufferA[index], x, y, z);
}
bool Antennas_BackendTransform(void)
{
	// Retain the original vertex count, null matrix and zero transform flags
	return RwIm3DTransform(vertexbufferA, 2, nil, 0) != nil;
}
void Antennas_BackendRenderLine(void)
{
	// Render the same two indices after a successful transform
	RwIm3DRenderLine(0, 1);
}
void Antennas_BackendEnd(void)
{
	// Finish the original successful immediate-mode draw
	RwIm3DEnd();
}
//- rouz edit (ChatGPT)
'''

HELPERS='''static AntennasVector Antennas_Add(AntennasVector left, AntennasVector right)
{
	// Add each original vector component without regrouping expressions
	AntennasVector result = {left.x+right.x, left.y+right.y, left.z+right.z};
	return result;
}
static AntennasVector Antennas_Subtract(AntennasVector left, AntennasVector right)
{
	// Subtract each original vector component
	AntennasVector result = {left.x-right.x, left.y-right.y, left.z-right.z};
	return result;
}
static AntennasVector Antennas_Scale(AntennasVector vector, float scalar)
{
	// Preserve the original componentwise scalar multiplication
	AntennasVector result = {vector.x*scalar, vector.y*scalar, vector.z*scalar};
	return result;
}
static AntennasVector Antennas_Divide(AntennasVector vector, float scalar)
{
	// Preserve the original componentwise division before adding the result
	AntennasVector result = {vector.x/scalar, vector.y/scalar, vector.z/scalar};
	return result;
}
static void Antennas_Normalise(AntennasVector *vector)
{
	// Retain the original float squared length and reciprocal square root
	float sq = vector->x*vector->x + vector->y*vector->y + vector->z*vector->z;
	if (sq > 0.0f) {
		// Scale all three components in the original order
		float invsqrt = 1.0f/sqrtf(sq);
		vector->x *= invsqrt;
		vector->y *= invsqrt;
		vector->z *= invsqrt;
	} else
		vector->x = 1.0f;
}

'''

def calls(source):
    # Replace both removed classes and keep every original caller expression
    if 'class CAntenna\n' in source:return HEADER
    if 'CAntenna CAntennas::aAntennas[NUMANTENNAS];' in source:return HOST
    changed=bool(re.search(r'\bCAntennas::\w+\(',source))
    has_vectors=bool(re.search(r'CAntennas::RegisterOne(?:ForEnvMap)?\(',source))
    if has_vectors and '#include "Antennas_backend.h"' not in source:source=source.replace('#include "Antennas.h"','#include "Antennas.h"\n#include "Antennas_backend.h"')
    source=source.replace('CAntennas::RegisterOne((uintptr)this, GetUp(), pos,','Antennas_RegisterOne((uintptr)this, Antennas_BackendVector(&GetUp()), Antennas_BackendVector(&pos),')
    source=source.replace('CAntennas::RegisterOneForEnvMap((uintptr)vehicle, vehicle->GetUp(), antennaPosition,','Antennas_RegisterOneForEnvMap((uintptr)vehicle, Antennas_BackendVector(&vehicle->GetUp()), Antennas_BackendVector(&antennaPosition),')
    source=re.sub(r'\bCAntennas::(\w+)\(',r'Antennas_\1(',source)
    if changed:
        comments={'Init':'Initialize the C antenna pool','Update':'Advance the C antenna frame state','RegisterOne':'Register the C antenna with typed vector copies','RegisterOneForEnvMap':'Queue the C reflection antenna with typed vector copies','Render':'Render the C antenna pool','RenderForEnvMap':'Render and consume the C reflection antenna queue'}
        lines=[]
        for line in source.splitlines(keepends=True):
            match=re.match(r'(\s*)Antennas_(\w+)\(',line)
            if match and match[2] in comments:
                comment=match[1]+'// '+comments[match[2]]+'\n'
                if not lines or lines[-1]!=comment:lines.append(comment)
            lines.append(line)
        source=''.join(lines)
    return ropes.calls(source)

def kernel(source):
    # Preserve all original bodies while making vector operations explicit in C
    source=source[source.index('CAntenna CAntennas::aAntennas'):]
    source=source.replace('CAntenna CAntennas::aAntennas[NUMANTENNAS];','static Antenna aAntennas[NUMANTENNAS];')
    source=re.sub(r'\bCAntenna\b','Antenna',source).replace('static RwIm3DVertex vertexbufferA[2];','')
    source=re.sub(r'\bCAntennas::(\w+)\(',r'Antennas_\1(',source)
    source=source.replace('Antenna::Update(CVector dir, CVector basepos)','Antenna_Update(Antenna *antenna, CVector dir, CVector basepos)')
    source=source.replace('uint32 id','uint32_t id').replace('CVector','AntennasVector')
    source=source.replace('aAntennas[i].Update(dir, position);','Antenna_Update(&aAntennas[i], dir, position);')
    source=source.replace('position + dir*j*aAntennas[i].segmentLength','Antennas_Add(position, Antennas_Scale(Antennas_Scale(dir, (float)j), aAntennas[i].segmentLength))')
    source=source.replace('position + dir*i*antenna->segmentLength','Antennas_Add(position, Antennas_Scale(Antennas_Scale(dir, (float)i), antenna->segmentLength))')
    source=source.replace('AntennasVector(0.0f, 0.0f, 0.0f)','(AntennasVector){0.0f, 0.0f, 0.0f}')
    start=source.index('Antenna_Update(Antenna *antenna');prefix=source[:start];body=source[start:]
    body=re.sub(r'(?<![\w])\b(pos|speed|segmentLength)\b',r'antenna->\1',body)
    body=body.replace('basepos + dir*antenna->segmentLength','Antennas_Add(basepos, Antennas_Scale(dir, antenna->segmentLength))')
    body=body.replace('antenna->pos[i-1] - antenna->pos[i-2]','Antennas_Subtract(antenna->pos[i-1], antenna->pos[i-2])')
    body=body.replace('antenna->pos[i] - antenna->pos[i-1] +\t// drag along\n\t\t\tdir*0.1f +\t// also drag up a bit for stiffness\n\t\t\tantenna->speed[i];\t// and keep moving','Antennas_Add(Antennas_Add(Antennas_Subtract(antenna->pos[i], antenna->pos[i-1]), Antennas_Scale(dir, 0.1f)), antenna->speed[i]);')
    body=body.replace('newdir.Normalise();','Antennas_Normalise(&newdir);').replace('newdir *= antenna->segmentLength;','newdir = Antennas_Scale(newdir, antenna->segmentLength);')
    body=body.replace('antenna->pos[i-1] + (basedir + newdir)/2.0f','Antennas_Add(antenna->pos[i-1], Antennas_Divide(Antennas_Add(basedir, newdir), 2.0f))')
    body=body.replace('(newpos - antenna->pos[i])*0.9f','Antennas_Scale(Antennas_Subtract(newpos, antenna->pos[i]), 0.9f)')
    source=prefix+body
    for name,expression in [('ZWrite','rwRENDERSTATEZWRITEENABLE, (void*)TRUE'),('SrcBlend','rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA'),('DestBlend','rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA'),('TextureRaster','rwRENDERSTATETEXTURERASTER, nil')]:
        source=source.replace('RwRenderStateSet('+expression+');','Antennas_Backend'+name+'();')
    for original,value in [('TRUE','true'),('FALSE','false')]:source=source.replace('RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)'+original+');','Antennas_BackendVertexAlpha('+value+');')
    source=source.replace('PUSH_RENDERGROUP(','Antennas_BackendPush(').replace('POP_RENDERGROUP()','Antennas_BackendPop()')
    source=re.sub(r'RwIm3DVertexSetRGBA\(&vertexbufferA\[([01])\],',r'Antennas_BackendSetRGBA(\1,',source)
    source=re.sub(r'RwIm3DVertexSetPos\(&vertexbufferA\[([01])\],',r'Antennas_BackendSetPos(\1,',source)
    source=source.replace('RwIm3DTransform(vertexbufferA, 2, nil, 0)','Antennas_BackendTransform()').replace('RwIm3DRenderLine(0, 1)','Antennas_BackendRenderLine()').replace('RwIm3DEnd()','Antennas_BackendEnd()')
    source=re.sub(r'\t*//[+-] rouz edit \(ChatGPT\)\n','',source)
    comments={'Antennas_Init':'Clear only the original active and updated flags and auxiliary count','Antennas_Update':'Expire unregistered antennas before clearing every updated flag','Antennas_RegisterOne':'Find the original matching ID or first available record','Antennas_RegisterOneForEnvMap':'Retain main-pool and auxiliary-queue duplicate checks before allocating','Antennas_Render':'Preserve main-pool traversal and every render-state and vertex operation','Antennas_RenderForEnvMap':'Render and consume the original auxiliary queue','Antenna_Update':'Update the original spring chain sequentially with explicit vector operations'}
    for name,comment in comments.items():source=re.sub(r'('+name+r'\([^\n]*\)\n\{)',r'\1\n\t// '+comment,source)
    source=re.sub(r'^(\t+)([^\n]*\{\n)',r'\1\2\1\t// Retain the original antenna traversal and update order\n',source,flags=re.M)
    assert 'CVector' not in source and 'RwIm3D' not in source and 'vertexbufferA' not in source
    return '//+ rouz edit (ChatGPT)\n#include "Antennas_backend.h"\n#include <math.h>\n\n'+HELPERS+source.rstrip()+'\n//- rouz edit (ChatGPT)\n'

def project(source):
    # Compile both units with separate objects in the existing project and filters
    if '..\\src\\renderer\\Antennas.c"' not in source:
        entry='<ClCompile Include="..\\src\\renderer\\Antennas.cpp" />'
        addition='<ClCompile Include="..\\src\\renderer\\Antennas.c">\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>'
        source=source.replace(entry,entry+'\n    '+addition)
        entry='<ClCompile Include="..\\src\\renderer\\Antennas.cpp">\n      <Filter>renderer</Filter>\n    </ClCompile>'
        source=source.replace(entry,entry+'\n    '+entry.replace('Antennas.cpp','Antennas.c'))
    if '..\\src\\renderer\\Antennas_backend.h"' not in source:
        for entry in ['<ClInclude Include="..\\src\\renderer\\Antennas.h" />','<ClInclude Include="..\\src\\renderer\\Antennas.h">\n      <Filter>renderer</Filter>\n    </ClInclude>']:
            source=source.replace(entry,entry+'\n    '+entry.replace('Antennas.h','Antennas_backend.h'))
    return ropes.project(objects.project(source))
