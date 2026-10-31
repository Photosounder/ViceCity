"""Prepare complete original/C rendering-helper owners and independently compiled public callers."""
from pathlib import Path
import re,json
import rw_helper_migration as m
from rw_helper_schema import OPAQUE
from mblur_schema import SETTERS
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/rw-helper-c-tests';probe=out/'runtime';probe.mkdir(exist_ok=True);original=(out/'before/src/rw/RwHelper.cpp').read_text();operations=json.loads((out/'stage-summary.json').read_text())['operations'];operations=[v for v in operations if v not in ['GetAnimHierarchyCallback','GetAnimHierarchyFromClumpCB']]
owners=(root/'utils/tests/rw_helper_owners.h.in').read_text()+'\ntypedef int32 RwRenderState;\n'
_,constants=m.kernel(original)
for index,name in enumerate(constants+['rwPRIMTYPETRISTRIP']):
    # Give both producers the same controlled SDK identities without embedding those identities in C policy
    value=1 if name=='rpHANIMPUSHPARENTMATRIX' else 2 if name=='rpHANIMPOPPARENTMATRIX' else 4 if name=='rwVIDEOMODEEXCLUSIVE' else 0 if name=='rpMATFXEFFECTNULL' else 100+index
    owners+='enum {'+name+'='+str(value)+'};\n'
for name,(args,_) in SETTERS.items():owners+='void RwIm2DVertexSet'+name+'(RwIm2DVertex *vertex,'+args+');\n'
owners+='''#ifdef ORIGINAL
#include "original-api.h"
#else
#include "RwHelper.h"
#endif
'''
entry='''#pragma once
#include <stdint.h>
#include <stdbool.h>
enum {'''+','.join('OP_'+name for name in operations)+'''};
typedef struct TestHelperArgs {void *object,*data;float values[7];int32_t x,y,z;bool flag;const char *text;} TestHelperArgs;
typedef struct TestHelperResult {void *object,*output;int32_t value;} TestHelperResult;
typedef struct TestHelperState {bool ps2,cull,debug,sphere,blur;} TestHelperState;
#ifdef __cplusplus
extern "C" {
#endif
void TestHelper(unsigned operation,TestHelperArgs *args,TestHelperResult *result);
void TestHelperGetState(TestHelperState *state);
void TestHelperSetState(const TestHelperState *state);
#ifdef __cplusplus
}
#endif
'''
simple=['OpenCharsetSafe','CreateDebugFont','DestroyDebugFont','FlushObrsPrintfs','DefinedState','PopRendergroup','RestoreAlphaTest']
guards={'OpenCharsetSafe':'!defined(FINAL) || defined(DEBUGMENU)','PushRendergroup':'!defined(FINAL)','PopRendergroup':'!defined(FINAL)','findPlatform':'defined(LIBRW)','attachPipe':'defined(LIBRW)','switchPipes':'defined(LIBRW)','ConvertPlatformAtomic':'defined(LIBRW)','SetAlphaTest':'defined(FIX_BUGS) && defined(GTA_PC)','RestoreAlphaTest':'defined(FIX_BUGS) && defined(GTA_PC)'}
types={'GetFirstChild':'RwFrame','GetFirstObject':'RwFrame','GetFirstAtomic':'RpClump','GetFirstTexture':'RwTexDictionary','IsClumpSkinned':'RpClump','GetAnimHierarchyFromSkinClump':'RpClump','GetAnimHierarchyFromClump':'RpClump','RenderSkeleton':'RpHAnimHierarchy','HAnimAnimationCreateForHierarchy':'RpHAnimHierarchy','CameraDestroy':'RwCamera','findPlatform':'rw::Atomic','attachPipe':'rw::Atomic'}
returns={'GetFirstChild','GetFirstObject','GetFirstAtomic','GetFirstTexture','GetAnimHierarchyFromSkinClump','GetAnimHierarchyFromClump','HAnimAnimationCreateForHierarchy','CameraCreate','RpClumpGetBoundingSphere','findPlatform','IsClumpSkinned','Im2DRenderQuad','ConvertPlatformAtomic'}
callbacks={'GetFirstFrameCallback':'RwFrame','GetFirstObjectCallback':'RwObject','GetFirstAtomicCallback':'RpAtomic','GetFirstTextureCallback':'RwTexture'}
for phase in ['before','after']:
    # Keep the public C caller separate from both application policy and the native SDK primitive implementation
    cpp=phase=='before';source=('#include "owners.h"\n' if cpp else '#include "RwHelper.h"\n#include <stddef.h>\n')+'#include "entry.h"\nextern bool b_cbsUseLTM;\n'
    for name in operations:
        # Derive exact original callable signatures while excluding the two truly private static callbacks
        start,_,_=m.definition(original,name);declaration=original[start:original.index('{',start)].strip()
        if not cpp:
            declaration=declaration.replace('rw::Atomic','void')
            for kind in OPAQUE:declaration=re.sub(r'\b'+kind+r'\b','void',declaration)
            for a,b in [('RwBool','int32_t'),('RwUInt32','uint32_t'),('RwInt32','int32_t'),('RwReal','float'),('uint32','uint32_t'),('int32','int32_t')]:declaration=re.sub(r'\b'+a+r'\b',b,declaration)
            if name not in callbacks and name not in ['cbsCalcMeanBSphereRadiusCB','cbsCalcMeanBSphereCenterCB','findPlatform','attachPipe','switchPipes']:continue
        if name in guards:source+='#if '+guards[name]+'\n'
        source+=declaration+';\n'
        if name in guards:source+='#endif\n'
    source+='void TestHelper(unsigned operation,TestHelperArgs *args,TestHelperResult *result)\n{\n    // Enter complete original or C operations without reproducing their policy\n    result->object=result->output=NULL;result->value=0;switch(operation){\n'
    for name in operations:
        # Convert only native argument types in the original caller and retain raw handles in the independent C caller
        if name in guards:source+='#if '+guards[name]+'\n'
        expression=None
        if name in simple:expression=name+'()'
        elif name in types:expression=name+'('+('('+types[name]+'*)' if cpp else '')+'args->object)'
        elif name=='ObrsPrintfString':expression=name+'(args->text,(short)args->x,(short)args->y)'
        elif name=='SetCullMode' or name=='SetAlphaTest':expression=name+'((uint32_t)args->x)'
        elif name=='PushRendergroup':expression=name+'(args->text)'
        elif name=='SkinGetBonePositionsToTable':expression=name+'('+('(RpClump*)' if cpp else '')+'args->object,'+('(RwV3d*)' if cpp else '')+'args->data)'
        elif name=='Im2DRenderQuad':expression=name+'('+','.join('args->values['+str(i)+']' for i in range(7))+')'
        elif name=='RpClumpGetBoundingSphere':expression=name+'('+('(RpClump*)' if cpp else '')+'args->object,'+('(RwSphere*)' if cpp else '')+'args->data,args->flag)'
        elif name=='CameraSize':expression=name+'('+('(RwCamera*)' if cpp else '')+'args->object,'+('(RwRect*)' if cpp else '')+'args->data,args->values[0],args->values[1])'
        elif name=='CameraCreate':expression=name+'(args->x,args->y,args->z)'
        elif name=='switchPipes':expression=name+'('+('(rw::Atomic*)' if cpp else '')+'args->object,args->x)'
        elif name=='ConvertPlatformAtomic':expression=(name if cpp else 'RwHelper_'+name)+'('+('(RpAtomic*)' if cpp else '')+'args->object,args->data)'
        elif name.startswith('cbsCalc'):expression=name+'('+('(RpAtomic*)' if cpp else '')+'args->object,args->data)'
        if name in callbacks:
            kind=callbacks[name] if cpp else 'void';statement='{'+kind+' *value=NULL;result->object='+name+'(('+kind+'*)args->object,&value);result->output=value;}'
        else:
            assert expression,name
            if name=='GetFirstChild' and not cpp:expression=expression.replace('GetFirstChild(','RwHelper_GetFirstChild(')
            prefix='result->value=' if name in ['findPlatform','IsClumpSkinned','Im2DRenderQuad'] else 'result->object=' if name in returns or name.startswith('cbsCalc') else ''
            statement=prefix+expression+';'
        source+='case OP_'+name+':'+statement+'break;\n'
        if name in guards:source+='#endif\n'
    source+='}}\nvoid TestHelperGetState(TestHelperState *state)\n{\n    // Observe original shared state through independently compiled declarations\n    state->ps2=gPS2alphaTest;state->cull=gBackfaceCulling;state->sphere=b_cbsUseLTM;state->blur=MBlur_BlurOn;\n#ifndef FINAL\n    state->debug=bDebugRenderGroups;\n#else\n    state->debug=false;\n#endif\n}\nvoid TestHelperSetState(const TestHelperState *state)\n{\n    // Write only actual application globals rather than shadow copies in the fixture\n    gPS2alphaTest=state->ps2;gBackfaceCulling=state->cull;b_cbsUseLTM=state->sphere;MBlur_BlurOn=state->blur;\n#ifndef FINAL\n    bDebugRenderGroups=state->debug;\n#endif\n}\n'
    if not cpp:source=source.replace('#include <stddef.h>','#include <stddef.h>\n#include "MBlur.h"')
    (probe/(phase+'-caller.'+('cpp' if cpp else 'c'))).write_text(source,newline='\n')
