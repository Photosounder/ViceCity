"""Audit full wind relocation and generate original/C callers and storage observers."""
from pathlib import Path
import re,json
import wind_migration as m
root=Path(__file__).resolve().parents[2];out=root/'build/wind-c-tests'
def normalize(source):return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',source,flags=re.S))
for path in json.loads((out/'changes.json').read_text()):
    old=(out/'before'/path).read_text();expected=m.project(old) if path.startswith('build/') else m.calls(old)
    assert normalize(expected)==normalize((root/path).read_text()),path
    assert b'\r' not in (root/path).read_bytes(),path
old=(out/'before/src/renderer/WindModifiers.cpp').read_text();header=(out/'before/src/renderer/WindModifiers.h').read_text()
assert normalize(m.kernel(old))==normalize((root/'src/renderer/WindModifiers.c').read_text())
assert (root/'src/renderer/WindModifiers_backend.h').read_text()==m.BACKEND_HEADER
assert old.count('CWindModifiers::RegisterOne(')==1 and old.count('CWindModifiers::FindWindModifier(')==1
deps=['src/math/Vector.h','src/math/Vector2D.h','src/math/maths.h','vendor/librw/src/rwbase.h']
for path in deps:
    saved=out/'before'/path
    if not saved.exists():saved.parent.mkdir(parents=True,exist_ok=True);saved.write_bytes((root/path).read_bytes())
    assert saved.read_bytes()==(root/path).read_bytes(),path
