"""Apply all C motion-blur policy and live SDK clients after complete staged verification."""
from pathlib import Path
import hashlib,json,subprocess,sys
import mblur_migration as m

root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/mblur-c-tests'
subprocess.run([sys.executable,str(root/'utils/tests/mblur_stage_audit.py'),'--integration'],check=True)
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
for name in ['src/core/Camera_effects.h','src/renderer/MBlur_backend.h','src/renderer/MBlur_native.h','src/renderer/MBlur_native.cpp']:
    # Add only the verified C constants and typed SDK primitive boundary
    target=root/name;assert not target.exists();target.write_text((out/'overlay'/name).read_text(),newline='\n')

def update(name,transform):
    # Preserve each preceding verifier before extending its exact complete-source successor proof
    path=root/'utils/tests'/name;backup=out/'before/utils/tests'/name;assert not backup.exists();backup.parent.mkdir(parents=True,exist_ok=True);backup.write_bytes(path.read_bytes());source=path.read_text();value=transform(source);assert value!=source,name;path.write_text(value,newline='\n')

def endpoint(source):
    # Extend the prior endpoint without changing archived texture-pool fixtures
    assert 'def texture_pools_calls(' not in source
    source=source.replace('def calls(source):','def texture_pools_calls(source):').replace('def project(source):','def texture_pools_project(source):')
    return source+'''

def calls(source):
    # Retain complete C texture/palette ownership before applying the complete motion-blur successor
    import mblur_migration as blur
    return blur.calls(texture_pools_calls(source))

def project(source):
    # Preserve preceding C metadata before selecting C blur policy and its typed SDK primitive unit
    import mblur_migration as blur
    return blur.project(texture_pools_project(source))
'''

def successor(source):
    # Record only the explicit motion-blur producer rename and exact complete source projections
    assert 'MBLUR_RENAMES' not in source
    source=source.replace('        from texture_pools_migration import RENAMES as TEXTURE_POOLS_RENAMES','        from texture_pools_migration import RENAMES as TEXTURE_POOLS_RENAMES\n        from mblur_migration import RENAMES as MBLUR_RENAMES').replace('**TEXTURE_POOLS_RENAMES}','**TEXTURE_POOLS_RENAMES,**MBLUR_RENAMES}')
    return source.replace("'lights','texture-pools']:","'lights','texture-pools','mblur']:")

def pool_audit(source):
    # Preserve the archived pool project while requiring complete later projections for all current source bytes
    source=source.replace('assert m.project(project.read_text())==project.read_text()',"assert getattr(m,'texture_pools_project',m.project)(project.read_text())==project.read_text()")
    return source.replace("if path.is_file():assert path.read_bytes()==(root/'src'/path.relative_to(out/'overlay/src')).read_bytes(),path","if path.is_file():verify(root,'src/'+path.relative_to(out/'overlay/src').as_posix(),hashlib.sha256(path.read_bytes()).hexdigest())")

def light_audit(source):
    # Require the entire lighting native source after the complete blur successor without changing original SDK fixtures
    assert 'native==m.native()' in source
    return source.replace('native==m.native()','native==m.calls(m.native())')

update('texture_pools_migration.py',endpoint)
update('stats_successor.py',successor)
update('texture_pools_stage_audit.py',pool_audit)
update('lights_stage_audit.py',light_audit)
print('Applied the complete C motion-blur owner, shared camera constants and all live SDK clients')
