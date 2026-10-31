"""Prepare whole original/C plugin producers with real animation dependencies and SDK boundary fixtures."""
from pathlib import Path
import re
import anim_plugin_migration as m
import frame_update_migration as frames
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/anim-plugin-c-tests';probe=out/'runtime';probe.mkdir(exist_ok=True);saved=out/'before';tree=out/'overlay'
for target,source in [('owners.h','anim_plugin_owners.h.in'),('fixture.cpp','anim_plugin_fixture.cpp.in')]:
    # Generate independent SDK records and scenarios without changing the audited owner source trees
    (probe/target).write_text((root/'utils/tests'/source).read_text(),newline='\n')
(probe/'config.h').write_text('#pragma once\n#ifndef CLASSIC\n#define FIX_BUGS\n#endif\n',newline='\n')
entry='''#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "AnimBlendAssociation.h"
#ifdef __cplusplus
extern "C" {
#endif
bool Run_Attach(void);
void Run_Init(void *clump);
bool Run_Initialized(void *clump);
void Run_Fill(void *clump,AnimBlendFrameData **frames);
AnimBlendFrameData *Run_FindFrame(void *clump,const char *name);
AnimBlendFrameData *Run_FindBone(void *clump,uint32_t tag);
CAnimBlendAssociation *Run_Query(unsigned operation,void *clump,CAnimBlendAssociation *association,uint32_t mask,int n,CAnimBlendAssociation **second,float *blend);
void Run_Deltas(void *clump,uint32_t mask,float delta);
void Run_Update(void *clump,float delta,bool render);
void Run_Allocate(void *clump);
void Run_Remove(void *clump,uint32_t mask,bool all);
#ifdef __cplusplus
}
#endif
'''
(probe/'entry.h').write_text(entry,newline='\n')
queries=[('RpAnimBlendGetNextAssociation(association)','RpAnimBlendGetNextAssociation(association)'),('RpAnimBlendGetNextAssociation(association,mask)','RpAnimBlendGetNextAssociationWithMask(association,mask)'),('RpAnimBlendClumpGetFirstAssociation(clump)','RpAnimBlendClumpGetFirstAssociation(clump)'),('RpAnimBlendClumpGetFirstAssociation(clump,mask)','RpAnimBlendClumpGetFirstAssociationWithMask(clump,mask)'),('RpAnimBlendClumpGetMainAssociation(clump,second,blend)','RpAnimBlendClumpGetMainAssociation(clump,second,blend)'),('RpAnimBlendClumpGetMainPartialAssociation(clump)','RpAnimBlendClumpGetMainPartialAssociation(clump)'),('RpAnimBlendClumpGetMainAssociation_N(clump,n)','RpAnimBlendClumpGetMainAssociation_N(clump,n)'),('RpAnimBlendClumpGetMainPartialAssociation_N(clump,n)','RpAnimBlendClumpGetMainPartialAssociation_N(clump,n)'),('RpAnimBlendClumpGetAssociation(clump,mask)','RpAnimBlendClumpGetAssociation(clump,mask)'),('RpAnimBlendClumpGetAssociation(clump,(uint32_t)n)','RpAnimBlendClumpGetAssociation(clump,(uint32_t)n)')]
for phase in ['before','after']:
    # Compare all public operations through the same C entry declarations despite the original overloaded C++ interface
    prefix='#include "owners.h"\n#include "entry.h"\n' if phase=='before' else '#include "RpAnimBlend.h"\n#include "entry.h"\n'
    pointer='RpClump *clump=(RpClump*)opaque;' if phase=='before' else 'void *clump=opaque;'
    operations=[('bool','Attach','void','return RpAnimBlendPluginAttach();'),('void','Init','void *opaque',pointer+'RpAnimBlendClumpInit(clump);'),('bool','Initialized','void *opaque',pointer+'return RpAnimBlendClumpIsInitialized(clump);'),('void','Fill','void *opaque,AnimBlendFrameData **frames',pointer+'RpAnimBlendClumpFillFrameArray(clump,frames);'),('AnimBlendFrameData *','FindFrame','void *opaque,const char *name',pointer+'return RpAnimBlendClumpFindFrame(clump,name);'),('AnimBlendFrameData *','FindBone','void *opaque,uint32_t tag',pointer+'return RpAnimBlendClumpFindBone(clump,tag);'),('void','Deltas','void *opaque,uint32_t mask,float delta',pointer+'RpAnimBlendClumpSetBlendDeltas(clump,mask,delta);'),('void','Update','void *opaque,float delta,bool render',pointer+'RpAnimBlendClumpUpdateAnimations(clump,delta,render);')]
    source=prefix
    for result,name,args,body in operations:
        # Dispatch to complete real owner entry points rather than duplicating their algorithms
        source+=result+' Run_'+name+'('+args+')\n{\n    // Invoke the original or C plugin operation through its public boundary\n    '+body+'\n}\n'
    source+='void Run_Allocate(void *opaque)\n{\n    // Invoke complete plugin storage initialization through the public boundary\n    '+pointer+'RpAnimBlendAllocateData(clump);\n}\n'
    source+='void Run_Remove(void *opaque,uint32_t mask,bool all)\n{\n    // Retain distinct complete all-association and masked removal entry points\n    '+pointer+'if(all)RpAnimBlendClumpRemoveAllAssociations(clump);else RpAnimBlendClumpRemoveAssociations(clump,mask);\n}\n'
    source+='CAnimBlendAssociation *Run_Query(unsigned operation,void *opaque,CAnimBlendAssociation *association,uint32_t mask,int n,CAnimBlendAssociation **second,float *blend)\n{\n    // Keep each original overload distinct while crossing the independent C entry boundary\n    '+pointer+'switch(operation){\n'+''.join('    case '+str(i)+':return '+row[0 if phase=='before' else 1]+';\n' for i,row in enumerate(queries))+'    }return NULL;\n}\n'
    (probe/('before-caller.cpp' if phase=='before' else 'after-caller.c')).write_text(source,newline='\n')
original=re.sub(r'^#include[^\n]*\n','',(saved/'src/animation/RpAnimBlend.cpp').read_text(),flags=re.M)
(probe/'original.cpp').write_text('#include "owners.h"\n'+original,newline='\n')
native=re.sub(r'^#include[^\n]*\n','',m.native((saved/'src/animation/RpAnimBlend.cpp').read_text()),flags=re.M)
(probe/'native.cpp').write_text('#include "owners.h"\n#include "RpAnimBlend_backend.h"\n'+native,newline='\n')
nativeFrames=re.sub(r'^#include[^\n]*\n','',frames.native(),flags=re.M)
(probe/'frame-native.cpp').write_text('#include "owners.h"\n#include "FrameUpdate_backend.h"\n'+nativeFrames,newline='\n')
print('Prepared entire original/C plugin producers and independent public query/lifecycle scenarios')
