"""Capture complete production sources and diagnose the next C owner before editing it."""
from pathlib import Path
import hashlib,json,re,shutil
from memory_heap_migration import body
from rect_migration import mask

root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/weapon-effects-c-tests';saved=out/'before/src'
assert json.loads((root/'.diagnostics/sprite2d-c-tests/verification-summary.json').read_text())['status']=='applied-verified'
out.mkdir(exist_ok=True)
if not saved.exists():
    # Capture the complete verified source tree before changing any owner or consumer
    shutil.copytree(root/'src',saved,ignore=shutil.ignore_patterns('.git','.codex-build','.agents','.codex'))
digests={};uses=[]
for path in saved.rglob('*'):
    # Verify the immutable baseline and discover every complete owner reference or consumer
    if path.suffix not in {'.cpp','.c','.h'}:continue
    name='src/'+path.relative_to(saved).as_posix();assert (root/name).read_bytes()==path.read_bytes(),name;digests[name]=hashlib.sha256(path.read_bytes()).hexdigest()
    if re.search(r'\bCWeaponEffects\b',mask(path.read_text())):uses.append(name)
source=(saved/'weapons/WeaponEffects.cpp').read_text();header=(saved/'weapons/WeaponEffects.h').read_text();vector=(saved/'math/Vector.h').read_text()
assert not mask(body(source,'CWeaponEffects::CWeaponEffects()')).strip() and not mask(body(source,'CWeaponEffects::~CWeaponEffects()')).strip()
assert not mask(body(vector,'CVector(void)')).strip()
operations=re.findall(r'\bCWeaponEffects::(Init|Shutdown|MarkTarget|ClearCrossHair|Render)\s*\(',mask(source));assert len(operations)==len(set(operations))==5
assert len(uses)==6,uses
report={'status':'diagnosed','production_changed':False,'source_count':len(digests),'original_operations':operations,'original_lifetime_operations':2,'owner_record_size':28,'global_records':['gCrossHair'],'global_SDK_resources':['gpCrossHairTex'],'complete_source_scope':uses,'preserve':['Empty constructor and destructor including the empty vector default constructor','MarkTarget ignores its original colour arguments','Rendering does not read the stored size field','Weapon table indexing and unguarded texture shutdown retain their original behavior','Rotation advances only after successful projection and wraps with the original strict comparison','FIX_BUGS selects the original destination blend mode','Two complete sprite draws and original render-state ordering']}
for name,value in [('baseline-source-digests',digests),('preflight',report)]:
    # Retain exact source digests and concrete conversion requirements for the next stage
    path=out/(name+'.json')
    if path.exists():assert json.loads(path.read_text())==value,name
    else:path.write_text(json.dumps(value,indent=2),newline='\n')
print(json.dumps(report,indent=2))