for name,value in [('owners.h',owners),('entry.h',entry),('original-api.h',(out/'before/src/rw/RwHelper.h').read_text()),('original.cpp','#include "owners.h"\n'+re.sub(r'^#include[^\n]*\n','',original,flags=re.M)),('native.cpp','#include "owners.h"\n#include "RwHelper_backend.h"\n'+re.sub(r'^#include[^\n]*\n','',(out/'overlay/src/rw/RwHelper_native.cpp').read_text(),flags=re.M))]:
    # Materialize both complete producers and the complete native boundary without substituting their algorithms
    (probe/name).write_text(value,newline='\n')
template=root/'utils/tests/rw_helper_fixture.cpp.in'
if template.exists():
    # Observe individual SDK setters without copying any quad-construction decisions into the fixture
    fixture=template.read_text();cases=root/'utils/tests/rw_helper_cases.cpp.in'
    if cases.exists():fixture+='\n'+cases.read_text()
    fields={'ScreenX':'x','ScreenY':'y','ScreenZ':'z','RecipCameraZ':'reciprocal','U':'u','V':'v'}
    for index,(name,(args,_)) in enumerate(SETTERS.items()):
        # Preserve every initialized setter argument including reciprocal coordinates
        fixture+='\nvoid RwIm2DVertexSet'+name+'(RwIm2DVertex *vertex,'+args+')\n{\n    // Observe the original SDK setter inputs and initialized vertex subobjects\n    event('+str(100+index)+');'
        if name=='IntRGBA':fixture+='uint8_t values[]={red,green,blue,alpha};bytes(values,sizeof(values));vertex->colour={red,green,blue,alpha};'
        else:
            fixture+='bytes(&value,sizeof(value));'
            if name in fields:fixture+='vertex->'+fields[name]+'=value;'
            if name in ['U','V']:fixture+='bytes(&reciprocal,sizeof(reciprocal));'
        fixture+='\n}\n'
    (probe/'fixture.cpp').write_text(fixture,newline='\n')
print('Prepared all 38 complete original/C operations with independent callers; two static callbacks are reached through their original traversals')
