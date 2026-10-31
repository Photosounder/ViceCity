"""Move the complete quaternion record, operators and overloaded conversions into C11."""
import re
from rect_migration import mask,closing,arguments
import object_names_migration as objects
RENAMES={'src/math/Quaternion.cpp':'src/math/Quaternion.c'}
DECLARATIONS=[
('CQuaternion','Make','float x,float y,float z,float w'),
('float','Magnitude','const CQuaternion *self'),('float','MagnitudeSqr','const CQuaternion *self'),
('void','Normalise','CQuaternion *self'),('void','Multiply','CQuaternion *self,const CQuaternion *q1,const CQuaternion *q2'),('void','Invert','CQuaternion *self'),
('const CQuaternion *','AddAssign','CQuaternion *self,const CQuaternion *right'),('const CQuaternion *','SubAssign','CQuaternion *self,const CQuaternion *right'),('const CQuaternion *','ScaleAssign','CQuaternion *self,float right'),('const CQuaternion *','DivideAssign','CQuaternion *self,float right'),
('CQuaternion','Negate','CQuaternion value'),('float','Dot','CQuaternion q1,CQuaternion q2'),
('CQuaternion','Add','CQuaternion left,CQuaternion right'),('CQuaternion','Sub','CQuaternion left,CQuaternion right'),('CQuaternion','Scale','CQuaternion left,float right'),('CQuaternion','LeftScale','float left,CQuaternion right'),('CQuaternion','Divide','CQuaternion left,float right'),
('void','Slerp','CQuaternion *self,const CQuaternion *q1,const CQuaternion *q2,float theta,float invSin,float t'),
('void','GetAxisAngle','const CQuaternion *self,QuaternionVectorOut axis,float *angle'),('void','SetAxisAngle','CQuaternion *self,QuaternionVectorRef axis,float angle'),
('void','GetMatrix','const CQuaternion *self,QuaternionMatrixOut matrix'),('void','SetMatrix','CQuaternion *self,QuaternionMatrixRef matrix'),
('void','GetEuler','const CQuaternion *self,float *f1,float *f2,float *f3'),('void','SetEuler','CQuaternion *self,float f1,float f2,float f3')]
OPERATIONS=[name for result,name,args in DECLARATIONS]

def header():
    # Keep the original four floats independent of native vector and matrix definitions
    return '''//+ rouz edit (ChatGPT)
#pragma once
typedef struct CQuaternion {float x,y,z,w;} CQuaternion;
typedef struct QuaternionVectorRef {const float *x,*y,*z;} QuaternionVectorRef;
typedef struct QuaternionVectorOut {float *x,*y,*z;} QuaternionVectorOut;
typedef struct QuaternionMatrixRef {QuaternionVectorRef right,up,at;} QuaternionMatrixRef;
typedef struct QuaternionMatrixOut {QuaternionVectorOut right,up,at;} QuaternionMatrixOut;
#ifdef __cplusplus
extern "C" {
#endif
'''+''.join(result+' Quaternion_'+name+'('+args+');\n' for result,name,args in DECLARATIONS)+'''#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''

def native():
    # Borrow actual native fields without introducing unrelated record views or gameplay bridges
    return '''//+ rouz edit (ChatGPT)
#pragma once
#include "Quaternion.h"
static inline QuaternionVectorRef Quaternion_VectorRef(const RwV3d &value)
{
    // Borrow the original native coordinates for ordered C reads
    const QuaternionVectorRef result={&value.x,&value.y,&value.z};return result;
}
static inline QuaternionVectorOut Quaternion_VectorOut(RwV3d &value)
{
    // Preserve the actual destination coordinates and their alias relationships
    const QuaternionVectorOut result={&value.x,&value.y,&value.z};return result;
}
static inline QuaternionMatrixRef Quaternion_MatrixRef(const RwMatrix &value)
{
    // Borrow only the original matrix axes while leaving flags and position outside the interface
    const QuaternionMatrixRef result={Quaternion_VectorRef(value.right),Quaternion_VectorRef(value.up),Quaternion_VectorRef(value.at)};return result;
}
static inline QuaternionMatrixOut Quaternion_MatrixOut(RwMatrix &value)
{
    // Preserve the original nine matrix writes through actual native field addresses
    const QuaternionMatrixOut result={Quaternion_VectorOut(value.right),Quaternion_VectorOut(value.up),Quaternion_VectorOut(value.at)};return result;
}
//- rouz edit (ChatGPT)
'''

def methods(source):
    # Distinguish all original overloads by their complete parameter text
    result=[];code=mask(source)
    for match in re.finditer(r'(?m)^CQuaternion::(\w+)\(([^\n]*)\)\n\{',code):
        start=match.end()-1;end=start+1;depth=1
        while depth:
            if code[end]=='{':depth+=1
            elif code[end]=='}':depth-=1
            end+=1
        result.append((match[1],source[match.start(2):match.end(2)],source[start+1:end-1]))
    assert len(result)==9,len(result)
    return result

def kernel(original):
    # Preserve all arithmetic groupings, live input aliases and ordered output writes in C
    result='''//+ rouz edit (ChatGPT)
