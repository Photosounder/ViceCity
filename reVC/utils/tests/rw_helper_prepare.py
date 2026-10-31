"""Prepare the complete C rendering-helper candidate without mutating production."""
from pathlib import Path
import re,json,hashlib,subprocess,sys
import rw_helper_migration as m
from rw_helper_schema import OPAQUE
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/rw-helper-c-tests';saved=out/'before';tree=out/'overlay'
for name,digest in json.loads((out/'baseline-source-digests.json').read_text()).items():
    # Reject concurrent changes to any source in the immutable baseline
    assert hashlib.sha256((root/name).read_bytes()).hexdigest()==digest,name
subprocess.run([sys.executable,str(root/'utils/tests/rw_helper_boundary.py')],check=True)
value=m.producer((saved/'src/rw/RwHelper.cpp').read_text());header=m.header((saved/'src/rw/RwHelper.h').read_text());_,constants=m.kernel((saved/'src/rw/RwHelper.cpp').read_text())
for name,text in [('src/rw/RwHelper.c',value),('src/rw/RwHelper.h',header)]:
    # Write complete deterministic LF source projections while retaining the original owner for comparison
    target=tree/name
    if not target.exists() or target.read_bytes()!=text.encode():target.write_text(text,newline='\n')
(out/'kernel-constants.json').write_text(json.dumps(constants,indent=2),newline='\n')
assert not re.search(r'::|\b(?:class|new|delete|CVector)\b',m.mask(value))
print('Prepared the full 38-operation C producer candidate and its SDK primitives; production unchanged')

changes=[];untouched={}
for path in (saved/'src').rglob('*'):
    # Project every whole existing native consumer from the same immutable source baseline
    if path.suffix not in {'.cpp','.c','.h'}:continue
    name='src/'+path.relative_to(saved/'src').as_posix();original=path.read_text();converted=m.calls(original);assert m.calls(converted)==converted,name
    if converted!=original:
        changes.append(name);target=tree/m.RENAMES.get(name,name)
        if not target.exists() or target.read_bytes()!=converted.encode():target.write_text(converted,newline='\n')
    else:untouched[name]=hashlib.sha256(path.read_bytes()).hexdigest()
for name,value in [('changes',changes),('untouched',untouched)]:
    # Retain exact owner and client manifests for full-source and predecessor audits
    (out/(name+'.json')).write_text(json.dumps(value,indent=2),newline='\n')
print('Projected',len(changes),'complete owner and native consumer paths')
for old,new in m.RENAMES.items():
    # Keep the original comparison producer only in the immutable snapshot and select one staged C owner
    target=(tree/old).resolve();assert target.is_relative_to((tree/'src').resolve()) and (tree/new).exists();target.unlink(missing_ok=True)
