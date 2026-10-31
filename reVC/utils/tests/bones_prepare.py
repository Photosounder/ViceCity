"""Stage the bone helper conversion against a complete immutable source snapshot."""
from pathlib import Path
import hashlib,json,shutil
import bones_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/bones-c-tests';out.mkdir(exist_ok=True)
if not (out/'changes.json').exists():
    # Inventory exact production changes before copying the coherent native baseline
    changes=[];untouched={}
    for path in (root/'src').rglob('*'):
        if path.suffix not in {'.cpp','.c','.h'} or any(part.startswith('.') for part in path.relative_to(root/'src').parts):continue
        name=path.relative_to(root).as_posix();source=path.read_text()
        if m.calls(source)!=source:changes.append(name)
        else:untouched[name]=hashlib.sha256(path.read_bytes()).hexdigest()
    assert len(changes)==3,changes
    shutil.copytree(root/'src',out/'before/src',ignore=shutil.ignore_patterns('.git','.codex-build','.agents','.codex'))
    for name,value in [('changes',changes),('untouched',untouched)]:
        # Record the exact conversion boundary and byte digests of unrelated sources
        (out/(name+'.json')).write_text(json.dumps(value,indent=2),newline='\n')
shutil.copytree(out/'before/src',out/'overlay/src',dirs_exist_ok=True)
for name in json.loads((out/'changes.json').read_text()):
    # Project complete source files into the isolated tree with consistent LF line endings
    value=m.calls((out/'before'/name).read_text());assert m.calls(value)==value,name
    (out/'overlay'/m.RENAMES.get(name,name)).write_text(value,newline='\n')
    if name in m.RENAMES:(out/'overlay'/name).unlink(missing_ok=True)
(out/'overlay/src/modelinfo/PedNode.h').write_text(m.ped_header((out/'before/src/modelinfo/PedModelInfo.h').read_text()),newline='\n')
print('Staged complete C bone mappings and the unchanged shared ped-node enum')
