"""Convert complete wind algorithms and preserve typed legacy-vector reads."""
import re
import object_names_migration as object_names
import references_migration as references
HEADER='''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
typedef struct WindModifiersVector {
	float x, y, z;
} WindModifiersVector;
typedef struct WindModifiersRecord {
	WindModifiersVector m_pos;
	int32_t m_type;
} WindModifiersRecord;
#ifdef __cplusplus
extern "C" {
#endif
extern int32_t WindModifiers_Number;
void WindModifiers_RegisterOne(WindModifiersVector pos, int32_t type);
bool WindModifiers_FindWindModifier(WindModifiersVector pos, float *x, float *y);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
BACKEND_HEADER='''//+ rouz edit (ChatGPT)
#pragma once
#include "WindModifiers.h"
#ifdef __cplusplus
extern "C" {
#endif
WindModifiersVector WindModifiers_BackendVector(const void *position);
WindModifiersVector WindModifiers_BackendCamera(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
HOST='''//+ rouz edit (ChatGPT)
#include "common.h"
#include "Camera.h"
#include "WindModifiers_backend.h"

WindModifiersVector
WindModifiers_BackendVector(const void *position)
{
	// Copy the original typed position once without assuming a CVector layout
	const CVector *vector = (const CVector *)position;
	WindModifiersVector result = {vector->x, vector->y, vector->z};
	return result;
}

WindModifiersVector
WindModifiers_BackendCamera(void)
{
	// Query the live camera position only after the original capacity check
	return WindModifiers_BackendVector(&TheCamera.GetPosition());
}
//- rouz edit (ChatGPT)
'''
HELPERS='''static WindModifiersVector
WindModifiers_Subtract(WindModifiersVector left, WindModifiersVector right)
{
	// Retain all three original component subtractions
	WindModifiersVector result = {left.x-right.x, left.y-right.y, left.z-right.z};
	return result;
}

static WindModifiersVector
WindModifiers_Multiply(WindModifiersVector left, float right)
{
	// Multiply each original vector component before division
	WindModifiersVector result = {left.x*right, left.y*right, left.z*right};
	return result;
}

static WindModifiersVector
WindModifiers_Divide(WindModifiersVector left, float right)
{
	// Divide each original multiplied component without folding the scale factor
	WindModifiersVector result = {left.x/right, left.y/right, left.z/right};
	return result;
}

static float
WindModifiers_Magnitude(WindModifiersVector vector)
{
	// Retain the original float sum and sqrtf wrapper
	return sqrtf(vector.x*vector.x + vector.y*vector.y + vector.z*vector.z);
}

static bool
WindModifiers_NearCamera(WindModifiersVector pos)
{
	// Preserve the single camera query within the original short-circuit operand
	return WindModifiers_Magnitude(WindModifiers_Subtract(pos, WindModifiers_BackendCamera())) < 100.0f;
}

'''
def calls(source):
    # Preserve every original position expression, short circuit and aliased output address
    if 'class CWindModifiers\n' in source:return HEADER
    if 'CWindModifiers Array[16];' in source:return HOST
    hasCalls='CWindModifiers::RegisterOne(' in source or 'CWindModifiers::FindWindModifier(' in source
    if hasCalls and '#include "WindModifiers_backend.h"' not in source:source=source.replace('#include "WindModifiers.h"','#include "WindModifiers.h"\n#include "WindModifiers_backend.h"')
    source=source.replace('CWindModifiers::RegisterOne(GetPosition(),','WindModifiers_RegisterOne(WindModifiers_BackendVector(&GetPosition()),')
    source=source.replace('CWindModifiers::FindWindModifier(GetPosition(),','WindModifiers_FindWindModifier(WindModifiers_BackendVector(&GetPosition()),')
    source=source.replace('CWindModifiers::FindWindModifier(TheCamera.GetPosition(),','WindModifiers_FindWindModifier(WindModifiers_BackendVector(&TheCamera.GetPosition()),')
    return references.calls(source.replace('CWindModifiers::Number','WindModifiers_Number'))

def kernel(source):
    # Relocate both entire original algorithms and explicit vector-operation order
    source=source[source.index('#define MAX_HEIGHT_DIST'):]
    source=source.replace('CWindModifiers Array[16];','static WindModifiersRecord Array[16];').replace('int32 CWindModifiers::Number;','int32_t WindModifiers_Number;')
    source=source.replace('CWindModifiers::RegisterOne(CVector pos, int32 type = 1)','WindModifiers_RegisterOne(WindModifiersVector pos, int32_t type)').replace('CWindModifiers::FindWindModifier(CVector pos, float *x, float *y)','WindModifiers_FindWindModifier(WindModifiersVector pos, float *x, float *y)')
    source=source.replace('CWindModifiers::Number','WindModifiers_Number');source=re.sub(r'(?<![\w])\bNumber\b','WindModifiers_Number',source)
    source=source.replace('(pos - TheCamera.GetPosition()).Magnitude() < 100.0f','WindModifiers_NearCamera(pos)')
    source=source.replace('CVector2D dir;','WindModifiersVector dir;').replace('Abs(','WindModifiers_Abs(')
    source=source.replace('(pos - Array[i].m_pos).Magnitude()','WindModifiers_Magnitude(WindModifiers_Subtract(pos, Array[i].m_pos))')
    source=source.replace('(pos - Array[i].m_pos) * fade / dist','WindModifiers_Divide(WindModifiers_Multiply(WindModifiers_Subtract(pos, Array[i].m_pos), fade), dist)')
    for name,comment in [('RegisterOne','Retain capacity and camera-distance checks before copying the original record'),('FindWindModifier','Preserve the last matching direction, single random draw and ordered output updates')]:
        source=re.sub(r'(WindModifiers_'+name+r'\([^\n]*\)\n\{)',r'\1\n\t// '+comment,source)
    source=source.replace('//- rouz edit (ChatGPT)','').rstrip()
    absolute='''#ifdef _MSC_VER
float fast_absf(float x);
#define WindModifiers_Abs fast_absf
#else
#define WindModifiers_Abs fabsf
#endif

'''
    return '//+ rouz edit (ChatGPT)\n#include "WindModifiers.h"\n#include "WindModifiers_backend.h"\n#include "General.h"\n#include <math.h>\n'+absolute+HELPERS+source+'\n//- rouz edit (ChatGPT)\n'

def project(source):
    # Compile the complete C wind logic beside its two typed position readers
    entry='<ClCompile Include="..\\src\\renderer\\WindModifiers.cpp" />'
    addition='<ClCompile Include="..\\src\\renderer\\WindModifiers.c" />' if '<Filter Include=' in source else '<ClCompile Include="..\\src\\renderer\\WindModifiers.c">\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>'
    source=source.replace(entry,entry+'\n    '+addition)
    entry='<ClCompile Include="..\\src\\renderer\\WindModifiers.cpp">\n      <Filter>renderer</Filter>\n    </ClCompile>'
    source=source.replace(entry,entry+'\n    '+entry.replace('WindModifiers.cpp','WindModifiers.c'))
    for entry in ['<ClInclude Include="..\\src\\renderer\\WindModifiers.h" />','<ClInclude Include="..\\src\\renderer\\WindModifiers.h">\n      <Filter>renderer</Filter>\n    </ClInclude>']:
        source=source.replace(entry,entry+'\n    '+entry.replace('WindModifiers.h','WindModifiers_backend.h'))
    return references.project(object_names.project(source))