#include "Quaternion.h"
#include <math.h>
#define QUATERNION_PI 3.1415926535897932384626433832795f
#define QUATERNION_TWOPI (2.0f*QUATERNION_PI)
'''
    bodies={}
    bodies['Make']='const CQuaternion value={x,y,z,w};return value;'
    bodies['MagnitudeSqr']='return self->x*self->x+self->y*self->y+self->z*self->z+self->w*self->w;'
    bodies['Magnitude']='return sqrtf(Quaternion_MagnitudeSqr(self));'
    bodies['Invert']='self->x=-self->x;self->y=-self->y;self->z=-self->z;'
    for name,op in [('AddAssign','+'),('SubAssign','-')]:bodies[name]=''.join('self->'+field+op+'=right->'+field+';' for field in 'xyzw')+'return self;'
    for name,op in [('ScaleAssign','*'),('DivideAssign','/')]:bodies[name]=''.join('self->'+field+op+'=right;' for field in 'xyzw')+'return self;'
    bodies['Negate']='return Quaternion_Make(-value.x,-value.y,-value.z,-value.w);'
    bodies['Dot']='return q1.x*q2.x+q1.y*q2.y+q1.z*q2.z+q1.w*q2.w;'
    for name,op in [('Add','+'),('Sub','-')]:bodies[name]='return Quaternion_Make('+','.join('left.'+field+op+'right.'+field for field in 'xyzw')+');'
    for name,op in [('Scale','*'),('Divide','/')]:bodies[name]='return Quaternion_Make('+','.join('left.'+field+op+'right' for field in 'xyzw')+');'
    bodies['LeftScale']='return Quaternion_Make('+','.join('left*right.'+field for field in 'xyzw')+');'
    for name,args,body in methods(original):
        public=name
        if name in ['Get','Set']:
            public+=('AxisAngle' if 'RwV3d' in args else 'Matrix' if 'RwMatrix' in args else 'Euler')
        body=body.replace('MagnitudeSqr()','Quaternion_MagnitudeSqr(self)').replace('RecipSqrt(sq)','(1.0f/sqrtf(sq))')
        body=body.replace('*this = q2;','*self = *q2;').replace('*this = w1 * q1 + w2 * q2;','*self = Quaternion_Add(Quaternion_LeftScale(w1,*q1),Quaternion_LeftScale(w2,*q2));')
        body=body.replace('q1.','q1->').replace('q2.','q2->')
        body=re.sub(r'\bmatrix(?:->|\.)(right|up|at)\.([xyz])',r'(*matrix.\1.\2)',body)
        body=re.sub(r'\baxis->([xyz])',r'(*axis.\1)',body)
        body=re.sub(r'(?<![\w.>])([xyzw])\b',r'self->\1',body)
        body=re.sub(r'\b(Sqrt|Sin|Cos|Acos|Atan2)\(',lambda m:{'Sqrt':'sqrtf','Sin':'sinf','Cos':'cosf','Acos':'acosf','Atan2':'atan2f'}[m[1]]+'(',body)
        body=re.sub(r'\bTWOPI\b','QUATERNION_TWOPI',body);body=re.sub(r'\bPI\b','QUATERNION_PI',body)
        if public=='GetEuler':
            body=body.replace('RwMatrix matrix;','struct {float x,y,z;} right,up,at;\n    const QuaternionMatrixOut output={{&right.x,&right.y,&right.z},{&up.x,&up.y,&up.z},{&at.x,&at.y,&at.z}};')
            body=body.replace('Get(&matrix);','Quaternion_GetMatrix(self,output);')
            body=re.sub(r'\(\*matrix\.(right|up|at)\.([xyz])\)',r'\1.\2',body)
        bodies[public]=body.strip()
    for resultType,name,args in DECLARATIONS:
        result+=resultType+' Quaternion_'+name+'('+args+')\n{\n    // Preserve the original quaternion operation and its field evaluation order\n    '+bodies[name]+'\n}\n'
    return result+'//- rouz edit (ChatGPT)\n'

def quaternion_calls(source):
    # Convert only the complete quaternion owner and its diagnosed native consumers
    original=source
    if 'class CQuaternion\n' in source:return header()
    if re.search(r'(?m)^CQuaternion::Normalise\(',source):return kernel(source)
    if re.search(r'(?m)^void Quaternion_Normalise\(',source) or 'typedef struct CQuaternion {' in source:return source
    if 'CQuaternion' not in source and not any(text in source for text in ['DotProduct(last, frame->rotation)','frame->rotation = -frame->rotation']):return source
    # Replace constructor expressions and named four-float initialization at their original boundaries
    source=re.sub(r'\bCQuaternion (q, )?(rot)\(([^;\n]+)\);',lambda m:'CQuaternion '+(m[1] or '')+m[2]+'=Quaternion_Make('+m[3]+');',source)
    source=source.replace('CQuaternion(0.0f, 0.0f, 0.0f, 0.0f)','Quaternion_Make(0.0f, 0.0f, 0.0f, 0.0f)')
    source=source.replace('frame->rotation = -frame->rotation','frame->rotation = Quaternion_Negate(frame->rotation)')
    for receiver in ['rot','rotB']:
        for operation,token in [('AddAssign','+='),('SubAssign','-='),('ScaleAssign','*=')]:
            source=re.sub(r'\b'+receiver+r' '+re.escape(token)+r' ([^;\n]+);',lambda m:'Quaternion_'+operation+'(&'+receiver+','+('&' if operation in ['AddAssign','SubAssign'] else '')+m[1]+');',source)
    source=re.sub(r'\brot\.(Normalise|Invert)\(\)',lambda m:'Quaternion_'+m[1]+'(&rot)',source)
    source=source.replace('rot.Get(mat)','Quaternion_GetMatrix(&rot,Quaternion_MatrixOut(*mat))')
    source=source.replace('quat.Set(&forkAxis, -m_fWheelAngle)','Quaternion_SetAxisAngle(&quat,Quaternion_VectorRef(forkAxis),-m_fWheelAngle)')
    source=source.replace('quat.Get(rot.m_attachment)','Quaternion_GetMatrix(&quat,Quaternion_MatrixOut(*rot.m_attachment))')
    code=mask(source)
    for match in reversed(list(re.finditer(r'\brot\.Slerp\(',code))):
        end=closing(code,match.end()-1);args=arguments(source[match.end():end-1]);assert len(args)==5,args
        source=source[:match.start()]+'Quaternion_Slerp(&rot,&'+args[0]+',&'+args[1]+','+','.join(args[2:])+')'+source[end:]
    source=re.sub(r'\bDotProduct\((rot, q|last, frame->rotation|kfA->rotation, kfB->rotation|rotA, rotB)\)',r'Quaternion_Dot(\1)',source)
    if source==original:return source
    # Describe changed executable statements at their original function-body locations
    oldLines=set(original.splitlines());lines=[]
    for line in source.splitlines():
        if line not in oldLines and 'Quaternion_' in line and not line.lstrip().startswith('#'):
            indent=re.match(r'\s*',line)[0];lines.append(indent+'// Use the C quaternion operation at the original animation boundary')
        lines.append(line)
    source='\n'.join(lines)+'\n'
    if 'Quaternion_MatrixOut(' in source or 'Quaternion_VectorRef(' in source:source=source.replace('#include "common.h"','#include "common.h"\n#include "Quaternion_native.h"',1)
    return source if source.lstrip().startswith('//+ rouz edit (ChatGPT)') and source.rstrip().endswith('//- rouz edit (ChatGPT)') else '//+ rouz edit (ChatGPT)\n'+source.rstrip('\n')+'\n//- rouz edit (ChatGPT)\n'

def quaternion_project(source):
    # Select C11 for the complete producer while preserving native object distinctions
    source=source.replace('..\\src\\math\\Quaternion.cpp','..\\src\\math\\Quaternion.c')
    if 'Microsoft.Cpp.targets' in source:
        pattern=r'<ClCompile Include="\.\.\\src\\math\\Quaternion\.c"\s*(?:/>|>([\s\S]*?)</ClCompile>)'
        def metadata(match):
            # Preserve unrelated metadata while requiring C11 for the renamed quaternion producer
            body=match[1] or ''
            for tag,value in [('CompileAs','CompileAsC'),('LanguageStandard_C','stdc11')]:
                if '<'+tag+'>' in body:body=re.sub(r'<'+tag+r'>[^<]*</'+tag+'>','<'+tag+'>'+value+'</'+tag+'>',body)
                else:body+='\n      <'+tag+'>'+value+'</'+tag+'>'
            return '<ClCompile Include="..\\src\\math\\Quaternion.c">'+body+'\n    </ClCompile>'
        source=re.sub(pattern,metadata,source)
    anchor='<ClInclude Include="..\\src\\math\\Quaternion.h" />';entry='<ClInclude Include="..\\src\\math\\Quaternion_native.h" />'
    if entry not in source:source=source.replace(anchor,anchor+'\n    '+entry)
    return objects.project(source)



def calls(source):
    # Preserve complete quaternion ownership before all four C keyframe records
    import keyframes_migration as frames
    return frames.calls(quaternion_calls(source))


def project(source):
    # Retain quaternion C11 integration before registering the complete keyframe producer
    import keyframes_migration as frames
    return frames.project(quaternion_project(source))
