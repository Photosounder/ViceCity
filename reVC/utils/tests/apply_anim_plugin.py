"""Apply complete C plugin ownership after full runtime, ABI, history and build validation."""
from pathlib import Path
import hashlib,json,subprocess,sys
import anim_plugin_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/anim-plugin-c-tests'
subprocess.run([sys.executable,str(root/'utils/tests/anim_plugin_stage_audit.py'),'--integration'],check=True)
changes=json.loads((out/'changes.json').read_text())
for name in changes:
    # Reject source edits made since the coherent immutable snapshot
    assert (root/name).read_bytes()==(out/'before'/name).read_bytes(),name
for name,digest in json.loads((out/'untouched.json').read_text()).items():
    # Protect every existing source outside the complete plugin and its diagnosed consumers
    assert hashlib.sha256((root/name).read_bytes()).hexdigest()==digest,name
for old,new in m.RENAMES.items():
    # Rename only the explicitly verified owner inside the workspace
    source=(root/old).resolve();target=(root/new).resolve();assert source.is_relative_to(root.resolve()) and target.is_relative_to(root.resolve()) and not target.exists();source.rename(target)
for name in changes:
    # Write exactly the fully compiled candidate and normalize the whole edited file to LF
    path=(root/m.RENAMES.get(name,name)).resolve();value=(out/'overlay'/m.RENAMES.get(name,name)).read_text();original=path.read_bytes()
    for attempt in range(3):
        # Retry transient Windows writes only while the original bytes remain intact
        try:path.write_text(value,newline='\n');break
        except OSError as error:
            if error.errno!=22 or attempt==2:raise
            assert path.read_bytes()==original,name
for name in ['RpAnimBlend_backend.h','RpAnimBlend_native.cpp']:
    # Add only the tested SDK primitive boundary and its C declarations
    path=root/'src/animation'/name;assert not path.exists();path.write_text((out/'overlay/src/animation'/name).read_text(),newline='\n')
endpoint=root/'utils/tests/frame_update_migration.py';source=endpoint.read_text();assert 'def frame_update_calls(' not in source
backup=out/'before/utils/tests/frame_update_migration.py';backup.parent.mkdir(parents=True,exist_ok=True);backup.write_bytes(endpoint.read_bytes())
source=source.replace('def calls(source):','def frame_update_calls(source):').replace('def project(source):','def frame_update_project(source):')
source+='''


def calls(source):
    # Preserve complete C frame callbacks before converting the complete animation plugin owner
    import anim_plugin_migration as plugin
    return plugin.calls(frame_update_calls(source))


def project(source):
    # Retain C callback metadata before selecting the C plugin and native SDK primitive unit
    import anim_plugin_migration as plugin
    return plugin.project(frame_update_project(source))
'''
endpoint.write_text(source,newline='\n')
successor=root/'utils/tests/stats_successor.py';source=successor.read_text();assert 'ANIM_PLUGIN_RENAMES' not in source
(out/'before/utils/tests/stats_successor.py').write_bytes(successor.read_bytes())
source=source.replace('        from frame_update_migration import RENAMES as FRAME_UPDATE_RENAMES','        from frame_update_migration import RENAMES as FRAME_UPDATE_RENAMES\n        from anim_plugin_migration import RENAMES as ANIM_PLUGIN_RENAMES')
source=source.replace('**FRAME_UPDATE_RENAMES}', '**FRAME_UPDATE_RENAMES,**ANIM_PLUGIN_RENAMES}')
source=source.replace("'bones','frame-update']:","'bones','frame-update','anim-plugin']:")
successor.write_text(source,newline='\n')
print('Applied complete C animation plugin ownership and the separate native SDK primitive unit')
