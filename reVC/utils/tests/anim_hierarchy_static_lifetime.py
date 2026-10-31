"""Inspect native object symbols to prove the manager array needs no C++ dynamic initializer."""
from pathlib import Path
import json,struct
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/anim-hierarchy-c-tests';rows=[]
for prefix in ['native','production']:
    for bits in [32,64]:
        for phase in ['before','after']:
            path=out/f'{prefix}{bits}-compression{bits}-{phase}-AnimManager.obj';data=path.read_bytes();symbols,count=struct.unpack_from('<II',data,8);strings=symbols+count*18;names=[];i=0
            while i<count:
                entry=symbols+i*18;raw=data[entry:entry+8];name=data[strings+struct.unpack_from('<I',raw,4)[0]:].split(b'\0',1)[0] if raw[:4]==b'\0'*4 else raw.rstrip(b'\0');value=name.decode(errors='replace')
                if 'ms_aAnimations' in value or '??0CAnimBlendHierarchy' in value:names.append(value)
                i+=1+data[entry+17]
            if phase=='before':assert any(name.startswith('??0CAnimBlendHierarchy') for name in names) and any(name.startswith('??__E?ms_aAnimations') for name in names),path
            else:assert len(names)==1 and names[0].startswith('?ms_aAnimations@'),(path,names)
            rows.append({'prefix':prefix,'bits':bits,'phase':phase,'symbols':names,'exit':0})
(out/'native-static-lifetime-symbols.json').write_text(json.dumps(rows,indent=2),newline='\n')
print('Verified the hierarchy manager array retains native static storage with no C++ constructor reference or dynamic initializer on x86 and x64')
