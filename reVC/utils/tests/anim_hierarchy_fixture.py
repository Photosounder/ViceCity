"""Compare complete hierarchy collection operations with real C sequences and native cache removal."""
from pathlib import Path
import json,re,sys
import anim_hierarchy_migration as m
from doors_migration import method
from stats_successor import current_path
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/anim-hierarchy-c-tests';saved=out/'before';tree=root if '--production' in sys.argv else out/'overlay'
if '--production' in sys.argv:
    for name in json.loads((out/'changes.json').read_text()):assert current_path(root,name).read_text()==m.calls((saved/name).read_text()),name
for name in ['AnimBlendSequence.h','AnimBlendSequence.c','AnimBlendSequence_backend.h','KeyFrames.h','KeyFrames.c']:(out/name).write_text((saved/'src/animation'/name).read_text(),newline='\n')
for name,path in [('Quaternion.h','src/math/Quaternion.h'),('Quaternion.c','src/math/Quaternion.c'),('LinkList.h','src/core/LinkList.h')]:(out/name).write_text((saved/path).read_text(),newline='\n')
(out/'AnimBlendHierarchy.h').write_text('#pragma once\n#ifdef ORIGINAL\n'+(saved/'src/animation/AnimBlendHierarchy.h').read_text()+'\n#else\n'+(tree/'src/animation/AnimBlendHierarchy.h').read_text()+'\n#endif\n',newline='\n')
for name in ['AnimBlendHierarchy.c','AnimBlendHierarchy_backend.h']:(out/name).write_text((tree/'src/animation'/name).read_text(),newline='\n')
(out/'templates.h').write_text('#pragma once\n#include "LinkList.h"\n',newline='\n');(out/'config.h').write_text('#pragma once\n#ifndef CLASSIC\n#define FIX_BUGS\n#endif\n',newline='\n')
owners='''#pragma once
#include "config.h"
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <new>
typedef int16_t int16;
#define nil NULL
#define VALIDATE_SIZE(a,b)
#define Max(a,b) ((a)>(b)?(a):(b))
#include "AnimBlendSequence.h"
#include "AnimBlendHierarchy.h"
#include "AnimBlendSequence_backend.h"
extern unsigned allocations,frees,eventCount,events[8192],moveMode;
extern CAnimBlendHierarchy *observedHierarchy;
void recordEvent(unsigned operation,void *memory);
void *RwMalloc(size_t bytes);
void RwFree(void *memory);
enum {MEMID_ANIMATION=3};
struct TestHeap {
    void PushMemId(int id) {
        // Preserve the sequence's real native memory category boundary
        assert(id==3);recordEvent(3,NULL);
    }
    void PopMemId() {
        // Observe the native memory category restoration
        recordEvent(4,NULL);
    }
    void RegisterMemPointer(void *memory) {
        // Verify registration uses one of the hierarchy's actual live sequence pointer fields
        bool found=false;for(int i=0;i<observedHierarchy->numSequences;++i)if(memory==&observedHierarchy->sequences[i].keyFrames||memory==&observedHierarchy->sequences[i].keyFramesCompressed)found=true;assert(found);recordEvent(5,*(void**)memory);
    }
    void *MoveMemory(void *memory);
};
extern TestHeap gMainHeap;
#ifdef USE_CUSTOM_ALLOCATOR
#define PUSH_MEMID(x) gMainHeap.PushMemId(x)
#define POP_MEMID() gMainHeap.PopMemId()
#define REGISTER_MEMPTR(x) gMainHeap.RegisterMemPointer(x)
#else
#define PUSH_MEMID(x)
#define POP_MEMID()
#define REGISTER_MEMPTR(x)
#endif
class CAnimManager {public:static CLinkList ms_animCache;static void RemoveFromUncompressedCache(CAnimBlendHierarchy*);};
extern "C" void re3_assert(const char*,const char*,unsigned int,const char*);
'''
(out/'owners.h').write_text(owners,newline='\n')
entry='''#pragma once
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif
void Run_Hierarchy(unsigned operation,void *self,char *name,bool onlyone);
bool Run_IsCompressed(void *self);
void Before_Hierarchy(unsigned,void*,char*,bool);
bool Before_IsCompressed(void*);
#ifdef __cplusplus
}
#endif
'''
caller='''#include "entry.h"
#ifndef ORIGINAL
#include "AnimBlendHierarchy.h"
#endif
void Run_Hierarchy(unsigned operation,void *self,char *name,bool onlyone)
{
    // Exercise every complete hierarchy operation through an independent C11 caller
#ifdef ORIGINAL
    Before_Hierarchy(operation,self,name,onlyone);
#else
    CAnimBlendHierarchy *h=(CAnimBlendHierarchy*)self;switch(operation){case 0:AnimHierarchy_Init(h);break;case 1:AnimHierarchy_Shutdown(h);break;case 2:AnimHierarchy_SetName(h,name);break;case 3:AnimHierarchy_CalcTotalTime(h);break;case 4:AnimHierarchy_CalcTotalTimeCompressed(h);break;case 5:AnimHierarchy_RemoveQuaternionFlips(h);break;case 6:AnimHierarchy_RemoveAnimSequences(h);break;case 7:AnimHierarchy_Uncompress(h);break;case 8:AnimHierarchy_RemoveUncompressedData(h);break;
#ifdef USE_CUSTOM_ALLOCATOR
    case 9:AnimHierarchy_MoveMemory(h,onlyone);break;
#endif
    }
#endif
}
bool Run_IsCompressed(void *self)
{
    // Query only the live compressed flag without requiring a sequence array
#ifdef ORIGINAL
    return Before_IsCompressed(self);
#else
    return AnimHierarchy_IsCompressed((CAnimBlendHierarchy*)self);
#endif
}
'''
original='''#include "owners.h"
#include "entry.h"
#undef assert
#ifndef MASTER
#define assert(expression) ((void)((!!(expression)) || (re3_assert(#expression,__FILE__,__LINE__,__FUNCTION__),0)))
#else
#define assert(expression) (expression)
#endif
'''+re.sub(r'^#include[^\n]*\n','',(saved/'src/animation/AnimBlendHierarchy.cpp').read_text(),flags=re.M)+'''
extern "C" void Before_Hierarchy(unsigned operation,void *self,char *name,bool onlyone)
{
    // Invoke all complete original methods without replacing their algorithms with a test model
    CAnimBlendHierarchy *h=(CAnimBlendHierarchy*)self;switch(operation){case 0:new(h) CAnimBlendHierarchy;break;case 1:h->Shutdown();break;case 2:h->SetName(name);break;case 3:h->CalcTotalTime();break;case 4:h->CalcTotalTimeCompressed();break;case 5:h->RemoveQuaternionFlips();break;case 6:h->RemoveAnimSequences();break;case 7:h->Uncompress();break;case 8:h->RemoveUncompressedData();break;
#ifdef USE_CUSTOM_ALLOCATOR
    case 9:h->MoveMemory(onlyone);break;
#endif
    }
}
extern "C" bool Before_IsCompressed(void *self)
{
    // Invoke the actual original flag query
    return ((CAnimBlendHierarchy*)self)->IsCompressed();
}
'''
math='#include "owners.h"\n#include "AnimBlendHierarchy_backend.h"\n'+(root/'utils/tests/anim_hierarchy_memory.cpp.in').read_text()+'\n'
cache=method((saved/'src/animation/AnimManager.cpp').read_text(),'CAnimManager::RemoveFromUncompressedCache');opening=cache.index('{')+1;cache=cache[:opening]+'\n    // Observe the original cache boundary before any owned sequence storage is freed\n    recordEvent(7,NULL);'+cache[opening:]
math+=cache+'\n'+m.backend()+'\n'
rp=(saved/'src/animation/RpAnimBlend.cpp').read_text()
for name in ['Allocate','Free','PushAnimation','PopMemory','Register']:math+=method(rp,'AnimSequence_Backend'+name)+'\n'
math+='#ifdef USE_CUSTOM_ALLOCATOR\n'+method(rp,'AnimSequence_BackendMoveMemory')+'\n#endif\n'
for name,value in [('entry.h',entry),('caller.c',caller),('original.cpp',original),('math.cpp',math),('fixture.cpp',(root/'utils/tests/anim_hierarchy.cpp.in').read_text())]:(out/name).write_text(value,newline='\n')
print('Prepared all eleven original/C hierarchy operations, complete C sequence ownership and actual manager cache removal')
