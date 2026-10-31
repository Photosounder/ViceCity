"""Stage the complete streaming-info conversion against immutable native sources."""
from pathlib import Path
import json,hashlib,shutil
import streaming_info_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/streaming-info-c-tests';out.mkdir(exist_ok=True)
if not (out/'changes.json').exists():
    changes=[];untouched={}
    for path in (root/'src').rglob('*'):
        if path.suffix not in ['.cpp','.c','.h']:continue
        name=path.relative_to(root).as_posix();source=path.read_text()
        if m.calls(source)!=source:changes.append(name)
        else:untouched[name]=hashlib.sha256(path.read_bytes()).hexdigest()
    shutil.copytree(root/'src',out/'before/src')
    for name,value in [('changes',changes),('untouched',untouched)]:(out/(name+'.json')).write_text(json.dumps(value,indent=2),newline='\n')
shutil.copytree(out/'before/src',out/'overlay/src',dirs_exist_ok=True)
for name in json.loads((out/'changes.json').read_text()):
    original=(out/'before'/name).read_text();value=m.calls(original);assert m.calls(value)==value,name
    (out/'overlay'/name).write_text(value,newline='\n')
for name,value in [('StreamingInfo.h',m.header((out/'before/src/core/Streaming.h').read_text())),('StreamingInfo.c',m.kernel((out/'before/src/core/Streaming.cpp').read_text()))]:(out/'overlay/src/core'/name).write_text(value,newline='\n')
print('Staged the complete C streaming-info record and six original operations')
