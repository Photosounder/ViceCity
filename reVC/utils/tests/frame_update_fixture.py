"""Prepare complete original/C frame callbacks with real converted animation dependencies."""
from pathlib import Path
import re
import frame_update_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/frame-update-c-tests';probe=out/'runtime';probe.mkdir(exist_ok=True)
before=out/'before';after=out/'overlay'
for name in ['config.h']:
    # Exercise the same explicit FIX_BUGS and compression profiles used by earlier frame-consumer comparisons
    (probe/name).write_text('#pragma once\n#ifndef CLASSIC\n#define FIX_BUGS\n#endif\n',newline='\n')
owners=(root/'.diagnostics/anim-group-c-tests/owners.h').read_text().split('\n#include <ctype.h>')[0]
old='struct AnimBlendFrameUpdateData {int foobar;CAnimBlendNode *nodes[16];};\nextern CAnimBlendClumpData *gpAnimBlendClump;'
assert old in owners
owners=owners.replace(old,'#ifdef ORIGINAL\n'+old+'\n#else\n#include "AnimBlendFrameUpdate.h"\n#endif')
owners=owners.replace('static inline RwMatrix *RwFrameGetMatrix', '#ifdef FRAME_MUTATIONS\nvoid FrameFixture_Mutate(unsigned event);\n#endif\nstatic inline RwMatrix *RwFrameGetMatrix')
owners=owners.replace('return &frame->matrix;', '#ifdef FRAME_MUTATIONS\n    // Allow the SDK primitive to change live inputs before the callback reads them\n    FrameFixture_Mutate(1);\n#endif\n    return &frame->matrix;')
owners=owners.replace('matrix->flags=1;', 'matrix->flags=1;\n#ifdef FRAME_MUTATIONS\n    // Change live owner and flag inputs at the original identity boundary\n    FrameFixture_Mutate(2);\n#endif')
owners=owners.replace('matrix->flags+=16;', 'matrix->flags+=16;\n#ifdef FRAME_MUTATIONS\n    // Observe the final SDK boundary and its externally visible mutations\n    FrameFixture_Mutate(3);\n#endif')
(probe/'owners.h').write_text(owners,newline='\n')
entry='''#pragma once
#ifdef __cplusplus
extern "C" {
#endif
void Run_FrameCallback(unsigned operation,void *frame,void *argument);
void Before_FrameCallback(unsigned operation,void *frame,void *argument);
#ifdef __cplusplus
}
#endif
'''
(probe/'entry.h').write_text(entry,newline='\n')
order=['NonSkinned','WithVelocityExtractionNonSkinned','With3dVelocityExtractionNonSkinned','Skinned','WithVelocityExtractionSkinned','With3dVelocityExtractionSkinned','NonSkinnedCompressed','SkinnedCompressed','Offscreen']
caller='''#include "entry.h"
#ifndef ORIGINAL
#include "AnimBlendFrameUpdate.h"
#endif
void Run_FrameCallback(unsigned operation,void *frame,void *argument)
{
    // Cross an independent C11 call boundary into each complete original or converted frame algorithm
#ifdef ORIGINAL
    Before_FrameCallback(operation,frame,argument);
#else
    switch(operation){
'''+''.join('    case '+str(i)+':FrameUpdateCallBack'+name+'((AnimBlendFrameData*)frame,argument);break;\n' for i,name in enumerate(order))+'''    }
#endif
}
'''
(probe/'caller.c').write_text(caller,newline='\n')
original=re.sub(r'^#include[^\n]*\n','',(before/'src/animation/FrameUpdate.cpp').read_text(),flags=re.M)
wrapper='''
#include "entry.h"
extern "C" void Before_FrameCallback(unsigned operation,void *frame,void *argument)
{
    // Invoke entire original callbacks without replacing their accumulation or SDK access policy
    switch(operation){
'''+''.join('    case '+str(i)+':FrameUpdateCallBack'+name+'((AnimBlendFrameData*)frame,argument);break;\n' for i,name in enumerate(order))+'''    }
}
'''
(probe/'original.cpp').write_text('#include "owners.h"\n'+original+wrapper,newline='\n')
native=re.sub(r'^#include[^\n]*\n','',m.native(),flags=re.M)
(probe/'native.cpp').write_text('#include "owners.h"\n#include "FrameUpdate_backend.h"\n'+native,newline='\n')
fixture=(root/'.diagnostics/anim-association-c-tests/frame-fixture.cpp').read_text()
fixture=fixture.replace('AssociationInput init={};init.self=&associations[channel];Run_Association(0,init);','AnimAssociation_Init(&associations[channel]);')
fixture=fixture.replace('callbacks[which](&data,&update);','Run_FrameCallback(which,&data,&update);')
mutation='''
#ifdef FRAME_MUTATIONS
static AnimBlendFrameData *mutationFrame;
static AnimBlendFrameUpdateData *mutationDispatch;
static CAnimBlendClumpData *mutationOwner;
static unsigned mutationCase,mutationEvents;
void FrameFixture_Mutate(unsigned event)
{
    // Change actual live objects only at native SDK calls that the original callbacks also perform
    ++mutationEvents;
    if(event==1){
        // Preserve velocity dimensionality while changing later flag and blend-dispatch reads
        mutationFrame->flag^=ANIM_FRAME_IGNORE_ROTATION;mutationDispatch->foobar^=1;
        mutationFrame->resetPos.x+=0.125f;
        if(mutationCase&512)gpAnimBlendClump=mutationOwner;
    }else if(event==2){
        // Require translation guards and velocity stores to observe the current owner after rotation work
        mutationFrame->flag^=ANIM_FRAME_IGNORE_TRANSLATION;mutationFrame->resetPos.z-=0.25f;
        gpAnimBlendClump=mutationOwner;
    }else{
        // Keep the terminal native mutation observable without injecting extra frame policy
        mutationFrame->resetPos.y+=0.5f;
    }
}
#endif
'''
fixture=fixture.replace('static void animation()\n',mutation+'\nstatic void animation()\n')
call='gpAnimBlendClump=&clump;Run_FrameCallback(which,&data,&update);'
replacement='''gpAnimBlendClump=&clump;
#ifdef FRAME_MUTATIONS
        // Keep both possible native velocity storage dimensions valid during SDK owner redirection
        CVector2D alternate2(-0.25f,0.875f);CVector alternate3(-0.25f,0.875f,-0.5f);CAnimBlendClumpData alternate={};
        if(which==2||which==5||which==6||which==7||((which==0||which==3)&&(test&12)==12))alternate.velocity3d=&alternate3;else alternate.velocity2d=&alternate2;
        mutationFrame=&data;mutationDispatch=&update;mutationOwner=&alternate;mutationCase=test;
#endif
        Run_FrameCallback(which,&data,&update);
#ifdef FRAME_MUTATIONS
        // Compare changed SDK inputs and actual alternate storage without hashing pointer addresses
        require(gpAnimBlendClump==&clump||gpAnimBlendClump==&alternate);unsigned changed=gpAnimBlendClump==&alternate;observe(&changed,4);observe(&data.flag,1);vectorResult(KeyFrame_ToNative(data.resetPos));observe(&update.foobar,4);vectorResult(alternate3);canonical(alternate2.x);canonical(alternate2.y);observe(&alternate2,8);observe(&mutationEvents,4);
#endif
        '''
