"""Prepare complete original/C TXD owners using the actual C pool and General dependencies."""
from pathlib import Path
import re
from rect_migration import mask
import txd_store_migration as m

root=Path(__file__).resolve().parents[2]
out=root/'.diagnostics/txd-store-c-tests'
probe=out/'runtime';probe.mkdir(exist_ok=True)
owners='''#pragma once
#include "config.h"
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <malloc.h>
#define nil NULL
#ifdef _MSC_VER
#define CITA_TLS __declspec(thread)
#else
#define CITA_TLS __thread
#endif
extern "C" {
#ifndef __FILE_NAME__
#define __FILE_NAME__ __FILE__
#endif
#include <cita_windows.h>
void re3_assert(const char*,const char*,unsigned int,const char*);
}
#undef assert
#ifndef MASTER
#define assert(expression) (void)((!!(expression)) || (re3_assert(#expression,__FILE__,__LINE__,__func__),0))
#else
#define assert(expression) (expression)
#endif
#include "Pool.h"
#include "General.h"
struct RwTexDictionary {unsigned id,destroys;};
struct RwStream {unsigned id;bool chunk;};
#ifdef ORIGINAL
#include "original-TxdStore.h"
#else
#include "TxdStore.h"
#endif
enum {rwID_TEXDICTIONARY=22,rwSTREAMFILENAME=1,rwSTREAMREAD=2};
class CStreaming {public:static void RemoveTxd(int slot);};
void RwTexDictionaryDestroy(RwTexDictionary*);
RwTexDictionary *RwTexDictionaryGetCurrent(void);
void RwTexDictionarySetCurrent(RwTexDictionary*);
RwTexDictionary *RwTexDictionaryCreate(void);
RwStream *RwStreamFindChunk(RwStream*,int,void*,void*);
RwTexDictionary *RwTexDictionaryGtaStreamRead(RwStream*);
RwTexDictionary *RwTexDictionaryGtaStreamRead1(RwStream*);
RwTexDictionary *RwTexDictionaryGtaStreamRead2(RwStream*,RwTexDictionary*);
void _rwD3D8TexDictionaryEnableRasterFormatConversion(bool);
RwStream *RwStreamOpen(int,int,const char*);
void RwStreamClose(RwStream*,void*);
'''
entry='''#pragma once
#include <stdint.h>
typedef struct TxdResult {void *pointer;int number;} TxdResult;
#ifdef __cplusplus
extern "C" {
#endif
TxdResult TestTxd(unsigned operation,int slot,void *argument);
#ifdef __cplusplus
}
#endif
'''
before=[];after=[];enums=[]
operations=m.OPERATIONS+[('TxdDef *','GetSlot','int slot','GetSlot(int slot)')]
for index,(result,name,args,signature) in enumerate(operations):
    # Dispatch whole original methods and public C functions through independently compiled callers
    enums.append('OP_'+name+'='+str(index))
    values=[]
    if args!='void':
        for argument in args.split(','):
            if argument=='int slot':values.append('slot')
            elif 'char *' in argument:values.append('(const char*)argument')
            else:values.append('(RwStream*)argument' if name in ['LoadStream','StartLoadTxd','FinishLoadTxd'] else 'argument')
    original='CTxdStore::'+('LoadTxd' if name in ['LoadFile','LoadStream'] else name)+'('+','.join(values)+')'
    modern='TxdStore_'+name+'('+','.join(value.replace('(RwStream*)','') for value in values)+')'
    prefix='' if result=='void' else 'result.pointer=' if '*' in result else 'result.number='
    before.append('case OP_'+name+':'+prefix+original+';break;')
    after.append('case OP_'+name+':'+prefix+modern+';break;')
entry+='enum {\n    '+',\n    '.join(enums)+'\n};\n'
entry=entry.replace('TxdResult TestTxd(unsigned operation,int slot,void *argument);','TxdResult TestTxd(unsigned operation,int slot,void *argument);\nTxdResult TestTxdBinding(unsigned operation,int slot,const char *name);')
originals=[(out/'before'/name).read_text() for name in m.BINDING_PATHS]
definitions=[(definition,symbol) for original in originals for _,_,definition,symbol in m.forwarder_definitions(original)]
actions={'PushCurrentTxd':1,'PopCurrentTxd':2,'FindTxdSlot':3,'SetCurrentTxd':4,'AddTxdSlot':5,'Create':6,'AddRef':7,'RemoveTxdSlot':8}
binding_cases=[];binding_actions=[]
for index,(definition,symbol) in enumerate(definitions):
    # Exercise each exact retained public ABI from both independent callers
    target=re.search(r'CTxdStore::(\w+)\(',mask(definition))[1]
    arguments=re.search(r'\(([^\n]*)\)\n\{',definition)[1]
    args='' if arguments=='void' else 'name' if 'const char *' in arguments else 'slot'
    assert arguments=='void' or 'const char *' in arguments or 'int32_t ' in arguments or 'int ' in arguments,arguments
    result='' if definition.startswith('void ') else 'result.number='
    binding_cases.append('case '+str(index)+':'+result+symbol+'('+args+');break;')
    binding_actions.append(actions[target])
entry+='static const unsigned TestBindingActions[]={'+','.join(str(action) for action in binding_actions)+'};\n'
dispatch='TxdResult TestTxdBinding(unsigned operation,int slot,const char *name)\n{\n    // Invoke every complete retained forwarding ABI through an independent caller\n    TxdResult result={0};switch(operation){\n    '+'\n    '.join(binding_cases)+'\n    }return result;\n}\n'
(probe/'original-bindings.h').write_text(m.bindings_header(originals),newline='\n')
(probe/'before-bindings.cpp').write_text('#include "owners.h"\n#include "entry.h"\n#include "original-bindings.h"\n'+'\n'.join(definition for definition,_ in definitions)+'\n'+dispatch,newline='\n')
(probe/'after-bindings-caller.c').write_text('#include "entry.h"\n#include "TxdStore_bindings.h"\n'+dispatch,newline='\n')
for name,cases,prefix in [('before-caller.cpp',before,'#include "owners.h"\nextern "C" '),('after-caller.c',after,'#include "TxdStore.h"\n')]:
    # Keep C linkage on the original caller and strict C compilation on the candidate caller
    value='#include "entry.h"\n'+prefix+'TxdResult TestTxd(unsigned operation,int slot,void *argument)\n{\n    // Invoke complete original or C dictionary ownership through its public API\n    TxdResult result={0};switch(operation){\n    '+'\n    '.join(cases)+'\n    }return result;\n}\n'
    (probe/name).write_text(value,newline='\n')
original=re.sub(r'^#include[^\n]*\n','',(out/'before/src/rw/TxdStore.cpp').read_text(),flags=re.M)
original_header=re.sub(r'^#include[^\n]*\n','',(out/'before/src/rw/TxdStore.h').read_text(),flags=re.M)
native=re.sub(r'^#include[^\n]*\n','',m.native(),flags=re.M)
for name,value in [('owners.h',owners),('entry.h',entry),('original-TxdStore.h',original_header),('original.cpp','#include "owners.h"\n'+original),('native.cpp','#include "owners.h"\n#include "TxdStore_backend.h"\n'+native),('fixture.cpp',(root/'utils/tests/txd_store_fixture.cpp.in').read_text())]:
    # Materialize entire owners while external SDK operations remain controlled fixture boundaries
    (probe/name).write_text(value,newline='\n')
print('Prepared all 21 original/C TXD operations with actual Pool and General code')
