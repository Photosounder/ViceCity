"""Capture the next complete owner baseline and diagnose its original policy before conversion."""
from pathlib import Path
import hashlib,json,re,shutil
from rect_migration import mask
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/bullet-info-c-tests';saved=out/'before/src'
assert json.loads((root/'.diagnostics/shot-info-c-tests/verification-summary.json').read_text())['status']=='applied-verified'
out.mkdir(exist_ok=True)
if not saved.exists():
    # Capture all authoritative production sources only after the preceding complete verification
    shutil.copytree(root/'src',saved,ignore=shutil.ignore_patterns('.git','.codex-build','.agents','.codex'))
digests={};uses=[]
for path in saved.rglob('*'):
    # Discover every actual owner consumer while retaining exact immutable complete source bytes
    if path.suffix not in {'.cpp','.c','.h'}:continue
    name='src/'+path.relative_to(saved).as_posix();assert (root/name).read_bytes()==path.read_bytes(),name;digests[name]=hashlib.sha256(path.read_bytes()).hexdigest()
    if re.search(r'\bCBulletInfo\b',mask(path.read_text())):uses.append(name)
source=(saved/'weapons/BulletInfo.cpp').read_text();operations=re.findall(r'\bCBulletInfo::(Initialise|AddBullet|Shutdown|Update|TestForSniperBullet)\s*\(',mask(source));assert len(operations)==len(set(operations))==5 and len(uses)==5
report={'status':'diagnosed','production_changed':False,'source_count':len(digests),'original_operations':operations,'complete_source_scope':uses,'record_arrays':['gaBulletInfo[100]'],'vector_globals':['PlayerSniperBulletStart','PlayerSniperBulletEnd'],'conditional_state':['SQUEEZE_PERFORMANCE: bulletInfoInUse','FIX_BUGS: collision termination and sniper box test'],'preserve':['Initialise leaves position speed and damage fields unchanged','The optional zero-counter return precedes clearing the sniper-bullet flag','Invalid ped sources are validated before skipping inactive slots','Expired bullets still complete the current update iteration','Boundary rejection includes equality and retains the original counter behavior','Keep original world query flags and ignore-entity writes and reset order','Occupant collision can replace the hit entity and collision point','Retain all live damage animation event particle fire glass and explosion call ordering','Retain existing conditional and repeated active-counter decrements','Retain duplicate pooled audio record resets and existing allocation failure behavior','Keep the original FIX_BUGS and legacy sniper-box algorithms','Keep the unconditional heli collision query and original sniper endpoint updates']}

for name,value in [('baseline-source-digests',digests),('preflight',report)]:
    # Retain exact complete source proof and the concrete behavior requirements for staging
    path=out/(name+'.json')
    if path.exists():assert json.loads(path.read_text())==value,name
    else:path.write_text(json.dumps(value,indent=2),newline='\n')
print(json.dumps(report,indent=2))
