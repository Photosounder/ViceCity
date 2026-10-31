"""Prepare complete original/C sprite producers and independent public language callers."""
from pathlib import Path
import re,json
from sprite_schema import signatures,OPERATIONS
from rect_migration import arguments
import sprite_peers as peers
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/sprite-c-tests';probe=out/'runtime';probe.mkdir(exist_ok=True);saved=out/'before';tree=out/'overlay';prior=root/'.diagnostics/rw-helper-c-tests/runtime';source=(saved/'src/renderer/Sprite.cpp').read_text();rows=signatures(source)
FLOATS=['x','y','z','w','h','recipz','rotation','cx','cy','x1','y1','x2','y2','x3','y3','x4','y4'];BYTES={'r':0,'g':1,'b':2,'a':3,'alpha':3,'r1':0,'g1':1,'b1':2,'r2':4,'g2':5,'b2':6}
owners=(prior/'owners.h').read_text();owners=re.sub(r'#ifdef ORIGINAL\n#include "original-api.h"\n#else\n#include "RwHelper.h"\n#endif','#include "RwHelper.h"',owners)
first=owners.index('struct CVector {');last=owners.index('struct TestGlobal',first)
owners=owners[:first]+"""struct CVector:public RwV3d {
    CVector()=default;
    CVector(float a,float b,float c){
        // Retain the native vector constructor's three scalar assignments
        x=a;y=b;z=c;
    }
    CVector(const RwV3d &value){
        // Preserve the actual SDK-to-native vector conversion
        x=value.x;y=value.y;z=value.z;
    }
};
"""+owners[last:]
owners=owners.replace('RwV2d viewWindow;','RwV2d viewWindow;float nearClip;')+'\n#include "sprite-sdk.h"\n'
(probe/'owners.h').write_text(owners,newline='\n')
for name in ['entry.h','after-caller.c']:
    # Keep the complete rendering-helper peer independently compiled for actual native vertex primitives
    (probe/name).write_bytes((prior/name).read_bytes())
for name,relative,header in [('helper-native.cpp','rw/RwHelper_native.cpp','RwHelper_backend.h'),('sprite-native.cpp','renderer/Sprite_native.cpp','Sprite_backend.h')]:
    # Preserve the entire actual native primitive implementation and replace only SDK include routing
    value=(tree/'src'/relative).read_text();(probe/name).write_text('#include "owners.h"\n#include "'+header+'"\n'+re.sub(r'^#include[^\n]*\n','',value,flags=re.M),newline='\n')
