"""Prove complete manager and consumer projections from every immutable prior conversion snapshot."""
from pathlib import Path
import importlib,json,sys
import anim_manager_migration as m
from stats_successor import current_path,verify
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/anim-manager-c-tests';target=root if '--production' in sys.argv else out/'overlay';changes=json.loads((out/'changes.json').read_text());rows=[]
stages=['events','wanted','darkel','stats','roadblocks','debug-display','autopilot','profile-power','lists','streaming-info','input-state','transmission','doors','damage','ped-routes','stored-car','rect','compressed-vector','col-triangle','col-line','col-box','col-sphere','col-point','stored-poly','quaternion','keyframes','anim-node','anim-clump','anim-frame','anim-sequence','anim-hierarchy','anim-association','anim-group']
for stage in stages:
    # Require exact complete successor transformations rather than accepting isolated matching snippets
    folder=root/f'.diagnostics/{stage}-c-tests';untouched=json.loads((folder/'untouched.json').read_text());migration=importlib.import_module(stage.replace('-','_')+'_migration')
    for name in changes:
        # Compare each real changed consumer from the older stage against its actual final target source
        if name in untouched:
            # Prove the complete historical digest chain before projecting the verified current source
            verify(root,name,untouched[name]);expected=m.calls(current_path(root,name).read_text())
        else:
            # Earlier partial snapshots retain changed files rather than whole source trees
            original=folder/'before'/name;assert original.exists(),(stage,name);expected=m.calls(migration.calls(original.read_text()))
        path=current_path(target,name) if target==root else target/m.RENAMES.get(name,name)
        assert expected==path.read_text(),(stage,name)
        rows.append({'stage':stage,'source':name,'exit':0})
    print(stage,len(changes),'complete manager/consumer projections passed',flush=True)
assert len(rows)==len(changes)*len(stages)
(out/('production-history.json' if target==root else 'historical-source-projections.json')).write_text(json.dumps(rows,indent=2),newline='\n')
