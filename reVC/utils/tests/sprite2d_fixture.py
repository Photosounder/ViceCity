"""Prepare whole original and C 2D sprite producers with exact current SDK peer primitives."""
from pathlib import Path
import re
from memory_heap_migration import body
from sprite2d_schema import signatures
from rect_migration import mask
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/sprite2d-c-tests';probe=out/'runtime';probe.mkdir(exist_ok=True);saved=out/'before';tree=out/'overlay';original=(saved/'src/renderer/Sprite2d.cpp').read_text();owners=(root/'utils/tests/sprite2d_sdk.h.in').read_text()
constants=sorted(set(re.findall(r'\brw(?:RENDERSTATE\w+|SHADEMODE\w+|BLEND\w+|FILTER\w+|PRIMTYPE\w+)\b',original)))
for index,name in enumerate(constants):owners+='enum {'+name+'='+str(200+index)+'};\n'
(probe/'owners.h').write_text(owners,newline='\n');(probe/'original-Sprite2d.h').write_text((saved/'src/renderer/Sprite2d.h').read_text().replace('class CSprite2d','struct CSprite2d'),newline='\n')
(probe/'original.cpp').write_text('#include "owners.h"\n#include "RenderBuffer.h"\n#include "original-Sprite2d.h"\n'+re.sub(r'^#include[^\n]*\n','',original,flags=re.M),newline='\n')
native=(tree/'src/renderer/Sprite2d_native.cpp').read_text();(probe/'native.cpp').write_text('#include "owners.h"\n#include "Sprite2d_backend.h"\n#include "RenderBuffer.h"\n'+re.sub(r'^#include[^\n]*\n','',native,flags=re.M),newline='\n')
peerNames={'rw/RwHelper_native.cpp':['RwHelper_HostVertexAt','RwHelper_HostRenderState']+['RwHelper_HostVertex'+name for name in ['ScreenX','ScreenY','ScreenZ','CameraZ','RecipCameraZ','U','V','IntRGBA']],'renderer/Sprite_native.cpp':['Sprite_HostSceneCamera','Sprite_HostNearClip','Sprite_HostNearScreenZ','Sprite_HostDraw'],'renderer/Shadow_native.cpp':['Shadow_HostTextureDestroy','Shadow_HostTextureAddress']}
code='#include "owners.h"\n#include "Sprite2d_backend.h"\n'
for path,names in peerNames.items():
    # Retain each complete actual C peer primitive rather than replacing the dependency with a policy stub
    source=(tree/'src'/path).read_text();masked=mask(source)
    for name in names:
        # Extract the exact native definition and its SDK argument conversions from the verified preceding owner
        match=re.search(r'(?m)^[\w *]+\b'+name+r'\([^{};]*\)\s*\{',masked);assert match,(path,name);declaration=source[match.start():match.end()-1].strip();code+=declaration+'\n{'+body(source,declaration)+'}\n'
(probe/'peers-native.cpp').write_text(code,newline='\n');print('Prepared both complete producers and',sum(map(len,peerNames.values())),'exact SDK peer primitive definitions')

