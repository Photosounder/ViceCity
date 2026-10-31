"""Compare complete original/C lists using the real pool and global shutdown ordering."""
from pathlib import Path
import re,sys,json
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/lists-c-tests';saved=out/'before';overlay=out/'overlay'
sourceTree=root if '--production' in sys.argv else overlay
if '--production' in sys.argv:
    import lists_migration as m
    for name in json.loads((out/'changes.json').read_text()):assert (root/m.RENAMES.get(name,name)).read_text()==m.calls((saved/name).read_text()),name
    assert not (root/'src/core/Lists.cpp').exists()
for name in ['Lists.h','Lists.c']:(out/name).write_text((sourceTree/'src/core'/name).read_text(),newline='\n')
(out/'config.h').write_text('#pragma once\n',newline='\n')
(out/'Pool.h').write_text((saved/'src/core/Pool.h').read_text(),newline='\n')
(out/'original-Lists.h').write_text((saved/'src/core/Lists.h').read_text(),newline='\n')
(out/'common.h').write_text('''#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#define nil NULL
extern "C" void re3_assert(const char *,const char *,unsigned int,const char *);
#ifndef MASTER
#define assert(expression) ((void)((!!(expression)) || (re3_assert(#expression,__FILE__,__LINE__,__func__),0)))
#else
#define assert(expression) (expression)
#endif
#include "Pool.h"
#ifdef ORIGINAL
#include "original-Lists.h"
#else
#include "Lists.h"
#endif
''',newline='\n')
(out/'Pools.h').write_text('''#pragma once
#include "Pool.h"
#ifdef __cplusplus
extern "C" {
#endif
CPool *CPools_GetPtrNodePool(void);
CPool *CPools_GetEntryInfoNodePool(void);
#ifdef __cplusplus
}
#ifdef ORIGINAL
class CPools {public:
static CPool *GetPtrNodePool(){return CPools_GetPtrNodePool();}
static CPool *GetEntryInfoNodePool(){return CPools_GetEntryInfoNodePool();}
};
#endif
#endif
''',newline='\n')
original=(saved/'src/core/Lists.cpp').read_text();original=original.replace('#include "Lists.h"','')
(out/'original.cpp').write_text(original,newline='\n')
fixture=(root/'utils/tests/lists.cpp.in').read_text()
for prefix,record,node in [('PtrList','CPtrList','CPtrNode'),('EntryInfoList','CEntryInfoList','CEntryInfoNode')]:
    for operation in ['InsertNode','RemoveNode','DeleteNode','Flush','InsertItem']+(['FindItem','RemoveItem'] if prefix=='PtrList' else []):
        fixture=fixture.replace('@MACROS@','#define '+prefix+'_'+operation+'(self,...) ((self)->'+operation+'(__VA_ARGS__))\n@MACROS@',1)
    fixture=fixture.replace('@MACROS@','#define '+prefix+'_Init(self) ((self)->first=NULL)\n#define '+prefix+'_Destroy(self) ((self)->Flush())\n@MACROS@',1)
fixture=fixture.replace('@MACROS@','')
# Exercise the literal original/projected global definitions and their registration expressions
pattern=r'CPtrList CWorld::ms_bigBuildingsList\[NUM_LEVELS\];[\s\S]*?CSector CWorld::ms_aSectors\[NUMSECTORS_Y\]\[NUMSECTORS_X\];[^\n]*\n(?:int CWorld::ms_sectorListsLifetime[^\n]*\n)?'
definitions=[]
for tree in [saved,overlay]:
    match=re.search(pattern,(tree/'src/core/World.cpp').read_text());assert match
    definitions.append(match[0])
fixture=fixture.replace('@DEFINITIONS@','#ifdef ORIGINAL\n'+definitions[0]+'#else\n'+definitions[1]+'#endif\n')
(out/'fixture.cpp').write_text(fixture,newline='\n')
print('Prepared complete original/C list operations, real pool behavior and the literal world lifetime registrations')