for path in deps[:3]:(out/Path(path).name).write_text((out/'before'/path).read_text(),newline='\n')
rw=(out/'before/vendor/librw/src/rwbase.h').read_text();a=rw.index('struct V3d\n{');b=rw.index('\n};',a)+3
(out/'rw-vector.h').write_text('namespace rw { typedef float float32; typedef int32_t int32; struct Matrix;\n'+rw[a:b]+'\n}\ntypedef rw::V3d RwV3d;\n',newline='\n')
common='''#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "config.h"
typedef int32_t int32;
typedef uint16_t uint16;
typedef uint32_t uint32;
#define nil NULL
#include "rw-vector.h"
#include "maths.h"
#include "Vector.h"
#include "Vector2D.h"
'''
(out/'common.h').write_text(common,newline='\n')
(out/'Camera.h').write_text('''#pragma once
extern unsigned testCameraReads;
class CCamera { public: CVector position;
    const CVector &GetPosition(void) {
        // Count each original lazy camera read without altering its typed coordinates
        testCameraReads++;return position;
    }
};
extern CCamera TheCamera;
''',newline='\n')
commonInclude='#ifdef NATIVE_PROJECT_HEADERS\n#include "'+(root/'src/core/common.h').as_posix()+'"\n#else\n#include "common.h"\n#endif'
cameraInclude='#ifdef NATIVE_PROJECT_HEADERS\n#include "'+(root/'src/core/Camera.h').as_posix()+'"\n#else\n#include "Camera.h"\n#endif'
# Expose only access control in the oracle header without changing any data declaration or method body
(out/'original-WindModifiers.h').write_text(header.replace('class CWindModifiers\n{','class CWindModifiers\n{\npublic:'),newline='\n')
(out/'original.cpp').write_text(old.replace('#include "common.h"',commonInclude).replace('#include "Camera.h"',cameraInclude).replace('#include "WindModifiers.h"','#include "original-WindModifiers.h"'),newline='\n')
protos='''void TestWind_Register(WindModifiersVector pos,int32_t type);
bool TestWind_Find(WindModifiersVector pos,float *x,float *y);
int32_t *TestWind_Count(void);
WindModifiersRecord TestWind_Record(int32_t index);
'''
(out/'caller.h').write_text('#pragma once\n#include "WindModifiers.h"\n#ifdef __cplusplus\nextern "C" {\n#endif\n'+protos+'#ifdef __cplusplus\n}\n#endif\n',newline='\n')
(out/'caller.c').write_text('''#include "caller.h"
void TestWind_Register(WindModifiersVector pos,int32_t type)
{
    // Call the complete registration algorithm through a separate C11 caller
    WindModifiers_RegisterOne(pos,type);
}
bool TestWind_Find(WindModifiersVector pos,float *x,float *y)
{
    // Preserve value inputs and caller output pointers at the C API boundary
    return WindModifiers_FindWindModifier(pos,x,y);
}
int32_t *TestWind_Count(void)
{
    // Return the actual live frame-reset count
    return &WindModifiers_Number;
}
''',newline='\n')
(out/'observed-kernel.c').write_text('#include "'+(root/'src/renderer/WindModifiers.c').as_posix()+'''"
WindModifiersRecord TestWind_Record(int32_t index)
{
    // Observe actual private storage in the same C translation unit as the full implementation
    return Array[index];
}
''',newline='\n')
(out/'original-caller.cpp').write_text(commonInclude+'''
#include "original-WindModifiers.h"
#include "caller.h"
#include <stddef.h>
extern CWindModifiers Array[16];
static_assert(sizeof(CWindModifiers)==16 && alignof(CWindModifiers)==4,"Original record layout");
static_assert(offsetof(CWindModifiers,m_pos)==0 && offsetof(CWindModifiers,m_type)==12,"Original field offsets");
extern "C" void TestWind_Register(WindModifiersVector pos,int32_t type)
{
    // Enter the whole original algorithm with its original value construction
    CWindModifiers::RegisterOne(CVector(pos.x,pos.y,pos.z),type);
}
extern "C" bool TestWind_Find(WindModifiersVector pos,float *x,float *y)
{
    // Enter the whole original search with the same pointer aliases
    return CWindModifiers::FindWindModifier(CVector(pos.x,pos.y,pos.z),x,y);
}
extern "C" int32_t *TestWind_Count(void)
{
    // Return the original live count without copying its state
    return &CWindModifiers::Number;
}
extern "C" WindModifiersRecord TestWind_Record(int32_t index)
{
    // Copy the original complete logical record for a byte comparison
    WindModifiersRecord value={{Array[index].m_pos.x,Array[index].m_pos.y,Array[index].m_pos.z},Array[index].m_type};return value;
}
''',newline='\n')
legacy=commonInclude+'\n#include "WindModifiers_backend.h"\n#include "caller.h"\n#ifdef ORIGINAL_WIND\n#include "original-WindModifiers.h"\n#endif\n'
legacy+='''extern "C" bool TestWind_LegacyFind(const void *position,float *x,float *y)
{
    // Exercise the production typed-reader call pattern with aliased caller coordinates
#ifdef ORIGINAL_WIND
    return CWindModifiers::FindWindModifier(*(const CVector *)position,x,y);
#else
    return WindModifiers_FindWindModifier(WindModifiers_BackendVector(position),x,y);
#endif
}
'''
(out/'legacy-caller.cpp').write_text(legacy,newline='\n')
(out/'mock-host.cpp').write_text((root/'src/renderer/WindModifiers.cpp').read_text(),newline='\n')
absolutePath=Path('C:/msys/home/rouziclib/math/ieee754.c')
savedAbsolute=out/'before/external/rouziclib/math/ieee754.c'
if not savedAbsolute.exists():savedAbsolute.parent.mkdir(parents=True,exist_ok=True);savedAbsolute.write_bytes(absolutePath.read_bytes())
assert savedAbsolute.read_bytes()==absolutePath.read_bytes()
absolute=savedAbsolute.read_text();start=absolute.index('float fast_absf(float x)');end=absolute.index('\n}',start)+2
absoluteFunction=absolute[start:end].replace('{','{\n\t// Execute the complete original sign-bit operation from the external C dependency',1)
(out/'fast-absf.c').write_text('#include <stdint.h>\n'+absoluteFunction+'\n',newline='\n')
(out/'test.cpp').write_text((root/'utils/tests/wind.cpp.in').read_text().replace('@COMMON@',commonInclude).replace('@CAMERA@',cameraInclude),newline='\n')
for name,extra in {'default':'','vanilla32':'#define VANILLA_DEFINES\n'}.items():
    (out/(name+'.h')).write_text('#pragma once\n'+extra+'#include "config.h"\n',newline='\n')
print('Audited both complete wind algorithms, four exact vector helpers, two typed readers, six caller expressions, 16-byte records and ten full source/project files')
