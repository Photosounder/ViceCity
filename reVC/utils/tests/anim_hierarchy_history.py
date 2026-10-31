"""Audit every historical version of the hierarchy owner and all affected native callers."""
from pathlib import Path
import importlib,json
import anim_hierarchy_migration as m
from stats_successor import current_path,verify
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/anim-hierarchy-c-tests';changes=json.loads((out/'changes.json').read_text());results=[]
stages=['events','wanted','darkel','stats','roadblocks','debug-display','autopilot','profile-power','lists','streaming-info','input-state','transmission','doors','damage','ped-routes','stored-car','rect','compressed-vector','col-triangle','col-line','col-box','col-sphere','col-point','stored-poly','quaternion','keyframes','anim-node','anim-clump','anim-frame','anim-sequence']
for stage in stages:
    folder=root/f'.diagnostics/{stage}-c-tests';untouched=json.loads((folder/'untouched.json').read_text());migration=importlib.import_module(stage.replace('-','_')+'_migration')
    for name in changes:
        if name in untouched:verify(root,name,untouched[name])
        else:
            original=folder/'before'/name;assert original.exists(),(stage,name)
            assert migration.calls(original.read_text())==current_path(root,name).read_text(),(stage,name)
        results.append({'stage':stage,'source':name,'exit':0})
    print(stage,'all five historical hierarchy projections passed',flush=True)
assert len(results)==150
(out/'historical-source-projections.json').write_text(json.dumps(results,indent=2),newline='\n')
