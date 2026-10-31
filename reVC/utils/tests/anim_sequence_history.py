"""Audit exact historical projections of the whole sequence owner and every affected consumer."""
from pathlib import Path
import importlib,json
import anim_sequence_migration as m
import anim_node_migration as nodes
from stats_successor import current_path,verify
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/anim-sequence-c-tests';changes=json.loads((out/'changes.json').read_text());results=[]
stages=['events','wanted','darkel','stats','roadblocks','debug-display','autopilot','profile-power','lists','streaming-info','input-state','transmission','doors','damage','ped-routes','stored-car','rect','compressed-vector','col-triangle','col-line','col-box','col-sphere','col-point','stored-poly','quaternion','keyframes','anim-node','anim-clump','anim-frame']
for stage in stages:
    folder=root/f'.diagnostics/{stage}-c-tests';untouched=json.loads((folder/'untouched.json').read_text());migration=importlib.import_module(stage.replace('-','_')+'_migration')
    for name in changes:
        old=name
        if name=='src/animation/AnimBlendNode.c' and name not in untouched and not (folder/'before'/name).exists():old='src/animation/AnimBlendNode.cpp'
        original=folder/'before'/old
        if old in untouched:verify(root,old,untouched[old])
        elif original.exists():assert migration.calls(original.read_text())==current_path(root,name).read_text(),(stage,name)
        else:assert name=='src/animation/AnimBlendNode_backend.h',(stage,name);continue
        results.append({'stage':stage,'source':name,'exit':0})
    print(stage,'existing historical sequence projections passed',flush=True)
assert (root/'src/animation/AnimBlendNode_backend.h').read_text()==m.calls(nodes.backend_header());results.append({'stage':'anim-node-introduction','source':'src/animation/AnimBlendNode_backend.h','exit':0})
assert len(results)==322,len(results)
(out/'historical-source-projections.json').write_text(json.dumps(results,indent=2),newline='\n')
