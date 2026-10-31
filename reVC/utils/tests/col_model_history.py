"""Prove complete collision-model owner and consumer projections through every previous C conversion."""
from pathlib import Path
import importlib,json,sys
import col_model_migration as m
from stats_successor import current_path,verify
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/col-model-c-tests';target=root if '--production' in sys.argv else out/'overlay';changes=json.loads((out/'changes.json').read_text());rows=[]
stages=['events','wanted','darkel','stats','roadblocks','debug-display','autopilot','profile-power','lists','streaming-info','input-state','transmission','doors','damage','ped-routes','stored-car','rect','compressed-vector','col-triangle','col-line','col-box','col-sphere','col-point','stored-poly','quaternion','keyframes','anim-node','anim-clump','anim-frame','anim-sequence','anim-hierarchy','anim-association','anim-group','anim-manager','bones','frame-update','anim-plugin','node-name']
for stage in stages:
    # Require exact complete historical source transformations rather than selected function snippets
    folder=root/f'.diagnostics/{stage}-c-tests';untouched=json.loads((folder/'untouched.json').read_text());migration=importlib.import_module(stage.replace('-','_')+'_migration')
    for name in changes:
        # Verify each historical byte digest before adding the complete plugin projection
        if name in untouched:verify(root,name,untouched[name]);expected=m.calls(current_path(root,name).read_text())
        elif name=='src/control/RoadBlocks_backend.h' and stages.index(stage)<=stages.index('roadblocks'):
            # Prove the exact generated owner contract when the older snapshot predates that file's creation
            import roadblocks_migration as roadblocks
            created=roadblocks.calls(roadblocks.BACKEND);assert current_path(root,name).read_text()==m.calls(created) if target==root else current_path(root,name).read_text()==created
            expected=m.calls(created)
        else:
            original=folder/'before'/name;assert original.exists(),(stage,name);expected=m.calls(migration.calls(original.read_text()))
        path=current_path(root,name) if target==root else target/m.RENAMES.get(name,name);assert expected==path.read_text(),(stage,name)
        rows.append({'stage':stage,'source':name,'exit':0})
assert len(rows)==608
(out/('production-history.json' if target==root else 'historical-source-projections.json')).write_text(json.dumps(rows,indent=2),newline='\n')
print('Verified',len(rows),'complete historical collision-model owner and consumer projections')
