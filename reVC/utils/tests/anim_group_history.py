"""Audit original group, manager and cutscene sources through every explicit prior conversion."""
from pathlib import Path
import importlib,json
import anim_group_migration as m
from stats_successor import current_path,verify
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/anim-group-c-tests';changes=json.loads((out/'changes.json').read_text());rows=[]
stages=['events','wanted','darkel','stats','roadblocks','debug-display','autopilot','profile-power','lists','streaming-info','input-state','transmission','doors','damage','ped-routes','stored-car','rect','compressed-vector','col-triangle','col-line','col-box','col-sphere','col-point','stored-poly','quaternion','keyframes','anim-node','anim-clump','anim-frame','anim-sequence','anim-hierarchy','anim-association']
for stage in stages:
    folder=root/f'.diagnostics/{stage}-c-tests';untouched=json.loads((folder/'untouched.json').read_text());migration=importlib.import_module(stage.replace('-','_')+'_migration')
    for name in changes:
        if name in untouched:verify(root,name,untouched[name])
        else:
            original=folder/'before'/name;assert original.exists(),(stage,name);assert migration.calls(original.read_text())==current_path(root,name).read_text(),(stage,name)
        rows.append({'stage':stage,'source':name,'exit':0})
    print(stage,'six historical group projections passed',flush=True)
assert (root/'src/animation/AnimBlock.h').read_text()==m.block_header();rows.append({'stage':'anim-group-introduction','source':'src/animation/AnimBlock.h','exit':0})
assert len(rows)==193,len(rows)
(out/'historical-source-projections.json').write_text(json.dumps(rows,indent=2),newline='\n')
