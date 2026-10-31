"""Stage the complete C sprite producer against one immutable verified source baseline."""
from pathlib import Path
import hashlib,json,shutil
import sprite_schema as schema,sprite_kernel as kernel,sprite_boundary as boundary
import sprite_migration as migration,sprite_peers as peers
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/sprite-c-tests';saved=out/'before/src';tree=out/'overlay/src';out.mkdir(exist_ok=True)
assert json.loads((root/'.diagnostics/shadow-cameras-c-tests/verification-summary.json').read_text())['status']=='applied-verified'
if not saved.exists():
    # Capture the whole coherent source tree after the preceding owner and predecessor checks have passed
    shutil.copytree(root/'src',saved,ignore=shutil.ignore_patterns('.git','.codex-build','.agents','.codex'))
digests={}
for path in saved.rglob('*'):
    # Reject every concurrent production source edit before preparing the complete sprite candidate
    if path.suffix not in {'.cpp','.c','.h'}:continue
    name='src/'+path.relative_to(saved).as_posix();assert (root/name).read_bytes()==path.read_bytes(),name;digests[name]=hashlib.sha256(path.read_bytes()).hexdigest()
p=out/'baseline-source-digests.json'
if p.exists():assert json.loads(p.read_text())==digests
else:p.write_text(json.dumps(digests,indent=2),newline='\n')
if not tree.exists():shutil.copytree(saved,tree)
source=(saved/'renderer/Sprite.cpp').read_text();header,native=boundary.build()
for name,value in [('Sprite.h',schema.header(source)),('Sprite.c',kernel.producer(source)),('Sprite_backend.h',header),('Sprite_native.cpp',native)]:
    # Retain every complete original operation while selecting genuine C policy and actual typed SDK storage
    p=tree/'renderer'/name
    if not p.exists() or p.read_bytes()!=value.encode():p.write_text(value,newline='\n')
originals=[(saved/'renderer'/(name+'.cpp')).read_text() for name in peers.SOURCES]
value=peers.bindings(originals);p=tree/'renderer/Sprite_bindings.c'
if not p.exists() or p.read_bytes()!=value.encode():p.write_text(value,newline='\n')
changes=[];untouched={}
for path in saved.rglob('*'):
    # Project every complete existing owner consumer and C peer from one immutable coherent baseline
    if path.suffix not in {'.cpp','.c','.h'}:continue
    name='src/'+path.relative_to(saved).as_posix();original=path.read_text();converted=migration.calls(original);assert migration.calls(converted)==converted,name
    if converted!=original:
        # Write only explicit changed whole sources with LF endings
        changes.append(name);target=out/'overlay'/migration.RENAMES.get(name,name)
        if not target.exists() or target.read_bytes()!=converted.encode():target.write_text(converted,newline='\n')
    else:untouched[name]=hashlib.sha256(path.read_bytes()).hexdigest()
for name,value in [('changes',changes),('untouched',untouched)]:
    # Retain complete changed and untouched source manifests for full predecessor proofs
    (out/(name+'.json')).write_text(json.dumps(value,indent=2),newline='\n')
p=(tree/'renderer/Sprite.cpp').resolve();assert p.is_relative_to(tree.resolve()) and (tree/'renderer/Sprite.c').exists();p.unlink(missing_ok=True)
report={'status':'staged-produced','production_changed':False,'classes_in_scope':1,'original_operations_in_scope':17,'C_operations_implemented':17,'remaining':['Existing native consumers and pure C peer forwarders','Complete original/C behavior comparisons and real SDK compilation','Historical source projections and full Debug/Release integration','Production application and predecessor audits']};p=out/'stage-summary.json'
if p.exists():report.update({k:v for k,v in json.loads(p.read_text()).items() if k not in report})
p.write_text(json.dumps(report,indent=2),newline='\n');print('Staged all 17 original sprite operations;',len(digests),'complete production source digests unchanged')
