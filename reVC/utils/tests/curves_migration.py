"""Move both complete curve algorithms into a C11 unit with explicit vector values."""
import re
import date_migration as date

HEADER='''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include "config.h"
typedef struct CurvesVector { float x, y, z; } CurvesVector;
#ifdef __cplusplus
#include "Vector.h"
static inline CurvesVector CurvesVector_FromVector(const CVector &vector)
{
    // Copy coordinates into a C value without relying on the C++ vector layout
    CurvesVector result = { vector.x, vector.y, vector.z };
    return result;
}
extern "C" {
#endif
float Curves_CalcSpeedScaleFactor(CurvesVector pPoint1, CurvesVector pPoint2, float dir1X, float dir1Y, float dir2X, float dir2Y);
void Curves_CalcCurvePoint(CurvesVector pPos1, CurvesVector pPos2, CurvesVector pDir1, CurvesVector pDir2, float between, int32_t timeOnCurve, CurvesVector *pOutPos, CurvesVector *pOutDir);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
HELPERS='''//+ rouz edit (ChatGPT)
#include "Curves.h"
#include <math.h>

static CurvesVector Curves_Subtract(CurvesVector left, CurvesVector right)
{
    // Preserve all three components of the original vector subtraction
    CurvesVector result = { left.x - right.x, left.y - right.y, left.z - right.z };
    return result;
}

static CurvesVector Curves_Scale(CurvesVector left, float right)
{
    // Preserve the original component-wise vector multiplication
    CurvesVector result = { left.x * right, left.y * right, left.z * right };
    return result;
}

static float Curves_Magnitude2D(CurvesVector vector)
{
    // Use the original float sum and square root for planar distance
    return sqrtf(vector.x * vector.x + vector.y * vector.y);
}

static float Curves_Dot2D(CurvesVector left, CurvesVector right)
{
    // Use the original two-component dot product and addition order
    return left.x * right.x + left.y * right.y;
}

'''

def arguments(source,start):
    # Split complete call arguments while retaining nested expressions and source offsets
    opening=source.index('(',start);end=opening+1;depth=1;last=end;parts=[]
    while depth:
        char=source[end]
        if char==',' and depth==1:parts.append(source[last:end].strip());last=end+1
        depth+=(char=='(')-(char==')');end+=1
    parts.append(source[last:end-1].strip())
    return opening,end,parts

def calls(source):
    # Remove the static owner and convert every complete actual curve call
    if 'class CCurves' in source:return HEADER
    changes=[]
    for match in re.finditer(r'CCurves::(CalcSpeedScaleFactor|CalcCurvePoint)\b',source):
        name=match[1];opening,end,parts=arguments(source,match.end())
        count=2 if name=='CalcSpeedScaleFactor' else 4
        converted=['CurvesVector_FromVector(*('+arg+'))' for arg in parts[:count]]+parts[count:]
        if name=='CalcSpeedScaleFactor':
            assert len(parts)==6
            replacement='Curves_'+name+'(\n\t\t'+',\n\t\t'.join(converted)+'\n\t)'
        else:
            assert len(parts)==8 and source[end]==';'
            position,direction=parts[-2:];converted[-2:]=['&curvePosition','&curveDirection']
            replacement='''// Calculate the curve through C and retain the original output assignment order
    {
        // Keep both C outputs separate until the curve calculation completes
        CurvesVector curvePosition, curveDirection;
        Curves_CalcCurvePoint(
            '''+',\n            '.join(converted)+'''
        );
        // Copy the position first and then the direction into the caller's vectors
        *('''+position+''') = CVector(curvePosition.x, curvePosition.y, curvePosition.z);
        *('''+direction+''') = CVector(curveDirection.x, curveDirection.y, curveDirection.z);
    }'''
            end+=1
        changes.append((match.start(),end,replacement))
    for a,b,value in reversed(changes):source=source[:a]+value+source[b:]
    return date.calls(source)

def kernel(source):
    # Preserve the full original algorithms and both strict/inclusive coefficient branches
    source=source[source.index('float CCurves::CalcSpeedScaleFactor'):]
    source=source.replace('CCurves::','Curves_').replace('int32 timeOnCurve','int32_t timeOnCurve')
    source=re.sub(r'CVector\* (pPoint[12]|pPos[12]|pDir[12])',r'CurvesVector \1',source)
    source=source.replace('CVector* pOut','CurvesVector* pOut')
    for name in ['pPoint1','pPoint2','pPos1','pPos2','pDir1','pDir2']:source=source.replace(name+'->',name+'.')
    source=source.replace('CVector2D dir1(dir1X, dir1Y);','CurvesVector dir1 = { dir1X, dir1Y, 0.0f };').replace('CVector2D dir2(dir2X, dir2Y);','CurvesVector dir2 = { dir2X, dir2Y, 0.0f };')
    source=source.replace('(*pPoint1 - *pPoint2).Magnitude2D()','Curves_Magnitude2D(Curves_Subtract(pPoint1, pPoint2))')
    source=source.replace('CVector2D dir1 = *pDir1 * actualFactor;','CurvesVector dir1 = Curves_Scale(pDir1, actualFactor);').replace('CVector2D dir2 = *pDir2 * actualFactor;','CurvesVector dir2 = Curves_Scale(pDir2, actualFactor);')
    source=source.replace('DotProduct2D(*pPos2 - *pPos1, *pDir1)','Curves_Dot2D(Curves_Subtract(pPos2, pPos1), pDir1)').replace('DotProduct2D(*pPos1 - *pPos2, *pDir2)','Curves_Dot2D(Curves_Subtract(pPos1, pPos2), pDir2)')
    source=source.replace('DotProduct2D(','Curves_Dot2D(').replace('CalcSpeedScaleFactor(pPos1','Curves_CalcSpeedScaleFactor(pPos1')
    source=source.replace('Abs(','fabsf(').replace('Cos(','cosf(')
    source=source.replace('= CVector(', '= (CurvesVector){').replace('0.0f);','0.0f};')
    source=source.replace('\n{\n','\n{\n\t// Retain the original curve arithmetic, branch boundaries and output order\n')
    assert 'CVector' not in source and 'CCurves' not in source and '->' not in source
    return HELPERS+source.rstrip()+'\n//- rouz edit (ChatGPT)\n'

def project(source):
    # Rename the entire algorithm unit and compile it as C11
    entry='<ClCompile Include="..\\src\\control\\Curves.cpp" />'
    if entry in source:return date.project(source.replace(entry,'<ClCompile Include="..\\src\\control\\Curves.c">\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>'))
    return date.project(source.replace('Curves.cpp"','Curves.c"'))
