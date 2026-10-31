"""Prove complete frame-name plugin projections through every previous C conversion."""
from pathlib import Path
import importlib,json,sys
import node_name_migration as m
from stats_successor import current_path,verify
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/node-name-c-tests';target=root if '--production' in sys.argv else out/'overlay';changes=json.loads((out/'changes.json').read_text());rows=[]
stages=['events','wanted','darkel','stats','roadblocks','debug-display','autopilot','profile-power','lists','streaming-info','input-state','transmission','doors','damage','ped-routes','stored-car','rect','compressed-vector','col-triangle','col-line','col-box','col-sphere','col-point','stored-poly','quaternion','keyframes','anim-node','anim-clump','anim-frame','anim-sequence','anim-hierarchy','anim-association','anim-group','anim-manager','bones','frame-update','anim-plugin']
for stage in stages:
    # Require exact complete historical source transformations rather than selected function snippets
    folder=root/f'.diagnostics/{stage}-c-tests';untouched=json.loads((folder/'untouched.json').read_text());migration=importlib.import_module(stage.replace('-','_')+'_migration')
    for name in changes:
        # Verify each historical byte digest before adding the complete plugin projection
        if name in untouched:verify(root,name,untouched[name]);expected=m.calls(current_path(root,name).read_text())
        else:
            original=folder/'before'/name;assert original.exists(),(stage,name);expected=m.calls(migration.calls(original.read_text()))
        path=current_path(root,name) if target==root else target/m.RENAMES.get(name,name);assert expected==path.read_text(),(stage,name)
        rows.append({'stage':stage,'source':name,'exit':0})
assert len(rows)==74
(out/('production-history.json' if target==root else 'historical-source-projections.json')).write_text(json.dumps(rows,indent=2),newline='\n')
print('Verified',len(rows),'complete historical frame-name plugin projections')
