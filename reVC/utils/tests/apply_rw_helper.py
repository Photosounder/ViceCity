"""Apply all C rendering-helper policy and live SDK clients after complete staged verification."""
from pathlib import Path
import hashlib,json,subprocess,sys
import rw_helper_migration as m

root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/rw-helper-c-tests'
subprocess.run([sys.executable,str(root/'utils/tests/rw_helper_stage_audit.py'),'--integration'],check=True)
changes=json.loads((out/'changes.json').read_text())
for name in changes:
    # Reject concurrent production edits before applying the exact compiled owner and client projection
    assert (root/name).read_bytes()==(out/'before'/name).read_bytes(),name
for name,digest in json.loads((out/'untouched.json').read_text()).items():
    # Preserve every complete project source outside the explicit blur owner and client boundary
    assert hashlib.sha256((root/name).read_bytes()).hexdigest()==digest,name
for old,new in m.RENAMES.items():
    # Rename only the verified producer inside the intended workspace
    source=(root/old).resolve();target=(root/new).resolve();assert source.is_relative_to(root.resolve()) and target.is_relative_to(root.resolve()) and not target.exists();source.rename(target)
for name in changes:
    # Apply complete verified source files with LF line endings
    target=root/m.RENAMES.get(name,name);target.write_text((out/'overlay'/m.RENAMES.get(name,name)).read_text(),newline='\n')
for name in ['src/rw/RwHelper_backend.h','src/rw/RwHelper_native.cpp']:
    # Add only the verified C constants and typed SDK primitive boundary
    target=root/name;assert not target.exists();target.write_text((out/'overlay'/name).read_text(),newline='\n')

def update(name,transform):
    # Preserve the preceding complete verifier before extending its exact successor proof
    path=root/'utils/tests'/name;backup=out/'before/utils/tests'/name;assert not backup.exists();backup.parent.mkdir(parents=True,exist_ok=True);backup.write_bytes(path.read_bytes());original=path.read_text();value=transform(original);assert value!=original,name;path.write_text(value,newline='\n')

def endpoint(source):
    # Preserve original PostFX fixture producers while composing the complete helper conversion after them
    assert 'def postfx_calls(' not in source
    source=source.replace('def calls(source):','def postfx_calls(source):').replace('def project(source):','def postfx_project(source):')
    return source+'''

def calls(source):
    # Retain every preceding C owner before applying the complete rendering-helper successor
    import rw_helper_migration as helper
    return helper.calls(postfx_calls(source))

def project(source):
    # Select the complete C helper and its individual SDK primitives after preceding C metadata
    import rw_helper_migration as helper
    return helper.project(postfx_project(source))
'''

def successor(source):
    # Resolve only the explicit helper producer rename and exact full-source successor projections
    assert 'RW_HELPER_RENAMES' not in source
    return source.replace('        from postfx_migration import RENAMES as POSTFX_RENAMES','        from postfx_migration import RENAMES as POSTFX_RENAMES\n        from rw_helper_migration import RENAMES as RW_HELPER_RENAMES').replace('**POSTFX_RENAMES}','**POSTFX_RENAMES,**RW_HELPER_RENAMES}').replace("'mblur','postfx']:","'mblur','postfx','rw-helper']:")

def preceding_audit(source):
    # Preserve the archived PostFX project while requiring full later projections for each current source byte
    source=source.replace('assert m.project(project.read_text())==project.read_text()',"assert getattr(m,'postfx_project',m.project)(project.read_text())==project.read_text()")
    return source.replace("if path.is_file():assert path.read_bytes()==(root/'src'/path.relative_to(out/'overlay/src')).read_bytes(),path","if path.is_file():verify(root,'src/'+path.relative_to(out/'overlay/src').as_posix(),hashlib.sha256(path.read_bytes()).hexdigest())")

update('postfx_migration.py',endpoint)
update('stats_successor.py',successor)
update('postfx_stage_audit.py',preceding_audit)
print('Applied all 38 complete C rendering-helper operations and all native SDK consumers')
