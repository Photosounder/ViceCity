"""Audit exact earlier projections including the frame interfaces introduced by the clump stage."""
from pathlib import Path
import importlib,json
import anim_frame_migration as m
import anim_clump_migration as clumps
from stats_successor import current_path,verify
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/anim-frame-c-tests';changes=json.loads((out/'changes.json').read_text());results=[]
stages=['events','wanted','darkel','stats','roadblocks','debug-display','autopilot','profile-power','lists','streaming-info','input-state','transmission','doors','damage','ped-routes','stored-car','rect','compressed-vector','col-triangle','col-line','col-box','col-sphere','col-point','stored-poly','quaternion','keyframes','anim-node','anim-clump']
introduced=['src/animation/AnimBlendClumpData_backend.h','src/animation/AnimBlendFrameData_native.h']
for stage in stages:
    folder=root/f'.diagnostics/{stage}-c-tests';untouched=json.loads((folder/'untouched.json').read_text());migration=importlib.import_module(stage.replace('-','_')+'_migration')
    for name in changes:
        original=folder/'before'/name;old=name
        if not original.exists() and name not in untouched and name=='src/animation/AnimBlendClumpData.c':old='src/animation/AnimBlendClumpData.cpp';original=folder/'before'/old
        if old in untouched:verify(root,old,untouched[old])
        elif original.exists():assert migration.calls(original.read_text())==current_path(root,name).read_text(),(stage,name)
        else:assert name in introduced,(stage,name);continue
        results.append({'stage':stage,'source':name,'exit':0})
    print(stage,'existing historical frame projections passed',flush=True)
for name,value in [(introduced[0],clumps.backend_header()),(introduced[1],clumps.frame_header((root/'.diagnostics/anim-clump-c-tests/before/src/animation/AnimBlendClumpData.h').read_text()))]:
    assert (root/name).read_text()==m.calls(value),name;results.append({'stage':'anim-clump-introduction','source':name,'exit':0})
assert len(results)==254,len(results)
(out/'historical-source-projections.json').write_text(json.dumps(results,indent=2),newline='\n')
