"""Apply all C rendering-helper policy and live SDK clients after complete staged verification."""
from pathlib import Path
import hashlib,json,subprocess,sys
import shot_info_migration as m

root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/shot-info-c-tests'
subprocess.run([sys.executable,str(root/'utils/tests/shot_info_stage_audit.py'),'--integration'],check=True)
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
for name in ['src/weapons/ShotInfo_backend.h','src/weapons/ShotInfo_native.cpp']:
    # Add only the verified C constants and typed SDK primitive boundary
    target=root/name;assert not target.exists();target.write_text((out/'overlay'/name).read_text(),newline='\n')

def update(name,transform):
    # Preserve the preceding complete verifier before extending its exact successor proof
    path=root/'utils/tests'/name;backup=out/'before/utils/tests'/name;assert not backup.exists();backup.parent.mkdir(parents=True,exist_ok=True);backup.write_bytes(path.read_bytes());original=path.read_text();value=transform(original);assert value!=original,name;path.write_text(value,newline='\n')

def endpoint(source):
    # Preserve raw weapon effect fixtures before composing the complete C shot owner
    assert 'def weapon_effects_calls(' not in source
    source=source.replace('def calls(source):','def weapon_effects_calls(source):').replace('def project(source):','def weapon_effects_project(source):')
    return source+"""


def calls(source):
    # Preserve all preceding C weapon effects before applying complete C shot policy
    import shot_info_migration as shots
    return shots.calls(weapon_effects_calls(source))


def project(source):
    # Select the complete C shot producer after existing native project metadata
    import shot_info_migration as shots
    return shots.project(weapon_effects_project(source))
"""


def successor(source):
    # Resolve only explicit producer renames and exact complete historical source projections
    assert 'SHOT_INFO_RENAMES' not in source
    return source.replace('        from weapon_effects_migration import RENAMES as WEAPON_EFFECTS_RENAMES','        from weapon_effects_migration import RENAMES as WEAPON_EFFECTS_RENAMES\n        from shot_info_migration import RENAMES as SHOT_INFO_RENAMES').replace('**WEAPON_EFFECTS_RENAMES}','**WEAPON_EFFECTS_RENAMES,**SHOT_INFO_RENAMES}').replace("'sprite2d','weapon-effects']:","'sprite2d','weapon-effects','shot-info']:")


def preceding_audit(source):
    # Preserve the archived weapon build while verifying entire current successor files
    source=source.replace('from rect_migration import mask','from rect_migration import mask\nfrom stats_successor import verify,current_path').replace('target=tree/m.RENAMES.get(name,name);expected=', 'target=current_path(root,name) if production else tree/m.RENAMES.get(name,name);expected=').replace('assert m.project(project.read_text())==project.read_text()',"assert getattr(m,'weapon_effects_project',m.project)(project.read_text())==project.read_text()")
    return source.replace("if path.is_file():assert path.read_bytes()==(root/'src'/path.relative_to(out/'overlay/src')).read_bytes(),path","if path.is_file():verify(root,'src/'+path.relative_to(out/'overlay/src').as_posix(),hashlib.sha256(path.read_bytes()).hexdigest())")


def preceding_history(source):
    # Resolve later explicit producer renames while retaining exact archived complete source comparisons
    return source.replace('from stats_successor import verify','from stats_successor import verify,current_path').replace('(target/m.RENAMES.get(name,name)).read_text()', '(current_path(root,name) if production else target/m.RENAMES.get(name,name)).read_text()')


update('weapon_effects_history.py',preceding_history)
update('weapon_effects_migration.py',endpoint)
update('stats_successor.py',successor)
update('weapon_effects_stage_audit.py',preceding_audit)
print('Applied all four complete C shot operations and the native weapon consumer')
