"""Audit complete boat-foam relocation and compare unchanged original decisions with C11."""
from pathlib import Path
import re,json
import boat_foam_migration as m
root=Path(__file__).resolve().parents[2];out=root/'build/boat-foam-c-tests'

def normalize(source):
    # Compare all complete statements independently of comments and formatting
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',source,flags=re.S))

for path in json.loads((out/'changes.json').read_text()):
    old=(out/'before'/path).read_text();expected=m.project(old) if path.startswith('build/') else m.calls(old)
    assert normalize(expected)==normalize((root/path).read_text()),path
    assert b'\r' not in (root/path).read_bytes(),path
old=(out/'before/src/renderer/SpecialFX.cpp').read_text();header=(out/'before/src/renderer/SpecialFX.h').read_text()
for name,value in [('BoatFoam.c',m.kernel(old)),('BoatFoam.cpp',m.HOST),('BoatFoam.h',m.HEADER),('BoatFoam_backend.h',m.BACKEND_HEADER)]:assert (root/'src/renderer'/name).read_text()==value
assert len(re.findall(r'\nCSpecialParticleStuff::\w+\(',m.section(old)))==3
for path in ['src/core/common.h','src/core/config.h','src/math/Vector.h','src/math/Vector.cpp','src/math/Vector2D.h','src/math/Matrix.h','src/math/Matrix.cpp','src/math/maths.h','src/math/Rect.h','src/renderer/ParticleType.h','src/renderer/Particle.h']:
    saved=out/'before'/path
    if not saved.exists():saved.parent.mkdir(parents=True,exist_ok=True);saved.write_bytes((root/path).read_bytes())
    assert saved.read_bytes()==(root/path).read_bytes(),path
    if path.startswith('src/math/') and path.endswith('.h'):(out/Path(path).name).write_text(saved.read_text(),newline='\n')
