"""Prepare literal original methods, strict C callers and complete streaming-info comparisons."""
from pathlib import Path
import re,json,sys
import streaming_info_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/streaming-info-c-tests';saved=out/'before';tree=root if '--production' in sys.argv else out/'overlay'
if '--production' in sys.argv:
    for name in json.loads((out/'changes.json').read_text()):assert (root/name).read_text()==m.calls((saved/name).read_text()),name
    assert (root/'src/core/StreamingInfo.h').read_text()==m.header((saved/'src/core/Streaming.h').read_text())
    assert (root/'src/core/StreamingInfo.c').read_text()==m.kernel((saved/'src/core/Streaming.cpp').read_text())
for name in ['StreamingInfo.h','StreamingInfo.c']:(out/name).write_text((tree/'src/core'/name).read_text(),newline='\n')
originalHeader=(saved/'src/core/Streaming.h').read_text();record=re.search(r'class CStreamingInfo\n\{[\s\S]*?\n\};',originalHeader)[0];enums=originalHeader[originalHeader.index('enum StreamFlags'):originalHeader.index('enum ChannelState')]
(out/'original.h').write_text('#pragma once\n#include <stdint.h>\n#include <stddef.h>\ntypedef uint8_t uint8;typedef int16_t int16;typedef uint32_t uint32;\n#define nil NULL\n'+enums+record+'\n',newline='\n')
operations=[('GetCdPosnAndSize','bool','void *self,uint32_t *posn,uint32_t *size',['self','posn','size'],['*posn','*size']),('SetCdPosnAndSize','void','void *self,uint32_t posn,uint32_t size',['self','posn','size'],['posn','size']),('AddToList','void','void *self,void *link',['self','link'],['(CStreamingInfo*)link']),('RemoveFromList','void','void *self',['self'],[]),('GetCdSize','uint32_t','void *self',['self'],[]),('IsPriority','bool','void *self',['self'],[])]
entry='#pragma once\n#include <stdint.h>\n#include <stdbool.h>\n#ifdef __cplusplus\nextern "C" {\n#endif\n'
original='#include "original.h"\n#include "entry.h"\n'+'\n\n'.join(m.definitions((saved/'src/core/Streaming.cpp').read_text()))+'\n'
caller='#include "entry.h"\n#ifndef ORIGINAL\n#include "StreamingInfo.h"\n#endif\n'
for name,result,args,values,nativeArgs in operations:
    for prefix in ['Before','Run']:entry+=result+' '+prefix+'_'+name+'('+args+');\n'
    original+='extern "C" '+result+' Before_'+name+'('+args+')\n{\n    // Exercise the complete original method through its original native record\n    '+('return ' if result!='void' else '')+'((CStreamingInfo*)self)->'+name+'('+','.join(nativeArgs)+');\n}\n'
    plain=['(CStreamingInfo*)self']+[('(CStreamingInfo*)'+v if v=='link' else v) for v in values[1:]]
    caller+=result+' Run_'+name+'('+args+')\n{\n    // Invoke the public record operation from an independent strict C11 caller\n#ifdef ORIGINAL\n    '+('return ' if result!='void' else '')+'Before_'+name+'('+','.join(values)+');\n#else\n    '+('return ' if result!='void' else '')+'StreamingInfo_'+name+'('+','.join(plain)+');\n#endif\n}\n'
entry+='#ifdef __cplusplus\n}\n#endif\n'
for name,value in [('entry.h',entry),('original.cpp',original),('caller.c',caller),('fixture.cpp',(root/'utils/tests/streaming_info.cpp.in').read_text())]:(out/name).write_text(value,newline='\n')
print('Prepared all six complete methods, independent C callers, output aliases and intrusive-list checks')