(probe/'original-Sprite.h').write_text((saved/'src/renderer/Sprite.h').read_text().replace('class CSprite','struct CSprite'),newline='\n')
entry="""#pragma once
#include <stdint.h>
#include <stdbool.h>
enum {"""+','.join('SP_'+name for name in OPERATIONS)+""",SP_PeerCloudProject,SP_PeerLightProject,SP_PeerMoneyProject,SP_PeerScrollProject,SP_PeerScrollPoints,SP_PeerCloudInit,SP_PeerCloudFlush,SP_PeerCloudOne,SP_PeerCloudBuffered,SP_PeerCloudDimension,SP_PeerCloudAspect,SP_PeerCloudColours,SP_PeerLightInit,SP_PeerLightFlush,SP_PeerLightSprite,SP_PeerScrollInit,SP_PeerScrollFlush,SP_PeerScrollSprite,SP_COUNT};
enum {"""+','.join('SF_'+name for name in FLOATS)+"""};
typedef struct SpriteArgs {float f[24];uint8_t bytes[8];int16_t intensity;void *inputOwner,*outputOwner,*vertices,*rect,*colourOwner[4];void *sdkColourOwner;const uint8_t *sdkColour[4];float *input[3],*output[3];const uint8_t *colours[4][4];float *width,*height;bool farclip;} SpriteArgs;
typedef struct SpriteResult {float value;bool visible;} SpriteResult;
typedef struct SpriteState {float nearDepth,farDepth,reciprocal;int32_t switchZ,index;void *buffered,*immediate;} SpriteState;
#ifdef __cplusplus
extern "C" {
#endif
void TestSprite(unsigned operation,SpriteArgs *args,SpriteResult *result);
void GetTestSpriteState(SpriteState *state);
void TestSpritePeers(unsigned operation,SpriteArgs *args,SpriteResult *result);
void FixturePoints(float *x,float *y,float *z,float *outX,float *outY,float *outZ,void *input,void *output);
#ifdef __cplusplus
}
#endif
"""
(probe/'sprite-entry.h').write_text(entry,newline='\n')
for original in [True,False]:
    # Enter each entire operation through the original native API or a separate genuine C API caller
    value=('#include "owners.h"\n#include "original-Sprite.h"\n' if original else '#include "Sprite.h"\n')+'#include "sprite-entry.h"\nvoid TestSprite(unsigned operation,SpriteArgs *args,SpriteResult *result)\n{\n    // Invoke complete producer behavior rather than reproducing clipping projection or render decisions\n    result->value=0;result->visible=false;switch(operation){\n'
    for op in OPERATIONS:
        # Convert argument representation only at the independent original/C language boundary
        params=next((r[2] for r in rows if r[1]==op),'void');call=[]
        for param in ([] if params=='void' else arguments(params)):
            name=re.search(r'(\w+)\s*$',param)[1]
            if 'CRect' in param:arg='*(CRect*)args->rect' if original else '(const CRect*)args->rect'
            elif name in FLOATS:arg='args->f[SF_'+name+']'
            elif name in BYTES:arg='args->bytes['+str(BYTES[name])+']'
            elif name in ['intens','intensity']:arg='args->intensity'
            elif name=='verts':arg='(RwIm2DVertex*)args->vertices' if original else 'args->vertices'
            elif name=='r':arg='*(CRect*)args->rect' if original else '(const CRect*)args->rect'
            elif name=='in':arg='*(RwV3d*)args->inputOwner' if original else 'Sprite_VectorRef(args->input[0],args->input[1],args->input[2])'
            elif name=='out':arg='(RwV3d*)args->outputOwner' if original else 'Sprite_VectorOut(args->output[0],args->output[1],args->output[2])'
            elif name in ['outw','outh']:arg='args->'+('width' if name=='outw' else 'height')
            elif name=='farclip':arg='args->farclip'
            elif name=='colour':arg='*(RwRGBA*)args->sdkColourOwner' if original else 'Sprite_ColourRef(args->sdkColour[0],args->sdkColour[1],args->sdkColour[2],args->sdkColour[3])'
            elif re.fullmatch('c[0-3]',name):
                # Keep colour object identity through the entire original reference or C scalar-reference call
                index=name[1];arg='*(CRGBA*)args->colourOwner['+index+']' if original else 'Sprite_ColourRef('+','.join('args->colours['+index+']['+str(i)+']' for i in range(4))+')'
            else:raise AssertionError((op,param,name))
            call.append(arg)
        actual='Set6Vertices2D' if original and op.startswith('Set6Vertices2D') else op;kind=next((r[0] for r in rows if r[1]==op),'float');expr=('CSprite::' if original else 'Sprite_')+actual+'('+','.join(call)+')';value+='case SP_'+op+':'+('result->value=' if kind=='float' else 'result->visible=' if kind=='bool' else '')+expr+';break;\n'
    value+='}}\n';(probe/('before-caller.cpp' if original else 'sprite-caller.c')).write_text(value,newline='\n')
    globals=['CSprite::'+name for name in ['m_f2DNearScreenZ','m_f2DFarScreenZ','m_fRecipNearClipPlane','m_bFlushSpriteBufferSwitchZTest']]+['nSpriteBufferIndex'] if original else ['Sprite_'+name for name in ['m_f2DNearScreenZ','m_f2DFarScreenZ','m_fRecipNearClipPlane','m_bFlushSpriteBufferSwitchZTest','nSpriteBufferIndex']]
    adapter='\n#include "sprite-entry.h"\nvoid GetTestSpriteState(SpriteState *state)\n{\n    // Observe only actual private producer state and the identities of its original SDK buffers\n'+''.join('    state->'+field+'='+name+';\n' for field,name in zip(['nearDepth','farDepth','reciprocal','switchZ','index'],globals))+'    state->buffered='+('SpriteBufferVerts' if original else 'Sprite_HostBufferedVertices()')+';state->immediate='+('verts' if original else 'Sprite_HostImmediateVertices()')+';\n}\n'
    producer='#include "owners.h"\n#include "original-Sprite.h"\n'+re.sub(r'^#include[^\n]*\n','',source,flags=re.M) if original else (tree/'src/renderer/Sprite.c').read_text()
    (probe/('original.cpp' if original else 'after-sprite.c')).write_text(producer+adapter,newline='\n')
originalPeers='#include "owners.h"\n#include "original-Sprite.h"\n#include "Clouds_backend.h"\n#include "PointLights_backend.h"\n#include "MoneyMessages_backend.h"\n#include "ScrollBar_backend.h"\n'
for name in peers.SOURCES:
    # Keep each full original forwarding definition and its native point lifetime for independent C-peer comparison
    for _,signature,value,_,_ in peers.definitions((saved/f'src/renderer/{name}.cpp').read_text()):originalPeers+=signature+'\n{'+value+'}\n'
(probe/'original-peers.cpp').write_text(originalPeers,newline='\n')

# Keep exact declarations for the controlled SDK separately from complete application producer bodies
(probe/'sprite-sdk.h').write_text((root/'utils/tests/sprite_sdk.h.in').read_text(),newline='\n')

# Preserve the full controlled SDK implementation while replacing its draw observer for every sprite primitive kind
fixture=(prior/'fixture.cpp').read_text().replace('int main(void)','int PriorHelperMain(void)')
signature='void RwIm2DRenderPrimitive(int32 kind,RwIm2DVertex *vertices,int32 count)';old=__import__('memory_heap_migration').body(fixture,signature);fixture=fixture.replace(signature+'\n{'+old+'}',signature+'\n{\n    // Observe complete sprite rendering with the actual live SDK vertex array\n    spriteDraw(kind,vertices,count);\n}')
for setter in ['ScreenX','ScreenY','ScreenZ','CameraZ','RecipCameraZ','U','V','IntRGBA']:
    # Observe external setter reentry without changing the original application vertex-construction policy
    pattern=r'(void RwIm2DVertexSet'+setter+r'\([^{}]*\)\s*\{)';fixture=re.sub(pattern,r'\1\n    // Keep caller-owned scalar mutations visible between original SDK setter calls\n    spriteSetter();',fixture)
fixture='#include <initializer_list>\n#include "owners.h"\nstatic void spriteDraw(int32,RwIm2DVertex*,int32);\nstatic void spriteSetter(void);\n'+fixture+'\n'+(root/'utils/tests/sprite_sdk.cpp.in').read_text()+'\n'+(root/'utils/tests/sprite_cases.cpp.in').read_text()
(probe/'fixture.cpp').write_text(fixture,newline='\n')
