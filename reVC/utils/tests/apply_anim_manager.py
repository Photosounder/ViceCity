"""Apply the complete manager conversion after exact staged runtime, native, history and full-link proof."""
from pathlib import Path
import hashlib,json,subprocess,sys
import anim_manager_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/anim-manager-c-tests'
subprocess.run([sys.executable,str(root/'utils/tests/anim_manager_stage_audit.py'),'--integration'],check=True)
changes=json.loads((out/'changes.json').read_text())
for name in changes:
    # Reject external source edits before any production mutation
    assert (root/name).read_bytes()==(out/'before'/name).read_bytes(),name
for name,digest in json.loads((out/'untouched.json').read_text()).items():
    # Preserve all source files outside this explicitly audited owner and its consumers
    assert hashlib.sha256((root/name).read_bytes()).hexdigest()==digest,name
for old,new in m.RENAMES.items():
    # Replace only the verified workspace producer with its C-language path
    source=(root/old).resolve();target=(root/new).resolve();assert source.is_relative_to(root.resolve()) and target.is_relative_to(root.resolve());assert not target.exists();source.rename(target)
for name in changes:
    # Apply exactly the staged implementation and convert each edited file completely to LF
    path=(root/m.RENAMES.get(name,name)).resolve();value=(out/'overlay'/m.RENAMES.get(name,name)).read_text();originalBytes=path.read_bytes()
    for attempt in range(3):
        # Retry Windows transient writes only after proving no bytes were changed or lost
        try:path.write_text(value,newline='\n');break
        except OSError as error:
            if error.errno!=22 or attempt==2:raise
            assert path.read_bytes()==originalBytes,name
for name in ['AnimManagerData.h','AnimManager_backend.h','AnimManager_native.cpp']:
    # Introduce the complete C records and the separately compiled native primitive leaves
    path=root/'src/animation'/name;assert not path.exists();path.write_text((out/'overlay/src/animation'/name).read_text(),newline='\n')
endpoint=root/'utils/tests/anim_group_migration.py';source=endpoint.read_text();assert 'def anim_group_calls(' not in source
snapshot=out/'before/utils/tests/anim_group_migration.py';snapshot.parent.mkdir(parents=True,exist_ok=True);snapshot.write_bytes(endpoint.read_bytes())
source=source.replace('def calls(source):','def anim_group_calls(source):').replace('def project(source):','def anim_group_project(source):')
source+='''


def calls(source):
    # Preserve complete group ownership before the full manager algorithms and native caller conversion
    import anim_manager_migration as manager
    return manager.calls(anim_group_calls(source))


def project(source):
    # Retain group project metadata before the C manager and separate native primitive producer
    import anim_manager_migration as manager
    return manager.project(anim_group_project(source))
'''
endpoint.write_text(source,newline='\n')
successor=root/'utils/tests/stats_successor.py';source=successor.read_text();assert 'ANIM_MANAGER_RENAMES' not in source
snapshot=out/'before/utils/tests/stats_successor.py';snapshot.write_bytes(successor.read_bytes())
source=source.replace('        from anim_group_migration import RENAMES as ANIM_GROUP_RENAMES','        from anim_group_migration import RENAMES as ANIM_GROUP_RENAMES\n        from anim_manager_migration import RENAMES as ANIM_MANAGER_RENAMES')
source=source.replace('**ANIM_GROUP_RENAMES}', '**ANIM_GROUP_RENAMES,**ANIM_MANAGER_RENAMES}')
source=source.replace("'anim-association','anim-group']:","'anim-association','anim-group','anim-manager']:")
successor.write_text(source,newline='\n')
print('Applied complete C animation manager ownership, all native consumers and explicit historical successor integration')