assert call in fixture;fixture=fixture.replace(call,replacement)
start=fixture.index('extern "C" uint64_t Association_FrameConsumers(void)')
fixture=fixture[:start]+'''int main(void)
{
    // Compare every actual frame callback, node advance, native output and source keyframe observation
    animation();require(states==18432);printf("frames=%u checks=%u hash=%016" PRIx64 "\\n",states,checks,hashValue);return 0;
}
'''
assert 'Run_Association' not in fixture
(probe/'fixture.cpp').write_text(fixture,newline='\n')
primitives='''#include "owners.h"
#include "AnimBlendClumpData_backend.h"
#include "AnimBlendAssociation_backend.h"
#include "AnimBlendSequence_backend.h"
extern "C" void *AnimBlendClump_BackendAllocate(uint32_t size,uint32_t alignment)
{
    // Provide the actual aligned allocation primitive for complete linked association and clump owners
    return _aligned_malloc(size?size:64,alignment);
}
extern "C" void AnimBlendClump_BackendFree(void *memory)
{
    // Release storage through the matching native primitive without frame policy
    _aligned_free(memory);
}
extern "C" CAnimBlendClumpData *AnimAssociation_HostClumpData(void *clump)
{
    // Borrow the existing live fixture plugin owner without moving association binding policy
    return static_cast<RpClump*>(clump)->data;
}
extern "C" AnimBlendFrameData *AnimAssociation_HostFindFrame(void*,const char*)
{
    // Reject unrelated SDK binding operations during these complete frame accumulation scenarios
    abort();return NULL;
}
extern "C" AnimBlendFrameData *AnimAssociation_HostFindBone(void*,uint32_t)
{
    // Reject unrelated native bone lookup instead of synthesizing frame policy
    abort();return NULL;
}
extern "C" void AnimAssociation_HostUncompressAnimation(CAnimBlendHierarchy*)
{
    // Reject manager playback setup while retaining the real association predicates and node algorithms
    abort();
}
extern "C" void *AnimSequence_BackendAllocate(size_t bytes)
{
    // Supply the same native buffer primitive for the complete linked sequence implementation
    return malloc(bytes);
}
extern "C" void AnimSequence_BackendFree(void *memory)
{
    // Preserve the matching native sequence buffer release primitive
    free(memory);
}
extern "C" void AnimSequence_BackendPushAnimation(void) {}
extern "C" void AnimSequence_BackendPopMemory(void) {}
extern "C" void AnimSequence_BackendRegister(void**) {}
'''
(probe/'primitives.cpp').write_text(primitives,newline='\n')
print('Prepared entire original/C callbacks with real association, node, sequence, keyframe and quaternion implementations')
