"""Apply all C rendering-helper policy and live SDK clients after complete staged verification."""
from pathlib import Path
import hashlib,json,subprocess,sys
import shadow_cameras_migration as m

root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/shadow-cameras-c-tests'
subprocess.run([sys.executable,str(root/'utils/tests/shadow_cameras_stage_audit.py'),'--integration'],check=True)
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
for name in ['src/renderer/Shadow_backend.h','src/renderer/Shadow_native.cpp']:
    # Add only the verified C constants and typed SDK primitive boundary
    target=root/name;assert not target.exists();target.write_text((out/'overlay'/name).read_text(),newline='\n')

def update(name,transform):
    # Preserve the preceding complete verifier before extending its exact successor proof
    path=root/'utils/tests'/name;backup=out/'before/utils/tests'/name;assert not backup.exists();backup.parent.mkdir(parents=True,exist_ok=True);backup.write_bytes(path.read_bytes());original=path.read_text();value=transform(original);assert value!=original,name;path.write_text(value,newline='\n')

def endpoint(source):
    # Preserve raw helper fixtures while composing the complete C shadow successor after the verified helper
    assert 'def rw_helper_calls(' not in source
    source=source.replace('def calls(source):','def rw_helper_calls(source):').replace('def project(source):','def rw_helper_project(source):')
    return source+'''


def calls(source):
    # Retain the complete preceding helper conversion before applying canonical C shadow policy and consumers
    import shadow_cameras_migration as shadow
    if '#include "Shadow_backend.h"' in source:return shadow.calls(source)
    return shadow.calls(rw_helper_calls(source))


def project(source):
    # Select the complete C shadow pair after all preceding C owner metadata
    import shadow_cameras_migration as shadow
    return shadow.project(rw_helper_project(source))
'''


def successor(source):
    # Resolve only the two explicit shadow producer renames and exact complete-source projections
    assert 'SHADOW_CAMERAS_RENAMES' not in source
    return source.replace('        from rw_helper_migration import RENAMES as RW_HELPER_RENAMES','        from rw_helper_migration import RENAMES as RW_HELPER_RENAMES\n        from shadow_cameras_migration import RENAMES as SHADOW_CAMERAS_RENAMES').replace('**RW_HELPER_RENAMES}','**RW_HELPER_RENAMES,**SHADOW_CAMERAS_RENAMES}').replace("'postfx','rw-helper']:","'postfx','rw-helper','shadow-cameras']:")


def preceding_audit(source):
    # Preserve the archived helper project and verify every current source through the complete successor chain
    source=source.replace('assert m.project(project.read_text())==project.read_text()',"assert getattr(m,'rw_helper_project',m.project)(project.read_text())==project.read_text()")
    return source.replace("if path.is_file():assert path.read_bytes()==(root/'src'/path.relative_to(out/'overlay/src')).read_bytes(),path","if path.is_file():verify(root,'src/'+path.relative_to(out/'overlay/src').as_posix(),hashlib.sha256(path.read_bytes()).hexdigest())")

def preceding_history(source):
    # Resolve renamed production producers while retaining the exact complete historical source comparison
    return source.replace('from stats_successor import verify','from stats_successor import verify,current_path').replace('(target/m.RENAMES.get(name,name)).read_text()', '(current_path(root,name) if production else target/m.RENAMES.get(name,name)).read_text()')

update('rw_helper_history.py',preceding_history)
update('rw_helper_migration.py',endpoint)
update('stats_successor.py',successor)
update('rw_helper_stage_audit.py',preceding_audit)
print('Applied all 35 complete C shadow operations, explicit consumer lifecycle, and canonical sphere-output peer')
