"""Audit every earlier snapshot of the seven affected sources through explicit successor projections."""
from pathlib import Path
import hashlib,importlib,json
import anim_node_migration as m
from stats_successor import current_path
from stats_successor import verify
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/anim-node-c-tests';changes=json.loads((out/'changes.json').read_text());results=[]
stages=['events','wanted','darkel','stats','roadblocks','debug-display','autopilot','profile-power','lists','streaming-info','input-state','transmission','doors','damage','ped-routes','stored-car','rect','compressed-vector','col-triangle','col-line','col-box','col-sphere','col-point','stored-poly','quaternion','keyframes']
for stage in stages:
    folder=root/f'.diagnostics/{stage}-c-tests';untouched=json.loads((folder/'untouched.json').read_text());migration=importlib.import_module(stage.replace('-','_')+'_migration')
    for name in changes:
        if name in untouched:
            verify(root,name,untouched[name])
        else:
            original=folder/'before'/name
            if not original.exists() and name=='src/animation/KeyFrames.h':continue
            assert original.exists(),(stage,name)
            expected=migration.calls(original.read_text());current=current_path(root,m.RENAMES.get(name,name)).read_text();assert expected==current,(stage,name)
        results.append({'stage':stage,'source':name,'exit':0})
    print(stage,'all existing historical source projections passed',flush=True)
frames=importlib.import_module('keyframes_migration');assert (root/'src/animation/KeyFrames.h').read_text()==frames.calls(frames.header())
results.append({'stage':'keyframes-introduction','source':'src/animation/KeyFrames.h','exit':0})
assert len(results)==235
(out/'historical-source-projections.json').write_text(json.dumps(results,indent=2),newline='\n')
