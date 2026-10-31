"""Capture the next complete owner baseline and diagnose its original policy before conversion."""
from pathlib import Path
import hashlib,json,re,shutil
from rect_migration import mask
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/shot-info-c-tests';saved=out/'before/src'
assert json.loads((root/'.diagnostics/weapon-effects-c-tests/verification-summary.json').read_text())['status']=='applied-verified'
out.mkdir(exist_ok=True)
if not saved.exists():
    # Capture all authoritative production sources only after the preceding complete verification
    shutil.copytree(root/'src',saved,ignore=shutil.ignore_patterns('.git','.codex-build','.agents','.codex'))
digests={};uses=[]
for path in saved.rglob('*'):
    # Discover every actual owner consumer while retaining exact immutable complete source bytes
    if path.suffix not in {'.cpp','.c','.h'}:continue
    name='src/'+path.relative_to(saved).as_posix();assert (root/name).read_bytes()==path.read_bytes(),name;digests[name]=hashlib.sha256(path.read_bytes()).hexdigest()
    if re.search(r'\bCShotInfo\b',mask(path.read_text())):uses.append(name)
source=(saved/'weapons/ShotInfo.cpp').read_text();operations=re.findall(r'\bCShotInfo::(Initialise|AddShot|Shutdown|Update)\s*\(',mask(source));assert len(operations)==len(set(operations))==4 and len(uses)==3
report={'status':'diagnosed','production_changed':False,'source_count':len(digests),'original_operations':operations,'complete_source_scope':uses,'record_arrays':['gaShotInfo[NUMSHOTINFOS]'],'static_arrays':['ms_afRandTable[20]'],'conditional_state':['SQUEEZE_PERFORMANCE: shotInfoInUse'],'preserve':['Validate and possibly clear source pointers even before skipping inactive slots','An expired shot still completes the current update iteration','Use the original strict expiration comparison and optional active counter','Select the first free slot after querying weapon information even when the array is full','Retain every random-number query and the original unscaled z spread','Retain vector normalization zero behavior and float conversions before scaling','Compare squared distance against Max(1.0f,radius) without squaring that threshold','Retain late live reads of ped lists positions flags and source pointers across callbacks','Retain original time-step and frame-counter query ordering','Set nearby vehicles on fire only at the original staggered frame boundary']}
for name,value in [('baseline-source-digests',digests),('preflight',report)]:
    # Retain exact complete source proof and the concrete behavior requirements for staging
    path=out/(name+'.json')
    if path.exists():assert json.loads(path.read_text())==value,name
    else:path.write_text(json.dumps(value,indent=2),newline='\n')
print(json.dumps(report,indent=2))
