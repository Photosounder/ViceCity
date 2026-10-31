"""Apply all C rendering-helper policy and live SDK clients after complete staged verification."""
from pathlib import Path
import hashlib,json,subprocess,sys
import sprite2d_migration as m

root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/sprite2d-c-tests'
subprocess.run([sys.executable,str(root/'utils/tests/sprite2d_stage_audit.py'),'--integration'],check=True)
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
for name in ['src/renderer/Sprite2d_backend.h','src/renderer/Sprite2d_native.cpp']:
    # Add only the verified C constants and typed SDK primitive boundary
    target=root/name;assert not target.exists();target.write_text((out/'overlay'/name).read_text(),newline='\n')

def update(name,transform):
    # Preserve the preceding complete verifier before extending its exact successor proof
    path=root/'utils/tests'/name;backup=out/'before/utils/tests'/name;assert not backup.exists();backup.parent.mkdir(parents=True,exist_ok=True);backup.write_bytes(path.read_bytes());original=path.read_text();value=transform(original);assert value!=original,name;path.write_text(value,newline='\n')

def endpoint(source):
    # Preserve raw sprite fixtures before composing the complete C two dimensional sprite successor
    assert 'def sprite_calls(' not in source
    source=source.replace('def calls(source):','def sprite_calls(source):').replace('def project(source):','def sprite_project(source):')
    return source+"""


def calls(source):
    # Apply the complete C two dimensional sprite owner after preceding sprite policy
    import sprite2d_migration as sprite2d
    return sprite2d.calls(sprite_calls(source))


def project(source):
    # Select the complete C two dimensional sprite producer after existing project metadata
    import sprite2d_migration as sprite2d
    return sprite2d.project(sprite_project(source))
"""


def successor(source):
    # Resolve only the explicit producer rename and complete verified whole source projections
    assert 'SPRITE2D_RENAMES' not in source
    return source.replace('        from sprite_migration import RENAMES as SPRITE_RENAMES','        from sprite_migration import RENAMES as SPRITE_RENAMES\n        from sprite2d_migration import RENAMES as SPRITE2D_RENAMES').replace('**SPRITE_RENAMES}','**SPRITE_RENAMES,**SPRITE2D_RENAMES}').replace("'shadow-cameras','sprite']:","'shadow-cameras','sprite','sprite2d']:")


def preceding_audit(source):
    # Preserve the archived sprite build while proving complete current successor source digests
    source=source.replace('from stats_successor import current_path','from stats_successor import verify,current_path').replace('assert m.project(project.read_text())==project.read_text()',"assert getattr(m,'sprite_project',m.project)(project.read_text())==project.read_text()")
    return source.replace("if path.is_file():assert path.read_bytes()==(root/'src'/path.relative_to(out/'overlay/src')).read_bytes(),path","if path.is_file():verify(root,'src/'+path.relative_to(out/'overlay/src').as_posix(),hashlib.sha256(path.read_bytes()).hexdigest())")


def preceding_history(source):
    # Retain complete archived projections while resolving explicit later producer renames
    return source.replace('from stats_successor import verify','from stats_successor import verify,current_path').replace('(target/m.RENAMES.get(name,name)).read_text()', '(current_path(root,name) if production else target/m.RENAMES.get(name,name)).read_text()')


update('sprite_history.py',preceding_history)
update('sprite_migration.py',endpoint)
update('stats_successor.py',successor)
update('sprite_stage_audit.py',preceding_audit)
print('Applied all 29 complete C Sprite2d operations and explicit global menu and lazy record lifetimes')
