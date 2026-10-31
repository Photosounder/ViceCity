"""Apply the entire C TXD and caller migration after complete staged integration checks."""
from pathlib import Path
import hashlib,json,subprocess,sys
import txd_store_migration as m

root=Path(__file__).resolve().parents[2]
out=root/'.diagnostics/txd-store-c-tests'
subprocess.run([sys.executable,str(root/'utils/tests/txd_store_stage_audit.py'),'--integration'],check=True)
changes=json.loads((out/'changes.json').read_text())
for name in changes:
    # Reject concurrent production edits before applying the exact compiled candidate
    assert (root/name).read_bytes()==(out/'before'/name).read_bytes(),name
for name,digest in json.loads((out/'untouched.json').read_text()).items():
    # Keep every source outside the complete owner and caller boundary unchanged
    assert hashlib.sha256((root/name).read_bytes()).hexdigest()==digest,name
for old,new in m.RENAMES.items():
    # Rename only the verified TXD producer inside the intended workspace
    source=(root/old).resolve();target=(root/new).resolve()
    assert source.is_relative_to(root.resolve()) and target.is_relative_to(root.resolve()) and not target.exists()
    source.rename(target)
for name in changes:
    # Apply complete verified source files with LF line endings
    target=root/m.RENAMES.get(name,name)
    target.write_text((out/'overlay'/m.RENAMES.get(name,name)).read_text(),newline='\n')
for name in ['TxdStore_backend.h','TxdStore_native.cpp','TxdStore_bindings.c','TxdStore_bindings.h']:
    # Add only the compiled native primitive boundary and C forwarding unit
    target=root/'src/rw'/name
    assert not target.exists(),target
    target.write_text((out/'overlay/src/rw'/name).read_text(),newline='\n')

def update(name,transform):
    # Preserve each preceding verifier before extending its complete successor proof
    path=root/'utils/tests'/name
    backup=out/'before/utils/tests'/name
    assert not backup.exists(),backup
    backup.parent.mkdir(parents=True,exist_ok=True)
    backup.write_bytes(path.read_bytes())
    original=path.read_text();value=transform(original)
    assert value!=original,name
    path.write_text(value,newline='\n')

def endpoint(source):
    # Extend the prior owner endpoint with the complete C TXD owner rather than weakening historical checks
    assert 'def memory_heap_calls(' not in source
    source=source.replace('def calls(source):','def memory_heap_calls(source):').replace('def project(source):','def memory_heap_project(source):')
    return source+'''

def calls(source):
    # Retain complete heap ownership before applying the complete C TXD successor
    import txd_store_migration as txd
    return txd.calls(memory_heap_calls(source))

def project(source):
    # Preserve preceding C metadata before selecting both complete C TXD units
    import txd_store_migration as txd
    return txd.project(memory_heap_project(source))
'''

def successor(source):
    # Record the explicit TXD producer rename and require complete source projections
    assert 'TXD_STORE_RENAMES' not in source
    source=source.replace('        from memory_heap_migration import RENAMES as MEMORY_HEAP_RENAMES','        from memory_heap_migration import RENAMES as MEMORY_HEAP_RENAMES\n        from txd_store_migration import RENAMES as TXD_STORE_RENAMES')
    source=source.replace('**MEMORY_HEAP_RENAMES}', '**MEMORY_HEAP_RENAMES,**TXD_STORE_RENAMES}')
    return source.replace("'memory-mgr','memory-heap']:","'memory-mgr','memory-heap','txd-store']:")

def heap_audit(source):
    # Preserve original heap build evidence while checking exact complete current source successors
    source=source.replace('assert m.project(project.read_text())==project.read_text()',"assert getattr(m,'memory_heap_project',m.project)(project.read_text())==project.read_text()")
    return source.replace("if path.is_file():assert path.read_bytes()==(root/'src'/path.relative_to(out/'overlay/src')).read_bytes(),path","if path.is_file():verify(root,'src/'+path.relative_to(out/'overlay/src').as_posix(),hashlib.sha256(path.read_bytes()).hexdigest())")

update('memory_heap_migration.py',endpoint)
update('stats_successor.py',successor)
update('memory_heap_stage_audit.py',heap_audit)
print('Applied all 21 complete C TXD operations, 27 C forwarding ABIs and every existing caller')
