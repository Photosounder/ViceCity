"""Prepare exact original vertex methods, strict C callers and native model/plane consumers."""
from pathlib import Path
import re,json,sys
import compressed_vector_migration as m
from doors_migration import method
from stats_successor import current_path
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/compressed-vector-c-tests';saved=out/'before';tree=root if '--production' in sys.argv else out/'overlay'
if '--production' in sys.argv:
    for name in json.loads((out/'changes.json').read_text()):assert current_path(root,name).read_text()==m.calls((saved/name).read_text()),name
for name in ['CompressedVector.h','CompressedVector.c']:(out/name).write_text((tree/'src/collision'/name).read_text(),newline='\n')
if (tree/'src/collision/CollisionVector.h').exists():(out/'CollisionVector.h').write_text((tree/'src/collision/CollisionVector.h').read_text(),newline='\n')
(out/'original-CompressedVector.h').write_text((saved/'src/collision/CompressedVector.h').read_text(),newline='\n')
(out/'config.h').write_text('#pragma once\n#ifdef VU_COLLISION\n#define COMPRESSED_COL_VECTORS\n#endif\n',newline='\n')
(out/'owners.h').write_text('''#pragma once
#include "config.h"
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef int16_t int16;typedef int32_t int32;
struct CVector {
    float x,y,z;
    CVector(void) {}
    CVector(float x,float y,float z) {this->x=x;this->y=y;this->z=z;}
};
inline CVector operator/(const CVector &v,float divisor) {return CVector(v.x/divisor,v.y/divisor,v.z/divisor);}
#ifdef ORIGINAL
#include "original-CompressedVector.h"
#else
#include "CompressedVector.h"
#endif
''',newline='\n')
entry='''#pragma once
#include "config.h"
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
'''
original='#include "owners.h"\n#include "entry.h"\n'
caller='#include "entry.h"\n#ifndef ORIGINAL\n#include "CompressedVector.h"\n#endif\n'
for name,args,values in [('Get','const void *self,float *value','self,value'),('Set','void *self,float x,float y,float z','self,x,y,z'),('Unpack','const void *self,void *value','self,value')]:
    conditional='#ifdef COMPRESSED_COL_VECTORS\n' if name=='Unpack' else '';ending='#endif\n' if name=='Unpack' else ''
    for prefix in ['Run','Before']:entry+=conditional+'void '+prefix+'_'+name+'('+args+');\n'+ending
    old={'Get':'const CVector result=((const CompressedVector*)self)->Get();\n    value[0]=result.x;value[1]=result.y;value[2]=result.z;','Set':'((CompressedVector*)self)->Set(x,y,z);','Unpack':'((const CompressedVector*)self)->Unpack((int32*)value);'}[name]
    original+=conditional+'extern "C" void Before_'+name+'('+args+')\n{\n    // Invoke the complete original collision vertex operation\n    '+old+'\n}\n'+ending
    new={'Get':'const CollisionVector3 result=CompressedVector_Get((const CompressedVector*)self);\n    value[0]=result.x;value[1]=result.y;value[2]=result.z;','Set':'CompressedVector_Set((CompressedVector*)self,x,y,z);','Unpack':'CompressedVector_Unpack((const CompressedVector*)self,value);'}[name]
    caller+=conditional+'void Run_'+name+'('+args+')\n{\n    // Exercise the public C operation through an independent strict C11 caller\n#ifdef ORIGINAL\n    Before_'+name+'('+values+');\n#else\n    '+new+'\n#endif\n}\n'+ending
entry+='#ifdef __cplusplus\n}\n#endif\n'
integration='#include "owners.h"\nstruct CColModel {CompressedVector *vertices;void GetTrianglePoint(CVector &v,int i) const;};\n'
for phase,base in [('before',saved),('after',tree)]:
    integration+='#ifdef ORIGINAL\n' if phase=='before' else '#else\n'
    modelPath=current_path(base,'src/collision/ColModel.cpp');model=modelPath.read_text()
    if modelPath.suffix=='.c':
        # Freeze the exact historical native consumer only after proving its complete current C successor
        import col_model_migration as models
        frozen=(root/'.diagnostics/col-model-c-tests/before/src/collision/ColModel.cpp').read_text();assert models.kernel(frozen)==model
        assert method(frozen,'CColModel::GetTrianglePoint')==method(m.compressed_vector_calls((saved/'src/collision/ColModel.cpp').read_text()),'CColModel::GetTrianglePoint')
        model=frozen
    integration+=method(model,'CColModel::GetTrianglePoint')+'\n'
integration+='#endif\nstruct CColTriangle {int a,b,c;};\nstruct CColTrianglePlane {CVector values[3];void Set(const CVector &a,const CVector &b,const CVector &c){values[0]=a;values[1]=b;values[2]=c;}\n'
for phase,base in [('before',saved),('after',tree)]:
    integration+='#ifdef ORIGINAL\n' if phase=='before' else '#else\n'
    text=(base/'src/collision/ColTriangle.h').read_text()
    if phase=='after' and 'void Set(const CompressedVector *v, CColTriangle &tri)' not in text:
        # Retain the exact historical value-return adapter after the plane owner itself has moved to C
        text=m.compressed_vector_calls((saved/'src/collision/ColTriangle.h').read_text())
    start=text.index('void Set(const CompressedVector *v, CColTriangle &tri)');opening=text.index('{',start);depth=1;end=opening+1
    while depth:
        if text[end]=='{':depth+=1
        elif text[end]=='}':depth-=1
        end+=1
    integration+=text[start:end]+'\n'
integration+='''#endif
};
extern "C" void Integration_Decode(CompressedVector *vertices,float *output)
{
    // Exercise the actual native model and plane consumers across the C structure-return boundary
    CColModel model;model.vertices=vertices;CVector value;model.GetTrianglePoint(value,1);
    output[0]=value.x;output[1]=value.y;output[2]=value.z;
    CColTriangle tri={2,0,1};CColTrianglePlane plane;plane.Set(vertices,tri);
    for(int i=0;i<3;++i){output[3+i*3]=plane.values[i].x;output[4+i*3]=plane.values[i].y;output[5+i*3]=plane.values[i].z;}
}
'''
for name,value in [('entry.h',entry),('original.cpp',original),('caller.c',caller),('integration.cpp',integration),('fixture.cpp',(root/'utils/tests/compressed_vector.cpp.in').read_text())]:(out/name).write_text(value,newline='\n')
print('Prepared all original/C vertex operations and literal native collision consumers')
