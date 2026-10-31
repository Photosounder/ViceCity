"""Prove complete staged rendering-helper projections through every preceding owner migration."""
from pathlib import Path
import importlib,json,sys
import shadow_cameras_migration as m
from stats_successor import verify,current_path

root=Path(__file__).resolve().parents[2]
out=root/'.diagnostics/shadow-cameras-c-tests'
changes=json.loads((out/'changes.json').read_text())
production='--production' in sys.argv
stages=['events','wanted','darkel','stats','roadblocks','debug-display','autopilot','profile-power','lists','streaming-info','input-state','transmission','doors','damage','ped-routes','stored-car','rect','compressed-vector','col-triangle','col-line','col-box','col-sphere','col-point','stored-poly','quaternion','keyframes','anim-node','anim-clump','anim-frame','anim-sequence','anim-hierarchy','anim-association','anim-group','anim-manager','bones','frame-update','anim-plugin','node-name','col-model','memory-mgr','memory-heap','txd-store','tex-read','clump-read','lights','texture-pools','mblur','postfx','rw-helper']
rows=[]
for stage in stages:
    # Compare each entire historical owner and consumer projection against the authoritative current baseline
    folder=root/f'.diagnostics/{stage}-c-tests'
    untouched=json.loads((folder/'untouched.json').read_text())
    migration=importlib.import_module(stage.replace('-','_')+'_migration')
    for name in changes:
        # Validate original digests or the exact creator when the historical tree predates a native unit
        current=(out/'before'/name).read_text()
        original=folder/'before'/name
        if name in untouched:
            verify(root,name,untouched[name])
            expected=current
        elif original.exists():
            expected=migration.calls(original.read_text())
        elif name=='src/rw/RwHelper.c':
            # Reconstruct the complete preceding C owner when older trees predate its producer rename
            import rw_helper_migration as helper
            expected=helper.producer((root/'.diagnostics/rw-helper-c-tests/before/src/rw/RwHelper.cpp').read_text())
        elif name=='src/renderer/MBlur_native.cpp':
            import mblur_migration as blur
            expected=blur.native()
        elif name=='src/rw/Lights_native.cpp':
            import lights_migration as lights
            expected=lights.calls(lights.native())
        elif name=='src/rw/TxdStore_native.cpp':
            import txd_store_migration as txd
            expected=txd.native()
        elif name=='src/rw/MemoryMgr_native.cpp':
            import memory_mgr_migration as memory
            expected=memory.native((root/'.diagnostics/memory-mgr-c-tests/before/src/rw/MemoryMgr.cpp').read_text())
        elif name=='src/collision/ColModel_native.cpp':
            import col_model_migration as model
            expected=model.native()
        elif name=='src/animation/RpAnimBlend_native.cpp':
            import anim_plugin_migration as plugin
            expected=plugin.calls(plugin.native((root/'.diagnostics/anim-plugin-c-tests/before/src/animation/RpAnimBlend.cpp').read_text()))
        else:
            raise AssertionError((stage,name,'Missing complete historical source'))
        if not production:assert expected==current,(stage,name,'Historical projection differs from baseline')
        assert m.calls(expected)==m.calls(current),(stage,name,'Complete successor differs')
        target=root if production else out/'overlay'
        assert m.calls(expected)==(current_path(root,name) if production else target/m.RENAMES.get(name,name)).read_text(),(stage,name)
        rows.append({'stage':stage,'source':name,'exit':0})
assert len(rows)==len(stages)*len(changes)==49*len(changes)
(out/('production-history.json' if production else 'historical-source-projections.json')).write_text(json.dumps(rows,indent=2),newline='\n')
print('Verified',len(rows),'complete historical rendering-helper owner and consumer projections')
