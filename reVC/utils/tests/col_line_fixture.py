"""Compare complete native line operations with independent C11 calls and real vector math."""
from pathlib import Path
import json,re,sys
import col_line_migration as m
from doors_migration import method
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/col-line-c-tests';saved=out/'before';tree=root if '--production' in sys.argv else out/'overlay'
if '--production' in sys.argv:
    for name in json.loads((out/'changes.json').read_text()):assert (root/m.RENAMES.get(name,name)).read_text()==m.calls((saved/name).read_text()),name
for name in ['ColLine.h','ColLine.c','CollisionVector.h','CollisionVector_native.h','ColLine_native.h']:(out/name).write_text((tree/'src/collision'/name).read_text(),newline='\n')
for name,source in [('original-ColLine.h',saved/'src/collision/ColLine.h'),('Vector.h',saved/'src/math/Vector.h'),('Maths.h',saved/'src/math/Maths.h'),('VuVector.h',tree/'src/math/VuVector.h'),('Matrix.h',saved/'src/math/Matrix.h')]:(out/name).write_text(source.read_text(),newline='\n')
(out/'owners.h').write_text('''#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <new>
#define nil nullptr
#ifdef _MSC_VER
#define TYPEALIGN(n) __declspec(align(n))
#else
#define TYPEALIGN(n) __attribute__((aligned(n)))
#endif
typedef int8_t int8;typedef int16_t int16;typedef uint16_t uint16;typedef int32_t int32;typedef uint8_t uint8;
struct RwV3d {float x,y,z;};struct RwMatrix;
#include "Maths.h"
#include "Vector.h"
#include "Matrix.h"
#ifdef ORIGINAL
#include "original-ColLine.h"
#else
#include "ColLine_native.h"
#endif
#include "VuVector.h"
''',newline='\n')
entry='''#pragma once
#ifdef __cplusplus
extern "C" {
#endif
'''
caller='''#include "entry.h"
#ifndef ORIGINAL
#include "ColLine.h"
#include <string.h>
static CollisionPointRef point(const void *value)
{
    // Refer to actual float coordinates without depending on native vector class declarations
    const unsigned char *p=(const unsigned char*)value;
    return CollisionPoint_Ref((const float*)p,(const float*)(p+4),(const float*)(p+8));
}
#endif
'''
original='#include "owners.h"\n#include "entry.h"\n'+re.sub(r'^#include[^\n]*\n','',(saved/'src/collision/ColLine.cpp').read_text(),flags=re.M)+'\n'
ops=[('Init','void *self,const void *a,const void *b','self,a,b','new(self) CColLine(*(const CVector*)a,*(const CVector*)b);','ColLine_Init((CColLine*)self,point(a),point(b));'),('Set','void *self,const void *a,const void *b','self,a,b','((CColLine*)self)->Set(*(const CVector*)a,*(const CVector*)b);','ColLine_Set((CColLine*)self,point(a),point(b));'),('Make','void *self,const void *a,const void *b','self,a,b','CColLine value(*(const CVector*)a,*(const CVector*)b);memcpy(self,&value.p0,12);memcpy((unsigned char*)self+16,&value.p1,12);','CColLine value=ColLine_Make(point(a),point(b));memcpy(self,&value.p0,12);memcpy((unsigned char*)self+16,&value.p1,12);'),('Raw','void *self,const void *bytes','self,bytes','memcpy(self,bytes,sizeof(CColLine));','*(CColLine*)self=ColLine_FromRaw32(bytes);')]
for name,args,values,old,new in ops:
    for prefix in ['Run','Before']:entry+='void '+prefix+'_'+name+'('+args+');\n'
    original+='extern "C" void Before_'+name+'('+args+')\n{\n    // Invoke the original complete line operation and retain both padding words\n    '+old+'\n}\n'
    caller+='void Run_'+name+'('+args+')\n{\n    // Exercise the public operation from a separately compiled strict C11 caller\n#ifdef ORIGINAL\n    Before_'+name+'('+values+');\n#else\n    '+new+'\n#endif\n}\n'
entry+='#ifdef __cplusplus\n}\n#endif\n'
source=(saved/'src/math/Vector.cpp').read_text();start=source.index('operator*(const CMatrix &mat, const CVector &vec)');start=source.rfind('\n',0,start-1)+1;end=source.index('\n}',start)+2;vector=source[start:end]
math='#include "owners.h"\nCMatrix::CMatrix() {}\nCMatrix::~CMatrix() {}\n'+vector+'\n'
for phase,base in [('before',saved),('after',tree)]:
    body=method((base/'src/math/math.cpp').read_text(),'TransformPoints' if phase=='before' else 'TransformColLines')
    math+=('#ifdef ORIGINAL\n' if phase=='before' else '#else\n')+body+'\n'
math+='#endif\n'
math+='''extern "C" void *NativeLineDefault(void *memory)
{
    // Preserve a separately compiled default-construction boundary for initialized storage
    return new(memory) CColLine;
}
extern "C" void *NativeLineInit(void *memory,const CVector *a,const CVector *b)
{
    // Exercise the native ABI without letting the fixture optimize across object lifetimes
#ifdef ORIGINAL
    return new(memory) CColLine(*a,*b);
#else
    CColLine *line=new(memory) CColLine;
    ColLine_Init(line,COLLISION_POINT_REF(*a),COLLISION_POINT_REF(*b));return line;
#endif
}
'''
for name,value in [('entry.h',entry),('caller.c',caller),('original.cpp',original),('math.cpp',math),('fixture.cpp',(root/'utils/tests/col_line.cpp.in').read_text())]:(out/name).write_text(value,newline='\n')
print('Prepared exact original/C line operations, native endpoint boundaries and complete batch transformation')
