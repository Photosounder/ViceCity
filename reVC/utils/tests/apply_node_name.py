"""Apply complete C frame-name plugin ownership after full runtime, ABI, history and build validation."""
from pathlib import Path
import hashlib,json,subprocess,sys
import node_name_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/node-name-c-tests'
subprocess.run([sys.executable,str(root/'utils/tests/node_name_stage_audit.py'),'--integration'],check=True)
changes=json.loads((out/'changes.json').read_text())
for name in changes:
    # Reject source edits made since the coherent immutable snapshot
    assert (root/name).read_bytes()==(out/'before'/name).read_bytes(),name
for name,digest in json.loads((out/'untouched.json').read_text()).items():
    # Protect every existing source outside the complete frame-name plugin
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
for name in ['NodeName_backend.h','NodeName_native.cpp']:
    # Add only the tested SDK primitive boundary and its C declarations
    path=root/'src/rw'/name;assert not path.exists();path.write_text((out/'overlay/src/rw'/name).read_text(),newline='\n')
endpoint=root/'utils/tests/anim_plugin_migration.py';source=endpoint.read_text();assert 'def anim_plugin_calls(' not in source
backup=out/'before/utils/tests/anim_plugin_migration.py';backup.parent.mkdir(parents=True,exist_ok=True);backup.write_bytes(endpoint.read_bytes())
source=source.replace('def calls(source):','def anim_plugin_calls(source):').replace('def project(source):','def anim_plugin_project(source):')
source+="\n\n\ndef calls(source):\n    # Preserve complete C animation plugin ownership before the frame-name plugin conversion\n    import node_name_migration as names\n    return names.calls(anim_plugin_calls(source))\n\n\ndef project(source):\n    # Retain C animation plugin metadata before adding the complete C name owner and SDK primitives\n    import node_name_migration as names\n    return names.project(anim_plugin_project(source))\n"
endpoint.write_text(source,newline='\n')
successor=root/'utils/tests/stats_successor.py';source=successor.read_text();assert 'NODE_NAME_RENAMES' not in source
(out/'before/utils/tests/stats_successor.py').write_bytes(successor.read_bytes())
source=source.replace('        from anim_plugin_migration import RENAMES as ANIM_PLUGIN_RENAMES','        from anim_plugin_migration import RENAMES as ANIM_PLUGIN_RENAMES\n        from node_name_migration import RENAMES as NODE_NAME_RENAMES')
source=source.replace('**ANIM_PLUGIN_RENAMES}', '**ANIM_PLUGIN_RENAMES,**NODE_NAME_RENAMES}')
source=source.replace("'frame-update','anim-plugin']:","'frame-update','anim-plugin','node-name']:")
successor.write_text(source,newline='\n')
print('Applied all eight complete C frame-name operations and the separate native SDK primitive unit')
