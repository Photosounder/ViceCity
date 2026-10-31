"""Convert the complete line algorithm while retaining typed RenderWare storage."""
import re
import filemgr_migration as filemgr

HEADER='''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
void Lines_RenderLineWithClipping(float x1, float y1, float z1, float x2, float y2, float z2, uint32_t c1, uint32_t c2);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''

BACKEND_HEADER='''//+ rouz edit (ChatGPT)
#pragma once
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif
void *Lines_BackendVertices(void);
void Lines_BackendSetRGBA(void *vertices, int index, int red, int green, int blue, int alpha);
void Lines_BackendSetPos(void *vertices, int index, float x, float y, float z);
void Lines_BackendLittleTest(void);
bool Lines_BackendTransform(void *vertices);
void Lines_BackendRenderLine(void);
void Lines_BackendEnd(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''

HOST='''//+ rouz edit (ChatGPT)
#include "common.h"
#include "main.h"
#include "Lines_backend.h"

void *
Lines_BackendVertices(void)
{
	// Retain the original function-local typed vertex buffer and its lifetime
	static RwIm3DVertex v[2];
	return v;
}

void
Lines_BackendSetRGBA(void *vertices, int index, int red, int green, int blue, int alpha)
{
	// Write the original vertex color through RenderWare's typed setter
	RwIm3DVertexSetRGBA(&((RwIm3DVertex *)vertices)[index], red, green, blue, alpha);
}

void
Lines_BackendSetPos(void *vertices, int index, float x, float y, float z)
{
	// Write the original vertex position without assuming a backend layout
	RwIm3DVertexSetPos(&((RwIm3DVertex *)vertices)[index], x, y, z);
}

void
Lines_BackendLittleTest(void)
{
	// Preserve the original hook before each transform attempt
	LittleTest();
}

bool
Lines_BackendTransform(void *vertices)
{
	// Preserve both vertices, the null matrix and the original transform flags
	return RwIm3DTransform((RwIm3DVertex *)vertices, 2, nil, 0) != nil;
}

void
Lines_BackendRenderLine(void)
{
	// Render the same two vertex indices after a successful transform
	RwIm3DRenderLine(0, 1);
}

void
Lines_BackendEnd(void)
{
	// Finish the successful immediate-mode draw
	RwIm3DEnd();
}
//- rouz edit (ChatGPT)
'''

def calls(source):
    # Replace the class API without changing any caller argument expression
    if 'class CLines\n' in source:return HEADER
    source=re.sub(r'(?m)^(\s*?)CLines::RenderLineWithClipping\(',lambda m:m[1]+'// Draw the original endpoints and colors through the C line API\n'+m[1]+'Lines_RenderLineWithClipping(',source)
    return filemgr.calls(source)

def kernel(source):
    # Retain both complete branches, float squares, double sqrt and original statement order
    source=source[source.index('// This is super inefficient'):]
    source=source.replace('CLines::RenderLineWithClipping','Lines_RenderLineWithClipping').replace('uint32 c','uint32_t c')
    source=source.replace('static RwIm3DVertex v[2];','// Obtain the original persistent buffer before preparing either rendering path\n\tvoid *v = Lines_BackendVertices();')
    source=source.replace('RwRGBA','LinesRGBA').replace('sq(','Lines_Square(')
    source=re.sub(r'RwIm3DVertexSet(RGBA|Pos)\(&v\[([01])\],',r'Lines_BackendSet\1(v, \2,',source)
    source=source.replace('LittleTest();','Lines_BackendLittleTest();').replace('RwIm3DTransform(v, 2, nil, 0)','Lines_BackendTransform(v)').replace('RwIm3DRenderLine(0, 1);','Lines_BackendRenderLine();').replace('RwIm3DEnd();','Lines_BackendEnd();')
    source=source.replace('\tint i;','\t// Preserve the original segment count and interpolation arithmetic\n\tint i;').replace('\tLinesRGBA col1;','\t// Decode the packed endpoint colors into the original byte channels\n\tLinesRGBA col1;')
    source=source.replace('\tfloat dx =','\t// Interpolate each segment in the original vertex-update order\n\tfloat dx =').replace('\t\tLines_BackendSetRGBA(v, 0,','\t\t// Prepare the two endpoint colors and positions for this segment\n\t\tLines_BackendSetRGBA(v, 0,').replace('\n\tLines_BackendSetRGBA(v, 0,','\n\t// Prepare the original two endpoints without segmenting the line\n\tLines_BackendSetRGBA(v, 0,')
    source=source.replace('\t\tLines_BackendLittleTest();','\t\t// Run the hook and draw only when transformation succeeds\n\t\tLines_BackendLittleTest();').replace('\n\tLines_BackendLittleTest();','\n\t// Run the hook and draw only when transformation succeeds\n\tLines_BackendLittleTest();')
    prefix='''//+ rouz edit (ChatGPT)
#include "Lines.h"
#include "Lines_backend.h"
#include <math.h>
#include "config.h"

typedef struct LinesRGBA { uint8_t red, green, blue, alpha; } LinesRGBA;

#ifdef THIS_IS_STUPID
static float
Lines_Square(float x)
{
	// Preserve the original float square helper
	return x*x;
}
#endif

'''
    assert 'RwIm3D' not in source and 'CLines' not in source
    return prefix+source.rstrip()+'\n//- rouz edit (ChatGPT)\n'

def project(source):
    # Register the actual C algorithm beside the remaining typed C++ backend
    entry='<ClCompile Include="..\\src\\renderer\\Lines.cpp" />'
    if '<CompileAs>' in source:
        addition='<ClCompile Include="..\\src\\renderer\\Lines.c">\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>'
    else:addition='<ClCompile Include="..\\src\\renderer\\Lines.c" />'
    source=source.replace(entry,entry+'\n    '+addition)
    h='<ClInclude Include="..\\src\\renderer\\Lines.h" />'
    return filemgr.project(source.replace(h,h+'\n    <ClInclude Include="..\\src\\renderer\\Lines_backend.h" />'))
