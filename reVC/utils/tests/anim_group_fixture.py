"""Compare original/C group operations with real converted association dependencies."""
from pathlib import Path
import re,sys,json
import anim_group_migration as m
from stats_successor import current_path
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/anim-group-c-tests';saved=out/'before';tree=root if '--production' in sys.argv else out/'overlay';dependency=root/'.diagnostics/anim-association-c-tests'
if '--production' in sys.argv:
    for name in json.loads((out/'changes.json').read_text()):assert current_path(root,name).read_text()==m.calls((saved/name).read_text()),name
names=['AnimBlendAssociation.h','AnimBlendAssociation.c','AnimBlendAssociation_backend.h','AnimBlendNode.h','AnimBlendNode.c','AnimBlendNode_backend.h','AnimBlendSequence.h','AnimBlendSequence.c','AnimBlendSequence_backend.h','AnimBlendHierarchy.h','AnimBlendList.h','AnimBlendList.c','AnimBlendClumpData.h','AnimBlendClumpData_backend.h','AnimBlendFrameData.h','AnimBlendFrameData_native.h','AnimBlendAssociation_flags.h','KeyFrames.h','KeyFrames.c','KeyFrames_native.h']
for name in names:(out/name).write_text((saved/'src/animation'/name).read_text(),newline='\n')
for name in ['Quaternion.h','Quaternion.c','Quaternion_native.h','Vector.h','Vector2D.h','Maths.h','LinkList.h']:(out/name).write_text((dependency/name).read_text(),newline='\n')
for name in ['AnimBlendAssocGroup.c','AnimBlendAssocGroup_backend.h','AnimBlock.h']:(out/name).write_text((tree/'src/animation'/name).read_text(),newline='\n')
(out/'AnimBlendAssocGroup.h').write_text('#pragma once\n#ifdef ORIGINAL\n'+(saved/'src/animation/AnimBlendAssocGroup.h').read_text()+'\n#else\n'+(tree/'src/animation/AnimBlendAssocGroup.h').read_text()+'\n#endif\n',newline='\n')
(out/'config.h').write_text('#pragma once\n#define MODELINFOSIZE 8\n#ifndef CLASSIC\n#define FIX_BUGS\n#endif\n',newline='\n');(out/'ModelIndices.h').write_text('#pragma once\n#define MI_PLAYER 0\n',newline='\n')
(out/'General.h').write_text('#pragma once\n#include <stdbool.h>\n#ifdef __cplusplus\nextern "C" {\n#endif\nbool General_faststricmp(const char*,const char*);\n#ifdef __cplusplus\n}\n#endif\n',newline='\n')
owners=(dependency/'owners.h').read_text().replace('static void UncompressAnimation(CAnimBlendHierarchy*);','static void UncompressAnimation(CAnimBlendHierarchy*);static CAnimBlock *GetAnimationBlock(const char*);static CAnimBlendHierarchy *GetAnimation(int32_t);static CAnimBlendHierarchy *GetAnimation(const char*,CAnimBlock*);')
owners=owners.replace('class CAnimManager {','#include "AnimBlock.h"\n#include "AnimBlendAssocGroup.h"\nclass CAnimManager {')
owners+='''
#include <ctype.h>
#include <wctype.h>
#ifdef _MSC_VER
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#else
#include <strings.h>
#endif
#include "ModelIndices.h"
#include "General.h"
#define debug(format,...) re3_debug("[DBG]: " format,__VA_ARGS__)
enum {rpCLUMP=2};
struct RwObject {unsigned type;};
struct CBaseModelInfo {char name[32];RwObject *object;RpClump *instance;char *GetModelName(){return name;}RwObject *GetRwObject(){return object;}RwObject *CreateInstance(){return (RwObject*)instance;}};
class CModelInfo {public:static CBaseModelInfo *GetModelInfo(int32_t);};
unsigned RwObjectGetType(RwObject*);
void RpAnimBlendClumpInit(RpClump*);
bool IsClumpSkinned(RpClump*);
typedef void *(*AtomicCallback)(void*,void*);
void *AtomicRemoveAnimFromSkinCB(void*,void*);
void RpClumpForAllAtomics(RpClump*,AtomicCallback,void*);
void RpClumpDestroy(RpClump*);
extern CAnimBlock block;
extern CAnimBlendHierarchy hierarchies[8];
extern CBaseModelInfo *models[8];
extern unsigned debugCalls,eventCount,events[8192];
extern bool skinned;
extern bool exitOutput;
extern "C" void re3_debug(const char*,...);
'''
(out/'owners.h').write_text(owners,newline='\n')
entry='''#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
void *Run_Group(unsigned operation,void *self,uint32_t id,const char *name);
void *Before_Group(unsigned operation,void *self,uint32_t id,const char *name);
#ifdef __cplusplus
}
#endif
'''
caller='''#include "entry.h"
#ifndef ORIGINAL
#include "AnimBlendAssocGroup.h"
#endif
void *Run_Group(unsigned operation,void *self,uint32_t id,const char *name)
{
    // Exercise complete ownership and lookup from an independent strict C11 consumer
#ifdef ORIGINAL
    return Before_Group(operation,self,id,name);
#else
    CAnimBlendAssocGroup *g=(CAnimBlendAssocGroup*)self;switch(operation){case 0:AnimGroup_Init(g);break;case 1:AnimGroup_Destroy(g);break;case 2:AnimGroup_DestroyAssociations(g);break;case 3:return AnimGroup_GetById(g,id);case 4:return AnimGroup_GetByName(g,name);case 5:return AnimGroup_CopyById(g,id);case 6:return AnimGroup_CopyByName(g,name);}return NULL;
#endif
}
'''
original='#include "owners.h"\n#include "entry.h"\n'+''.join(line+'\n' for line in (saved/'src/animation/AnimBlendAssocGroup.cpp').read_text().splitlines()[22:] if not line.startswith('#include'))+'''
extern "C" void *Before_Group(unsigned operation,void *self,uint32_t id,const char *name)
{
    // Invoke the complete original class algorithms without replacing them with a model
    CAnimBlendAssocGroup *g=(CAnimBlendAssocGroup*)self;switch(operation){case 0:new(g) CAnimBlendAssocGroup;break;case 1:g->~CAnimBlendAssocGroup();break;case 2:g->DestroyAssociations();break;case 3:return g->GetAnimation(id);case 4:return g->GetAnimation(name);case 5:return g->CopyAnimation(id);case 6:return g->CopyAnimation(name);}return NULL;
}
'''
entry=entry.replace('#include <stdint.h>','#include <stdint.h>\n#include <stdbool.h>')
entry=entry.replace('#ifdef __cplusplus\n}\n#endif','void Run_GroupCreate(unsigned,void*,void*,const char**,int32_t);\nvoid Before_GroupCreate(unsigned,void*,void*,const char**,int32_t);\nbool Run_GroupCompare(const char*,const char*);\nbool Before_GroupCompare(const char*,const char*);\nvoid *Run_GroupModel(const char*);\nvoid *Before_GroupModel(const char*);\n#ifdef __cplusplus\n}\n#endif')
caller+='''
void Run_GroupCreate(unsigned operation,void *self,void *clump,const char **names,int32_t count)
{
    // Invoke both complete creation overloads through the strict C11 boundary
#ifdef ORIGINAL
    Before_GroupCreate(operation,self,clump,names,count);
#else
    if(operation==7)AnimGroup_CreateForModels((CAnimBlendAssocGroup*)self,"block");else AnimGroup_CreateForClump((CAnimBlendAssocGroup*)self,"block",clump,names,count);
#endif
}
bool Run_GroupCompare(const char *a,const char *b)
{
    // Compare complete original character matching rather than reproducing its algorithm
#ifdef ORIGINAL
    return Before_GroupCompare(a,b);
#else
    return AnimGroup_CompareIgnoringDigits(a,b);
#endif
}
void *Run_GroupModel(const char *name)
{
    // Exercise the complete original model selection loop and player-name substitution
#ifdef ORIGINAL
    return Before_GroupModel(name);
#else
    return AnimGroup_FindModel(name);
#endif
}
'''
original+='''
extern "C" void Before_GroupCreate(unsigned operation,void *self,void *clump,const char **names,int32_t count)
{
    // Invoke both entire original creation algorithms on their real owner
    CAnimBlendAssocGroup *group=(CAnimBlendAssocGroup*)self;if(operation==7)group->CreateAssociations("block");else group->CreateAssociations("block",(RpClump*)clump,names,count);
}
extern "C" bool Before_GroupCompare(const char *a,const char *b)
{
    // Invoke the full original matching helper including its prefix behavior
    return strcmpIgnoringDigits(a,b);
}
extern "C" void *Before_GroupModel(const char *name)
{
    // Invoke the full original ordered model search
    return GetModelFromName(name);
}
'''
primitive=(dependency/'math.cpp').read_text().replace('++frees;if(observedOwner', 'if(exitOutput)puts("exit-node-release");++frees;if(observedOwner')
math=primitive+'\n#include "AnimBlendAssocGroup_backend.h"\n'+(root/'utils/tests/anim_group_native.cpp.in').read_text()+m.native_backend()
for name,value in [('entry.h',entry),('caller.c',caller),('original.cpp',original),('math.cpp',math),('fixture.cpp',(root/'utils/tests/anim_group.cpp.in').read_text())]:(out/name).write_text(value,newline='\n')
print('Prepared original/C group ownership and lookups with actual C association dependencies')
