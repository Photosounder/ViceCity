"""Apply verified C bone mappings and integrate exact historical producer renames."""
from pathlib import Path
import hashlib,json,subprocess,sys
import bones_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/bones-c-tests'
subprocess.run([sys.executable,str(root/'utils/tests/finalize_bones.py')],check=True)
changes=json.loads((out/'changes.json').read_text())
for name in changes:
    # Require unchanged original bytes before modifying any source or header
    assert (root/name).read_bytes()==(out/'before'/name).read_bytes(),name
for name,digest in json.loads((out/'untouched.json').read_text()).items():
    # Reject unrelated edits at the point where the complete conversion is applied
    assert hashlib.sha256((root/name).read_bytes()).hexdigest()==digest,name
for old,new in m.RENAMES.items():
    # Rename only the explicitly verified helper producer within the workspace
    source=(root/old).resolve();target=(root/new).resolve();assert source.is_relative_to(root.resolve()) and target.is_relative_to(root.resolve()) and not target.exists();source.rename(target)
for name in changes:
    # Apply the exact staged implementation and normalize all edited files to LF
    path=(root/m.RENAMES.get(name,name)).resolve();value=(out/'overlay'/m.RENAMES.get(name,name)).read_text();original=path.read_bytes()
    for attempt in range(3):
        # Retry transient Windows writes only after confirming the complete original bytes remain intact
        try:path.write_text(value,newline='\n');break
        except OSError as error:
            if error.errno!=22 or attempt==2:raise
            assert path.read_bytes()==original,name
path=root/'src/modelinfo/PedNode.h';assert not path.exists();path.write_text((out/'overlay/src/modelinfo/PedNode.h').read_text(),newline='\n')
endpoint=root/'utils/tests/anim_manager_migration.py';source=endpoint.read_text();assert 'def anim_manager_calls(' not in source
backup=out/'before/utils/tests/anim_manager_migration.py';backup.parent.mkdir(parents=True,exist_ok=True);backup.write_bytes(endpoint.read_bytes())
source=source.replace('def calls(source):','def anim_manager_calls(source):').replace('def project(source):','def anim_manager_project(source):')
source+='''


def calls(source):
    # Preserve complete C manager ownership before the bone helper and shared constant conversion
    import bones_migration as bones
    return bones.calls(anim_manager_calls(source))


def project(source):
    # Retain all manager integration metadata before selecting the C11 bone mapping producer
    import bones_migration as bones
    return bones.project(anim_manager_project(source))
'''
endpoint.write_text(source,newline='\n')
successor=root/'utils/tests/stats_successor.py';source=successor.read_text();assert 'BONES_RENAMES' not in source
(out/'before/utils/tests/stats_successor.py').write_bytes(successor.read_bytes())
source=source.replace('        from anim_manager_migration import RENAMES as ANIM_MANAGER_RENAMES','        from anim_manager_migration import RENAMES as ANIM_MANAGER_RENAMES\n        from bones_migration import RENAMES as BONES_RENAMES')
source=source.replace('**ANIM_MANAGER_RENAMES}', '**ANIM_MANAGER_RENAMES,**BONES_RENAMES}')
source=source.replace("'anim-group','anim-manager']:","'anim-group','anim-manager','bones']:")
successor.write_text(source,newline='\n')
print('Converted complete bone mappings and shared node constants to C without native algorithm bridges')
