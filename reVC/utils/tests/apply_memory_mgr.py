"""Apply complete C memory owner ownership after full runtime, ABI, history and build validation."""
from pathlib import Path
import hashlib,json,subprocess,sys
import memory_mgr_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/memory-mgr-c-tests'
subprocess.run([sys.executable,str(root/'utils/tests/memory_mgr_stage_audit.py'),'--integration'],check=True)
changes=json.loads((out/'changes.json').read_text())
for name in changes:
    # Reject source edits made since the coherent immutable snapshot
    assert (root/name).read_bytes()==(out/'before'/name).read_bytes(),name
for name,digest in json.loads((out/'untouched.json').read_text()).items():
    # Protect every existing source outside the complete memory owner
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
for name in ['MemoryMgr_backend.h','MemoryMgr_native.cpp','MemoryMgr_native.h']:
    # Add only the tested SDK primitive boundary and its C declarations
    path=root/'src/rw'/name;assert not path.exists();path.write_text((out/'overlay/src/rw'/name).read_text(),newline='\n')
endpoint=root/'utils/tests/col_model_migration.py';source=endpoint.read_text();assert 'def col_model_calls(' not in source
backup=out/'before/utils/tests/col_model_migration.py';backup.parent.mkdir(parents=True,exist_ok=True);backup.write_bytes(endpoint.read_bytes())
source=source.replace('def calls(source):','def col_model_calls(source):').replace('def project(source):','def col_model_project(source):')
source+="\n\n\ndef calls(source):\n    # Preserve complete C memory ownership before moving complete memory allocation policy into C\n    import memory_mgr_migration as memory\n    return memory.calls(col_model_calls(source))\n\n\ndef project(source):\n    # Retain C collision-model metadata before selecting the C memory owner and native SDK table\n    import memory_mgr_migration as memory\n    return memory.project(col_model_project(source))\n"
endpoint.write_text(source,newline='\n')
successor=root/'utils/tests/stats_successor.py';source=successor.read_text();assert 'MEMORY_MGR_RENAMES' not in source
(out/'before/utils/tests/stats_successor.py').write_bytes(successor.read_bytes())
source=source.replace('        from col_model_migration import RENAMES as COL_MODEL_RENAMES','        from col_model_migration import RENAMES as COL_MODEL_RENAMES\n        from memory_mgr_migration import RENAMES as MEMORY_MGR_RENAMES')
source=source.replace('**COL_MODEL_RENAMES}', '**COL_MODEL_RENAMES,**MEMORY_MGR_RENAMES}')
source=source.replace("'node-name','col-model']:","'node-name','col-model','memory-mgr']:")
successor.write_text(source,newline='\n')
print('Applied all seven complete C memory operations and the separate native SDK primitive unit')
