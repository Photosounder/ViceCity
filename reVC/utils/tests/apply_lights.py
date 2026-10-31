"""Apply complete C lighting only after all staged runtime, historical and integration checks."""
from pathlib import Path
import hashlib,json,subprocess,sys
import lights_migration as m

root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/lights-c-tests'
subprocess.run([sys.executable,str(root/'utils/tests/lights_stage_audit.py'),'--integration'],check=True)
changes=json.loads((out/'changes.json').read_text())
for name in changes:
    # Reject production edits since the immutable staging snapshot
    assert (root/name).read_bytes()==(out/'before'/name).read_bytes(),name
for name,digest in json.loads((out/'untouched.json').read_text()).items():
    # Protect the complete project outside the verified owner and client boundary
    assert hashlib.sha256((root/name).read_bytes()).hexdigest()==digest,name
for old,new in m.RENAMES.items():
    # Rename only the verified lighting producer inside the intended workspace
    source=(root/old).resolve();target=(root/new).resolve();assert source.is_relative_to(root.resolve()) and target.is_relative_to(root.resolve()) and not target.exists();source.rename(target)
for name in changes:
    # Apply complete verified source files with LF line endings
    target=root/m.RENAMES.get(name,name);target.write_text((out/'overlay'/m.RENAMES.get(name,name)).read_text(),newline='\n')
for name in ['Lights_backend.h','Lights_native.cpp']:
    # Add only the compiled native primitive boundary and self-contained C records
    target=root/'src/rw'/name;assert not target.exists();target.write_text((out/'overlay/src/rw'/name).read_text(),newline='\n')

def update(name,transform):
    # Preserve each preceding verifier before extending its exact successor proof
    path=root/'utils/tests'/name;backup=out/'before/utils/tests'/name;assert not backup.exists();backup.parent.mkdir(parents=True,exist_ok=True);backup.write_bytes(path.read_bytes());source=path.read_text();value=transform(source);assert value!=source,name;path.write_text(value,newline='\n')

def endpoint(source):
    # Extend the preceding owner endpoint without changing archived clump fixtures
    assert 'def clump_read_calls(' not in source
    source=source.replace('def calls(source):','def clump_read_calls(source):').replace('def project(source):','def clump_read_project(source):')
    return source+'''

def calls(source):
    # Retain complete clump ownership before applying the complete C lighting successor
    import lights_migration as lights
    return lights.calls(clump_read_calls(source))

def project(source):
    # Retain preceding C metadata before selecting C lighting and its native SDK unit
    import lights_migration as lights
    return lights.project(clump_read_project(source))
'''

def successor(source):
    # Record only the explicit lighting producer rename and exact whole-source projections
    assert 'LIGHTS_RENAMES' not in source
    source=source.replace('        from clump_read_migration import RENAMES as CLUMP_READ_RENAMES','        from clump_read_migration import RENAMES as CLUMP_READ_RENAMES\n        from lights_migration import RENAMES as LIGHTS_RENAMES')
    source=source.replace('**CLUMP_READ_RENAMES}','**CLUMP_READ_RENAMES,**LIGHTS_RENAMES}')
    return source.replace("'tex-read','clump-read']:","'tex-read','clump-read','lights']:")

def clump_audit(source):
    # Preserve the archived clump build while proving all applied bytes through complete later projections
    source=source.replace('assert m.project(project.read_text())==project.read_text()',"assert getattr(m,'clump_read_project',m.project)(project.read_text())==project.read_text()")
    return source.replace("if path.is_file():assert path.read_bytes()==(root/'src'/path.relative_to(out/'overlay/src')).read_bytes(),path","if path.is_file():verify(root,'src/'+path.relative_to(out/'overlay/src').as_posix(),hashlib.sha256(path.read_bytes()).hexdigest())")

update('clump_read_migration.py',endpoint)
update('stats_successor.py',successor)
update('clump_read_stage_audit.py',clump_audit)
print('Applied all 19 complete C lighting operations and all native clients')
