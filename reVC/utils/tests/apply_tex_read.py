"""Apply the entire C texture reader and caller migration after complete staged integration checks."""
from pathlib import Path
import hashlib,json,subprocess,sys
import tex_read_migration as m

root=Path(__file__).resolve().parents[2]
out=root/'.diagnostics/tex-read-c-tests'
subprocess.run([sys.executable,str(root/'utils/tests/tex_read_stage_audit.py'),'--integration'],check=True)
changes=json.loads((out/'changes.json').read_text())
for name in changes:
    # Reject concurrent production edits before applying the exact compiled candidate
    assert (root/name).read_bytes()==(out/'before'/name).read_bytes(),name
for name,digest in json.loads((out/'untouched.json').read_text()).items():
    # Keep every source outside the complete owner and caller boundary unchanged
    assert hashlib.sha256((root/name).read_bytes()).hexdigest()==digest,name
for old,new in m.RENAMES.items():
    # Rename only the verified texture producer inside the intended workspace
    source=(root/old).resolve();target=(root/new).resolve()
    assert source.is_relative_to(root.resolve()) and target.is_relative_to(root.resolve()) and not target.exists()
    source.rename(target)
for name in changes:
    # Apply complete verified source files with LF line endings
    target=root/m.RENAMES.get(name,name)
    target.write_text((out/'overlay'/m.RENAMES.get(name,name)).read_text(),newline='\n')
for name in ['TexRead.h','TexRead_backend.h','TexRead_native.cpp','TexRead_bindings.c']:
    # Add only the compiled native primitive boundary and C forwarding unit
    target=root/'src/rw'/name
    assert not target.exists(),target
    target.write_text((out/'overlay/src/rw'/name).read_text(),newline='\n')

for name in ['src/core/CdStream_constants.h','src/core/Streaming_constants.h','src/skel/crossplatform_constants.h']:
    # Add only the compiled shared constant declarations at their exact workspace paths
    target=root/name;assert not target.exists();target.write_text((out/'overlay'/name).read_text(),newline='\n')

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
    # Extend the prior owner endpoint with the complete C texture owner rather than weakening historical checks
    assert 'def txd_store_calls(' not in source
    source=source.replace('def calls(source):','def txd_store_calls(source):').replace('def project(source):','def txd_store_project(source):')
    return source+'''

def calls(source):
    # Retain complete dictionary-store ownership before applying the complete C texture successor
    import tex_read_migration as texture
    return texture.calls(txd_store_calls(source))

def project(source):
    # Preserve preceding C metadata before selecting both complete C texture units
    import tex_read_migration as texture
    return texture.project(txd_store_project(source))
'''

def successor(source):
    # Record the texture producer rename and require exact whole-source texture policy projections
    assert 'TEX_READ_RENAMES' not in source
    source=source.replace('        from txd_store_migration import RENAMES as TXD_STORE_RENAMES','        from txd_store_migration import RENAMES as TXD_STORE_RENAMES\n        from tex_read_migration import RENAMES as TEX_READ_RENAMES')
    source=source.replace('**TXD_STORE_RENAMES}', '**TXD_STORE_RENAMES,**TEX_READ_RENAMES}')
    return source.replace("'memory-heap','txd-store']:","'memory-heap','txd-store','tex-read']:")

def txd_audit(source):
    # Keep archived build evidence while requiring complete current source and native successors
    source=source.replace('from stats_successor import verify','from stats_successor import verify,current_path')
    source=source.replace('path=tree/m.RENAMES.get(name,name);source=path.read_text()', 'path=current_path(root,name) if production else tree/m.RENAMES.get(name,name);source=path.read_text()')
    source=source.replace(".read_text()==m.native()", ".read_text()==(m.calls(m.native()) if production else m.native())")
    source=source.replace('assert m.project(project.read_text())==project.read_text()',"assert getattr(m,'txd_store_project',m.project)(project.read_text())==project.read_text()")
    return source.replace("if path.is_file():assert path.read_bytes()==(root/'src'/path.relative_to(out/'overlay/src')).read_bytes(),path","if path.is_file():verify(root,'src/'+path.relative_to(out/'overlay/src').as_posix(),hashlib.sha256(path.read_bytes()).hexdigest())")

def txd_history(source):
    # Resolve only explicit completed producer renames when proving every preceding TXD source projection
    source=source.replace('from stats_successor import verify','from stats_successor import verify,current_path')
    return source.replace("assert m.calls(expected)==(target/m.RENAMES.get(name,name)).read_text(),(stage,name)","assert m.calls(expected)==(current_path(root,name) if production else target/m.RENAMES.get(name,name)).read_text(),(stage,name)")

update('txd_store_migration.py',endpoint)
update('stats_successor.py',successor)
update('txd_store_stage_audit.py',txd_audit)
update('txd_store_history.py',txd_history)
print('Applied complete C texture loading and conversion policy, three C forwarding ABIs and all callers')
