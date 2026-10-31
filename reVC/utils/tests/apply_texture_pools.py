"""Apply both C pool records and all complete operations after staged integration verification."""
from pathlib import Path
import hashlib,json,subprocess,sys
import texture_pools_migration as m

root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/texture-pools-c-tests'
subprocess.run([sys.executable,str(root/'utils/tests/texture_pools_stage_audit.py'),'--integration'],check=True)
changes=json.loads((out/'changes.json').read_text())
for name in changes:
    # Reject production edits after the immutable owner and client snapshot
    assert (root/name).read_bytes()==(out/'before'/name).read_bytes(),name
for name,digest in json.loads((out/'untouched.json').read_text()).items():
    # Protect the complete project outside the verified owner and two public clients
    assert hashlib.sha256((root/name).read_bytes()).hexdigest()==digest,name
for old,new in m.RENAMES.items():
    # Rename only the complete verified texture-pool producer inside the intended workspace
    source=(root/old).resolve();target=(root/new).resolve();assert source.is_relative_to(root.resolve()) and target.is_relative_to(root.resolve()) and not target.exists();source.rename(target)
for name in changes:
    # Apply complete verified source files with LF line endings
    target=root/m.RENAMES.get(name,name);target.write_text((out/'overlay'/m.RENAMES.get(name,name)).read_text(),newline='\n')

def update(name,transform):
    # Preserve each preceding verifier before extending its exact whole-source successor proof
    path=root/'utils/tests'/name;backup=out/'before/utils/tests'/name;assert not backup.exists();backup.parent.mkdir(parents=True,exist_ok=True);backup.write_bytes(path.read_bytes());source=path.read_text();value=transform(source);assert value!=source,name;path.write_text(value,newline='\n')

def endpoint(source):
    # Extend the preceding endpoint without changing archived lighting fixtures
    assert 'def lights_calls(' not in source
    source=source.replace('def calls(source):','def lights_calls(source):').replace('def project(source):','def lights_project(source):')
    return source+'''

def calls(source):
    # Retain complete lighting ownership before applying both complete C pool records and operations
    import texture_pools_migration as pools
    return pools.calls(lights_calls(source))

def project(source):
    # Preserve preceding C metadata before selecting the complete C texture-pool owner
    import texture_pools_migration as pools
    return pools.project(lights_project(source))
'''

def successor(source):
    # Record only the explicit producer rename and exact complete source projections
    assert 'TEXTURE_POOLS_RENAMES' not in source
    source=source.replace('        from lights_migration import RENAMES as LIGHTS_RENAMES','        from lights_migration import RENAMES as LIGHTS_RENAMES\n        from texture_pools_migration import RENAMES as TEXTURE_POOLS_RENAMES').replace('**LIGHTS_RENAMES}','**LIGHTS_RENAMES,**TEXTURE_POOLS_RENAMES}')
    return source.replace("'clump-read','lights']:","'clump-read','lights','texture-pools']:")

def lighting_audit(source):
    # Preserve the archived lighting project while proving all applied bytes through complete later projections
    source=source.replace('assert m.project(project.read_text())==project.read_text()',"assert getattr(m,'lights_project',m.project)(project.read_text())==project.read_text()")
    return source.replace("if path.is_file():assert path.read_bytes()==(root/'src'/path.relative_to(out/'overlay/src')).read_bytes(),path","if path.is_file():verify(root,'src/'+path.relative_to(out/'overlay/src').as_posix(),hashlib.sha256(path.read_bytes()).hexdigest())")

update('lights_migration.py',endpoint)
update('stats_successor.py',successor)
update('lights_stage_audit.py',lighting_audit)
print('Applied both complete C texture/palette records and all 17 operations with direct COM C calls')
