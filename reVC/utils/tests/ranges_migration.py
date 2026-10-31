"""Replace both range value classes and their complete operations with C11 records."""
import re
import model_indices_migration as models

def header(dimension):
    # Declare the complete plain record and dimension-specific C point values
    prefix='Range'+str(dimension)+'D'
    return '''//+ rouz edit (ChatGPT)
#pragma once
#include <stdbool.h>
typedef struct '''+prefix+'Point { float '+','.join('xyz'[:dimension])+'''; } '''+prefix+'''Point;
typedef struct '''+prefix+' { '+prefix+'''Point min,max; } '''+prefix+''';
#ifdef __cplusplus
extern "C" {
#endif
void '''+prefix+'_Init('+prefix+' *range,'+prefix+'Point min,'+prefix+'''Point max);
bool '''+prefix+'_IsInRange(const '+prefix+' *range,'+prefix+'''Point point);
void '''+prefix+'_DebugShowRange(const '+prefix+''' *range,float value,int mode);
'''+prefix+'Point '+prefix+'_GetRandomPointInRange(const '+prefix+''' *range);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''

def kernel(source,dimension):
    # Preserve all original comparisons, width truncation, draw order and live per-axis minima
    prefix='Range'+str(dimension)+'D';native='CVector2D' if dimension==2 else 'CVector'
    normalized=re.sub(r'\s+','',re.sub(r'//[^\n]*','',source))
    assert 'C'+prefix+'::C'+prefix+'('+native+'_min,'+native+'_max):min(_min),max(_max){}' in normalized
    assert 'voidC'+prefix+'::DebugShowRange(float,int){}' in normalized
    a=source.index('bool\nC'+prefix+'::IsInRange');b=source.index('\nvoid\nC'+prefix+'::DebugShowRange',a)
    inside=source[a:b].replace('C'+prefix+'::IsInRange('+native+' vec)',prefix+'_IsInRange(const '+prefix+' *range,'+prefix+'Point vec)')
    a=source.index(native+'\nC'+prefix+'::GetRandomPointInRange');b=source.index('\n}',a)+2
    random=source[a:b].rstrip().replace(native+'\nC'+prefix+'::GetRandomPointInRange()',prefix+'Point\n'+prefix+'_GetRandomPointInRange(const '+prefix+' *range)')
    random=random.replace('Abs(','fabsf(')
    args=', '.join('out'+axis.upper() for axis in 'xyz'[:dimension])
    random=random.replace('return '+native+'('+args+');',prefix+'Point result={'+args+'};\n\treturn result;')
    body=inside+'\nvoid\n'+prefix+'_DebugShowRange(const '+prefix+' *range,float value,int mode)\n{\n    // Preserve the original empty debug operation for every input\n    (void)range;(void)value;(void)mode;\n}\n\n'+random
    for field in ['min','max']:body=re.sub(r'(?<![\w>])'+field+r'\.', 'range->'+field+'.',body)
    body=body.replace('vec)\n{','vec)\n{\n    // Preserve the original strict component comparisons and short-circuit order')
    body=body.replace('*range)\n{\n\tint distX','*range)\n{\n    // Capture every original integer width before drawing coordinates from live minima\n\tint distX')
    return '''//+ rouz edit (ChatGPT)
#include "'''+prefix+'''.h"
#include "General.h"
#include <math.h>
void '''+prefix+'_Init('+prefix+' *range,'+prefix+'Point min,'+prefix+'''Point max)
{
    // Copy both original captured endpoint values in their constructor order
    range->min=min;range->max=max;
}
'''+body+'\n//- rouz edit (ChatGPT)\n'

def calls(source):
    # Replace both class declarations, complete units and the one actual pedestrian call site
    if 'CRange' not in source and 'm_wanderRangeBounds->GetRandomPointInRange' not in source:return models.calls(source)
    for dimension in [2,3]:
        prefix='Range'+str(dimension)+'D'
        if 'C'+prefix+'::C'+prefix+'(' in source:return models.calls(kernel(source,dimension))
        if 'class C'+prefix+'\n' in source:return models.calls(header(dimension))
    source=source.replace('class CRange2D *m_wanderRangeBounds;','struct Range2D *m_wanderRangeBounds; // rouz edit (ChatGPT)')
    source=source.replace('CVector2D newCoords2D = m_wanderRangeBounds->GetRandomPointInRange();','//+ rouz edit (ChatGPT)\n\t\t\t// Read the same sampled coordinates from the C range record\n\t\t\tRange2DPoint newCoords2D = Range2D_GetRandomPointInRange(m_wanderRangeBounds);\n\t\t\t//- rouz edit (ChatGPT)')
    return models.calls(source)

def project(source):
    # Replace both C++ source entries with actual C11 units and preserve every existing object distinction
    for dimension in [2,3]:
        prefix='Range'+str(dimension)+'D'
        entry='<ClCompile Include="..\\src\\core\\'+prefix+'.cpp" />'
        if entry in source:source=source.replace(entry,'<ClCompile Include="..\\src\\core\\'+prefix+'.c">\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>')
        source=source.replace('Include="..\\src\\core\\'+prefix+'.cpp"','Include="..\\src\\core\\'+prefix+'.c"')
    return models.project(source)

def target(path):
    # Preserve whole-source audit coverage after the two complete source-file relocations
    return path[:-3]+'c' if path in ['src/core/Range2D.cpp','src/core/Range3D.cpp'] else path
