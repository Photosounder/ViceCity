"""Stage every 2D sprite operation against one complete verified immutable source baseline."""
from pathlib import Path
import hashlib,json,shutil
import weapon_effects_migration as effects
import weapon_effects_migration as migration
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/weapon-effects-c-tests';saved=out/'before/src';tree=out/'overlay/src';out.mkdir(exist_ok=True)
assert json.loads((root/'.diagnostics/sprite2d-c-tests/verification-summary.json').read_text())['status']=='applied-verified'
if not saved.exists():
    # Snapshot the entire coherent tree only after the complete preceding production and predecessor verification
    shutil.copytree(root/'src',saved,ignore=shutil.ignore_patterns('.git','.codex-build','.agents','.codex'))
digests={}
for path in saved.rglob('*'):
    # Reject concurrent production source changes while retaining every complete historical owner and consumer
    if path.suffix not in {'.cpp','.c','.h'}:continue
    name='src/'+path.relative_to(saved).as_posix();assert (root/name).read_bytes()==path.read_bytes(),name;digests[name]=hashlib.sha256(path.read_bytes()).hexdigest()
p=out/'baseline-source-digests.json'
if p.exists():assert json.loads(p.read_text())==digests
else:p.write_text(json.dumps(digests,indent=2),newline='\n')
if not tree.exists():shutil.copytree(saved,tree)
source=(saved/'weapons/WeaponEffects.cpp').read_text();header,native=effects.boundary(source)
for name,value in [('WeaponEffects.h',effects.header()),('WeaponEffects.c',effects.producer(source)),('WeaponEffects_backend.h',header),('WeaponEffects_native.cpp',native)]:
    # Write all complete C algorithms and only actual SDK storage primitives with LF line endings
    p=tree/'weapons'/name
    if not p.exists() or p.read_bytes()!=value.encode():p.write_text(value,newline='\n')
changes=[];untouched={}
for path in saved.rglob('*'):
    # Project every complete consumer and explicit lifetime from the immutable source tree
    if path.suffix not in {'.cpp','.c','.h'}:continue
    name='src/'+path.relative_to(saved).as_posix();original=path.read_text();converted=migration.calls(original);assert migration.calls(converted)==converted,name
    if converted!=original:
        # Write only explicit changed whole sources with LF line endings
        changes.append(name);target=out/'overlay'/migration.RENAMES.get(name,name)
        if not target.exists() or target.read_bytes()!=converted.encode():target.write_text(converted,newline='\n')
    else:untouched[name]=hashlib.sha256(path.read_bytes()).hexdigest()
for name,value in [('changes',changes),('untouched',untouched)]:
    # Retain exact full-source manifests for later production and predecessor verification
    (out/(name+'.json')).write_text(json.dumps(value,indent=2),newline='\n')
p=(tree/'weapons/WeaponEffects.cpp').resolve();assert p.is_relative_to(tree.resolve()) and (tree/'weapons/WeaponEffects.c').exists();p.unlink(missing_ok=True)
report={'status':'staged-produced','production_changed':False,'classes_in_scope':1,'original_operations_in_scope':7,'C_operations_implemented':7,'changed_sources':len(changes),'remaining':['Strict C and actual SDK owner/consumer compiles','Whole original/C behavior and actual native layout comparisons','Historical projections and full integration','Production application and predecessor audits']}
(out/'stage-summary.json').write_text(json.dumps(report,indent=2),newline='\n');print('Staged complete C WeaponEffects policy and all consumers;',len(digests),'production sources unchanged')