common=(root/'build/special-fx-c-tests/common.h').read_text();saved=out/'before/fixture-common.h'
if not saved.exists():saved.write_text(common,newline='\n')
assert saved.read_text()==common
(out/'common.h').write_text('#ifndef NATIVE_PROJECT_HEADERS\n#define RWCORE_H\n#endif\n'+common,newline='\n')
owners='''#pragma once
#include "BoatFoam_backend.h"
#include "Timer.h"
#include "General.h"
#include "ParticleType.h"
class CCamera { public:CVector forward;CVector &GetForward(void); };
extern CCamera TheCamera;
extern CRGBA FoamColour;
class CEntity;
class CParticle { public:
    static CParticle *AddParticle(tParticleType,const CVector &,const CVector &,CEntity *,float,const RwRGBA &,int32,int32,int32,int32);
};
'''
(out/'owners.h').write_text(owners,newline='\n')
for name in ['Camera.h','Particle.h']:(out/name).write_text('#pragma once\n#include "owners.h"\n',newline='\n')
(out/'original-BoatFoam.h').write_text('#pragma once\n'+m.record_header(header),newline='\n')
prototypes='''typedef struct TestBoatState {
    uint32_t boatStart,animationStart,lastFrame;
    float x,y,z,dx,dy,dz;
} TestBoatState;
TestBoatState TestBF_State(void);
void TestBF_SetState(TestBoatState value);
void TestBF_Bind(uint32_t *animationStart,uint32_t *lastFrame,float *x,float *y,float *z,float *dx,float *dy,float *dz);
void TestBF_Start(void);
void TestBF_Update(void *matrix);
void TestBF_Create(void *matrix,float innerFw,float innerRg,float innerUp,int32_t count);
BoatFoamVector TestBF_RandomPosition(uint16_t value,float innerFw,float innerRg);
'''
(out/'caller.h').write_text('#pragma once\n#include "BoatFoam_backend.h"\n#ifdef __cplusplus\nextern "C" {\n#endif\n'+prototypes+'#ifdef __cplusplus\n}\n#endif\n',newline='\n')
bind='TestBF_Bind(&AnimationStartTime,&LastFrameInAnimation,&X,&Y,&Z,&dX,&dY,&dZ);'
observer='''static uint32_t *observedStart,*observedFrame;
static float *observedX,*observedY,*observedZ,*observedDX,*observedDY,*observedDZ;
void TestBF_Bind(uint32_t *animationStart,uint32_t *lastFrame,float *x,float *y,float *z,float *dx,float *dy,float *dz)
{
    // Expose only legal observation aliases without changing any original algorithm value
    observedStart=animationStart;observedFrame=lastFrame;observedX=x;observedY=y;observedZ=z;observedDX=dx;observedDY=dy;observedDZ=dz;
}
TestBoatState TestBF_State(void)
{
    // Observe all original persistent state and its zero initialization before first update
    TestBoatState result={@BOAT@,0,0,0,0,0,0,0,0};
    if(observedStart){
        // Read each correctly typed original static through its own address
        result.animationStart=*observedStart;result.lastFrame=*observedFrame;result.x=*observedX;result.y=*observedY;result.z=*observedZ;result.dx=*observedDX;result.dy=*observedDY;result.dz=*observedDZ;
    }
    return result;
}
void TestBF_SetState(TestBoatState value)
{
    // Seed the original legal persistent scalars only after their observation addresses were bound
    @BOAT@=value.boatStart;
    if(observedStart){
        // Avoid any assumption that function-local statics have adjacent storage
        *observedStart=value.animationStart;*observedFrame=value.lastFrame;*observedX=value.x;*observedY=value.y;*observedZ=value.z;*observedDX=value.dx;*observedDY=value.dy;*observedDZ=value.dz;
    }
}
'''
body=m.section(old);declaration='static float X, Y, Z, dX, dY, dZ;'
assert body.count(declaration)==1
body=body.replace(declaration,declaration+'\n\t// Bind observation-only aliases before the original first operation\n\t'+bind)
original='#include "common.h"\n#include "owners.h"\n#include "original-BoatFoam.h"\n#include "caller.h"\nCRGBA FoamColour(255,255,255,255);\n'+body+'\nextern "C" {\n'+observer.replace('@BOAT@','CSpecialParticleStuff::BoatFromStart')+'}\n'
for name,args,invoke in [('Start','void','CSpecialParticleStuff::StartBoatFoamAnimation()'),('Update','void *matrix','CSpecialParticleStuff::UpdateBoatFoamAnimation((CMatrix *)matrix)'),('Create','void *matrix,float innerFw,float innerRg,float innerUp,int32_t count','CSpecialParticleStuff::CreateFoamAroundObject((CMatrix *)matrix,innerFw,innerRg,innerUp,count)')]:original+='extern "C" void TestBF_'+name+'('+args+')\n{\n    // Enter the complete original method\n    '+invoke+';\n}\n'
a=old.index('\t\tpos.x = (int8)');b=old.index('\n\t\tif (DotProduct2D',a)
original+='extern "C" BoatFoamVector TestBF_RandomPosition(uint16_t value,float innerFw,float innerRg)\n{\n    // Execute every original random-position statement for exhaustive byte-narrowing comparison\n    int rnd=value;CVector pos;\n'+old[a:b]+'\n    BoatFoamVector result={pos.x,pos.y,pos.z};return result;\n}\n'
(out/'original.cpp').write_text(original,newline='\n')
kernel=(root/'src/renderer/BoatFoam.c').read_text().replace(declaration,declaration+'\n\t// Bind the same observation-only aliases without changing production state\n\t'+bind)
assert kernel.count(bind)==1
(out/'observed-kernel.c').write_text('#include "caller.h"\n'+kernel+'\n'+observer.replace('@BOAT@','BoatFromStart')+'\nBoatFoamVector TestBF_RandomPosition(uint16_t value,float innerFw,float innerRg)\n{\n    // Exercise the actual private C coordinate helper rather than a test reimplementation\n    return BoatFoam_RandomPosition(value,innerFw);\n}\n',newline='\n')
caller='#include "caller.h"\n'
for name,args,invoke in [('Start','void','BoatFoam_StartBoatFoamAnimation()'),('Update','void *matrix','BoatFoam_UpdateBoatFoamAnimation(matrix)'),('Create','void *matrix,float innerFw,float innerRg,float innerUp,int32_t count','BoatFoam_CreateFoamAroundObject(matrix,innerFw,innerRg,innerUp,count)')]:caller+='void TestBF_'+name+'('+args+')\n{\n    // Enter the actual algorithm through a separately compiled C11 public caller\n    '+invoke+';\n}\n'
(out/'caller.c').write_text(caller,newline='\n')
(out/'host.cpp').write_text(m.HOST,newline='\n')
matrix=(out/'before/src/math/Matrix.cpp').read_text();ctor=matrix[matrix.index('CMatrix::CMatrix(void)'):matrix.index('CMatrix::CMatrix(CMatrix const &')]
dtor=m.function(matrix,'CMatrix::','~CMatrix');vector=(out/'before/src/math/Vector.cpp').read_text();transform=vector[vector.index('CVector\noperator*(const CMatrix &'):]
(out/'original-math.cpp').write_text('#include "common.h"\n'+ctor+'\n'+dtor+'\n'+transform,newline='\n')
absolute=(root/'build/bullet-traces-c-tests/fast-absf.c').read_text();(out/'fast-absf.c').write_text(absolute,newline='\n')
actualAbs=Path('C:/msys/home/rouziclib/math/ieee754.c').read_text()
assert normalize(m.function(absolute,'','fast_absf'))==normalize(m.function(actualAbs,'','fast_absf'))
(out/'test.cpp').write_text((root/'utils/tests/boat_foam.cpp.in').read_text(),newline='\n')
print('Audited all three complete methods, exact typed boundary, persistent state, live color and five entire source/project files')