# Keep public calls independently compiled from the complete original and C producer bodies
from rect_migration import arguments
rows=signatures(original);ops=['Init','Deinit','GetVertices']+[r[1] for r in rows];floats=['x','y','w','h','x1','y1','x2','y2','x3','y3','x4','y4','u0','v0','u1','v1','u3','v3','u2','v2']
entry='#pragma once\n#include <stdint.h>\n#include <stdbool.h>\nenum {'+','.join('SD_'+name for name in ops)+'};\nenum {'+','.join('DF_'+name for name in floats)+'};\ntypedef struct Sprite2dArgs {void *self,*vertices,*colourOwner[4];const uint8_t *colours[4][4];const void *rect;float f[24],*positions,*uvs;int32_t count,address;const char *name,*mask;} Sprite2dArgs;\ntypedef struct Sprite2dResult {void *pointer;bool flag;} Sprite2dResult;\ntypedef struct Sprite2dState {float reciprocal,nearDepth,nearCamera;int32_t vertices,indices;void *immediate;} Sprite2dState;\n#ifdef __cplusplus\nextern "C" {\n#endif\nvoid TestSprite2d(unsigned operation,Sprite2dArgs *args,Sprite2dResult *result);\nvoid TestSprite2dState(Sprite2dState *state);\nvoid TestSprite2dInstance(void *self,void **texture);\n#ifdef __cplusplus\n}\n#endif\n'
(probe/'sprite2d-entry.h').write_text(entry,newline='\n')
for before in [True,False]:
    # Use exact original native references or actual C pointer views in separate language callers
    text=('#include "owners.h"\n#include "original-Sprite2d.h"\n#include <new>\n' if before else '#include "Sprite2d.h"\n')+'#include "sprite2d-entry.h"\nvoid TestSprite2d(unsigned operation,Sprite2dArgs *args,Sprite2dResult *result)\n{\n    // Enter complete application operations without reproducing their rendering resource or clipping decisions\n    '+('CSprite2d' if before else 'Sprite2d')+' *self=('+('CSprite2d' if before else 'Sprite2d')+'*)args->self;result->pointer=NULL;result->flag=false;switch(operation){\n'
    for op in ops:
        # Retain construction destruction overload selection and each public scalar argument order
        if op=='Init':stmt='new(self) CSprite2d();' if before else 'Sprite2d_Init(self);'
        elif op=='Deinit':stmt='self->~CSprite2d();' if before else 'Sprite2d_Deinit(self);'
        elif op=='GetVertices':stmt='result->pointer='+('CSprite2d::GetVertices()' if before else 'Sprite2d_GetVertices()')+';'
        else:
            row=next(r for r in rows if r[1]==op);kind,name,params,old,raw,*_=row;values=[]
            for param in ([] if raw.strip() in ['', 'void'] else arguments(raw)):
                # Marshal only original representation differences while keeping actual caller-owned colour and rectangle storage
                arg=re.search(r'(\w+)\s*$',param)[1]
                if 'const CRGBA &' in param:
                    index=int(arg[1]) if re.fullmatch(r'c[0-3]',arg) else 0;value='*(const CRGBA*)args->colourOwner['+str(index)+']' if before else 'Sprite_ColourRef('+','.join('args->colours['+str(index)+']['+str(i)+']' for i in range(4))+')'
                elif 'const CRect &' in param:value='*(const CRect*)args->rect' if before else '(const CRect*)args->rect'
                elif arg in floats:value='args->f[DF_'+arg+']'
                elif arg=='verts':value='(RwIm2DVertex*)args->vertices' if before else 'args->vertices'
                elif arg in ['positions','uvs','name','mask']:value='args->'+arg
                elif arg=='n':value='args->count'
                elif arg=='addr':value='(RwTextureAddressMode)args->address' if before else 'args->address'
                else:raise AssertionError((op,param))
                values.append(value)
            instance=name in __import__('sprite2d_schema').INSTANCE;expr=(('self->' if instance else 'CSprite2d::')+old+'('+','.join(values)+')') if before else 'Sprite2d_'+name+'('+','.join((['self'] if instance else [])+values)+')';stmt=('result->flag=' if kind=='bool' else '')+expr+';'
        text+='case SD_'+op+':'+stmt+'break;\n'
    text+='}}\nvoid TestSprite2dInstance(void *record,void **texture)\n{\n    // Observe the sole live instance field only while its original or C object lifetime is active\n    *texture=(('+('CSprite2d' if before else 'Sprite2d')+'*)record)->m_pTexture;\n}\n';(probe/('before-caller.cpp' if before else 'after-caller.c')).write_text(text,newline='\n')
    stateNames=['RecipNearClip','NearScreenZ','NearCamZ','nextBufferVertex','nextBufferIndex'];adapter='\n#include "sprite2d-entry.h"\nvoid TestSprite2dState(Sprite2dState *state)\n{\n    // Read actual private producer state and retain the identity of the original SDK vertex array\n'+''.join('    state->'+field+'='+('CSprite2d::' if before else 'Sprite2d_')+name+';\n' for field,name in zip(['reciprocal','nearDepth','nearCamera','vertices','indices'],stateNames))+'    state->immediate='+('CSprite2d::maVertices' if before else 'Sprite2d_HostVertices()')+';\n}\n'
    producer=(probe/'original.cpp').read_text() if before else (tree/'src/renderer/Sprite2d.c').read_text();(probe/('original-owner.cpp' if before else 'after-owner.c')).write_text(producer+adapter,newline='\n')

# Keep controlled SDK observation and independent operation cases separate from the complete producers
(probe/'fixture.cpp').write_text((root/'utils/tests/sprite2d_sdk.cpp.in').read_text()+'\n'+(root/'utils/tests/sprite2d_cases.cpp.in').read_text(),newline='\n')
