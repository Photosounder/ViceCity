"""Apply all complete C frame callbacks only after runtime, ABI, history and full-link proof."""
from pathlib import Path
import hashlib,json,subprocess,sys
import frame_update_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/frame-update-c-tests'
subprocess.run([sys.executable,str(root/'utils/tests/frame_update_stage_audit.py'),'--integration'],check=True)
changes=json.loads((out/'changes.json').read_text())
for name in changes:
    # Reject any production edits since the coherent original snapshot was captured
    assert (root/name).read_bytes()==(out/'before'/name).read_bytes(),name
for name,digest in json.loads((out/'untouched.json').read_text()).items():
    # Keep every source outside the callback owner and public dispatch header unchanged
    assert hashlib.sha256((root/name).read_bytes()).hexdigest()==digest,name
for old,new in m.RENAMES.items():
    # Replace only the explicitly verified producer within the workspace boundary
    source=(root/old).resolve();target=(root/new).resolve();assert source.is_relative_to(root.resolve()) and target.is_relative_to(root.resolve()) and not target.exists();source.rename(target)
for name in changes:
    # Write exactly the compiled candidate and normalize every edited source file to LF
    path=(root/m.RENAMES.get(name,name)).resolve();value=(out/'overlay'/m.RENAMES.get(name,name)).read_text();original=path.read_bytes()
    for attempt in range(3):
        # Retry Windows transient writes only after proving the original bytes are still complete
        try:path.write_text(value,newline='\n');break
        except OSError as error:
            if error.errno!=22 or attempt==2:raise
            assert path.read_bytes()==original,name
for name in ['AnimBlendFrameUpdate.h','FrameUpdate_backend.h','FrameUpdate_native.cpp']:
    # Introduce the complete C dispatch record and the separately compiled native primitives
    path=root/'src/animation'/name;assert not path.exists();path.write_text((out/'overlay/src/animation'/name).read_text(),newline='\n')
endpoint=root/'utils/tests/bones_migration.py';source=endpoint.read_text();assert 'def bones_calls(' not in source
backup=out/'before/utils/tests/bones_migration.py';backup.parent.mkdir(parents=True,exist_ok=True);backup.write_bytes(endpoint.read_bytes())
source=source.replace('def calls(source):','def bones_calls(source):').replace('def project(source):','def bones_project(source):')
source+='''


def calls(source):
    # Preserve complete C bone mappings before the full frame accumulation owner and dispatch data conversion
    import frame_update_migration as frames
    return frames.calls(bones_calls(source))


def project(source):
    # Retain bone compilation metadata before selecting the C callbacks and native SDK primitive producer
    import frame_update_migration as frames
    return frames.project(bones_project(source))
'''
endpoint.write_text(source,newline='\n')
successor=root/'utils/tests/stats_successor.py';source=successor.read_text();assert 'FRAME_UPDATE_RENAMES' not in source
(out/'before/utils/tests/stats_successor.py').write_bytes(successor.read_bytes())
source=source.replace('        from bones_migration import RENAMES as BONES_RENAMES','        from bones_migration import RENAMES as BONES_RENAMES\n        from frame_update_migration import RENAMES as FRAME_UPDATE_RENAMES')
source=source.replace('**BONES_RENAMES}', '**BONES_RENAMES,**FRAME_UPDATE_RENAMES}')
source=source.replace("'anim-manager','bones']:","'anim-manager','bones','frame-update']:")
successor.write_text(source,newline='\n')
print('Applied all nine complete C frame callbacks, C dispatch state and native SDK field primitives')
