"""Convert every general helper and preserve compiler-selected random overloads."""
from pathlib import Path
import json,re,textwrap
import timestep_migration as timestep

EVIDENCE=json.loads(Path(__file__).with_name('general_overloads.json').read_text())
KINDS={re.sub(r'\s+','',item['arguments']):item['kind'] for item in EVIDENCE}
METHODS=['GetATanOfXY','LimitAngle','LimitRadianAngle','GetRadianAngleBetweenPoints','GetAngleBetweenPoints','GetNodeHeadingFromVector','faststrcmp','faststrncmp','faststricmp','SolveQuadratic','GetRandomNumber','GetRandomTrueFalse','GetRandomNumberInRange','SetRandomSeed']
COMMENTS={
 'GetATanOfXY':'Compute the original quadrant angle using float ratios',
 'LimitAngle':'Wrap degrees into the original half-open range',
 'LimitRadianAngle':'Clamp and wrap radians at the original float boundaries',
 'GetRadianAngleBetweenPoints':'Compute point-to-point radians with the original zero-y adjustment',
 'GetAngleBetweenPoints':'Convert the original point-to-point angle into degrees',
 'GetNodeHeadingFromVector':'Map the vector angle to the original eight heading sectors',
 'faststrcmp':'Report string differences including unequal lengths',
 'faststrncmp':'Compare the first string prefix up to the original count',
 'faststricmp':'Compare case-folded characters using the configured implementation',
 'SolveQuadratic':'Leave failed outputs untouched and write the second root before the first',
 'GetRandomNumber':'Consume one random draw and apply the configured mask',
 'GetRandomTrueFalse':'Compare one random draw against the original Boolean threshold',
 'GetRandomNumberInRange':'Consume one draw and retain the original floating-point range formula',
 'SetRandomSeed':'Pass the original signed seed through the random provider conversion'}

def masked(source):
    # Keep source offsets while excluding comments and literal contents
    return re.sub(r'//[^\n]*|/\*[\s\S]*?\*/|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\\n])*\'',lambda m:''.join('\n' if c=='\n' else ' ' for c in m[0]),source)

def arguments(source,start):
    # Read the complete call without treating nested expressions as argument separators
    opening=source.index('(',start);end=opening+1;depth=1
    while depth:
        depth+=(source[end]=='(')-(source[end]==')');end+=1
    return opening,end,source[opening+1:end-1]

def calls(source):
    # Replace each former static member with its explicit C entry point
    if 'class CGeneral' in source:return header(source)
    code=masked(source);changes=[]
    for match in re.finditer(r'CGeneral::(\w+)\b',code):
        name=match[1];target='General_'+name
        if name=='GetRandomNumberInRange':
            opening,end,args=arguments(source,match.end())
            target+='Float' if KINDS[re.sub(r'\s+','',args)]=='FLOAT' else 'Int'
        if name=='SolveQuadratic':
            opening,end,args=arguments(source,match.end())
            assert args.endswith('root1, root2')
            changes.append((opening+1,end-1,args[:-len('root1, root2')]+'&root1, &root2'))
        changes.append((match.start(),match.end(),target))
    for a,b,value in sorted(changes,reverse=True):source=source[:a]+value+source[b:]
    return timestep.calls(source)

def methods(source):
    # Extract every original inline function body, including both range overloads
    result=[]
    for match in re.finditer(r'\bstatic\s+(\w+)\s+(\w+)\(([^)]*)\)\s*\{',source):
        start=match.end();end=start;depth=1
        while depth:depth+=(source[end]=='{')-(source[end]=='}');end+=1
        result.append((match[1],match[2],match[3],source[start:end-1]))
    assert len(result)==15
    return result

def signature(result,name,args):
    # Replace reference outputs and fixed-width aliases with C declarations
    suffix=('Float' if result=='float' else 'Int') if name=='GetRandomNumberInRange' else ''
    text=result+' General_'+name+suffix+'('+args.replace('float &','float *')+')'
    for a,b in [('int32','int32_t'),('uint32','uint32_t'),('uint16','uint16_t')]:text=re.sub(r'\b'+a+r'\b',b,text)
    return text

def header(source):
    # Expose the complete helper API directly to C and existing C++ consumers
    declarations='\n'.join(signature(result,name,args)+';' for result,name,args,body in methods(source))
    return '//+ rouz edit (ChatGPT)\n#pragma once\n#include <stdbool.h>\n#include <stdint.h>\n#include "config.h"\n#ifdef __cplusplus\nextern "C" {\n#endif\n'+declarations+'\n#ifdef __cplusplus\n}\n#endif\n//- rouz edit (ChatGPT)\n'

def kernel(source):
    # Retain original arithmetic, comparisons, loop order and random state consumption
    result='''//+ rouz edit (ChatGPT)
#include "General.h"
#include "Random.h"
#include <ctype.h>
#include <math.h>
#define GENERAL_PI ((float)3.14159265358979323846)
#define GENERAL_TWOPI (GENERAL_PI * 2)
#define GENERAL_DEGTORAD(x) ((x) * GENERAL_PI / 180.0f)
#define GENERAL_RADTODEG(x) ((x) * 180.0f / GENERAL_PI)
#define GENERAL_CLAMP(v,low,high) ((v) < (low) ? (low) : (v) > (high) ? (high) : (v))
#ifdef USE_PS2_RAND
#define GENERAL_RAND_MAX 65535
#else
#define GENERAL_RAND_MAX 32767
#endif

'''
    for returntype,name,args,body in methods(source):
        body=body.replace('CGeneral::','').replace('float(MYRAND_MAX + 1)','(float)(MYRAND_MAX + 1)')
        for method in METHODS:
            if method!='GetRandomNumberInRange':body=re.sub(r'\b'+method+r'\(', 'General_'+method+'(',body)
        for a,b in [('Abs','fabsf'),('Atan2','atan2f'),('Sqrt','sqrtf'),('Floor','floorf'),('PI','GENERAL_PI'),('TWOPI','GENERAL_TWOPI'),('DEGTORAD','GENERAL_DEGTORAD'),('RADTODEG','GENERAL_RADTODEG'),('Clamp','GENERAL_CLAMP'),('MYRAND_MAX','GENERAL_RAND_MAX'),('uint32','uint32_t')]:body=re.sub(r'\b'+a+r'\b',b,body)
        if name=='SolveQuadratic':body=re.sub(r'\b(root[12])\s*=',r'*\1 =',body)
        body=textwrap.dedent(body).strip()
        body='\n'.join(line if line.startswith('#') else '\t'+line for line in body.splitlines())
        result+=signature(returntype,name,args)+'\n{\n\t// '+COMMENTS[name]+'\n'+body+'\n}\n\n'
    return result+'//- rouz edit (ChatGPT)\n'

def project(source):
    # Add the standalone C11 unit beside the existing general helper header
    if 'General.c"' in source:return timestep.project(source)
    if '<LanguageStandard_C>' in source:
        entry='    <ClCompile Include="..\\src\\core\\General.c">\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>\n'
    else:entry='    <ClCompile Include="..\\src\\core\\General.c">\n      <Filter>core</Filter>\n    </ClCompile>\n'
    position=source.index('    <ClCompile Include="..\\src\\core\\Zones.cpp"')
    return timestep.project(source[:position]+entry+source[position:])
