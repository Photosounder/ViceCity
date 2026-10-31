"""Apply all C post-processing policy and live SDK clients after complete staged verification."""
from pathlib import Path
import hashlib,json,subprocess,sys
import postfx_migration as m

root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/postfx-c-tests'
subprocess.run([sys.executable,str(root/'utils/tests/postfx_stage_audit.py'),'--integration'],check=True)
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
for name in ['src/extras/PostFX_backend.h','src/extras/PostFX_native.cpp','src/extras/PostFX_bindings.c']:
    # Add only the verified C constants and typed SDK primitive boundary
    target=root/name;assert not target.exists();target.write_text((out/'overlay'/name).read_text(),newline='\n')

def update(name,transform):
    # Preserve the previous complete verifier before adding an explicit successor projection
    path=root/'utils/tests'/name;backup=out/'before/utils/tests'/name;assert not backup.exists();backup.parent.mkdir(parents=True,exist_ok=True);backup.write_bytes(path.read_bytes());source=path.read_text();value=transform(source);assert value!=source,name;path.write_text(value,newline='\n')

def endpoint(source):
    # Preserve frozen original motion-blur fixtures while extending full source and project projections
    assert 'def mblur_calls(' not in source
    source=source.replace('def calls(source):','def mblur_calls(source):').replace('def project(source):','def mblur_project(source):')
    return source+'''

def calls(source):
    # Retain all preceding C blur policy before the complete C post-processing successor
    import postfx_migration as post
    return post.calls(mblur_calls(source))

def project(source):
    # Select C post-processing policy and forwarding after all preceding project metadata
    import postfx_migration as post
    return post.project(mblur_project(source))
'''

def successor(source):
    # Record the explicit producer rename and full-source successor without weakening digest validation
    assert 'POSTFX_RENAMES' not in source
    return source.replace('        from mblur_migration import RENAMES as MBLUR_RENAMES','        from mblur_migration import RENAMES as MBLUR_RENAMES\n        from postfx_migration import RENAMES as POSTFX_RENAMES').replace('**MBLUR_RENAMES}','**MBLUR_RENAMES,**POSTFX_RENAMES}').replace("'texture-pools','mblur']:","'texture-pools','mblur','postfx']:")

def blur_audit(source):
    # Resolve the renamed post-processing client and preserve the exact archived motion-blur project
    source=source.replace('from stats_successor import verify','from stats_successor import verify,current_path').replace('path=tree/m.RENAMES.get(name,name);','path=current_path(root,name) if production else tree/m.RENAMES.get(name,name);').replace('native==m.native()','native==m.calls(m.native())').replace('assert m.project(project.read_text())==project.read_text()',"assert getattr(m,'mblur_project',m.project)(project.read_text())==project.read_text()")
    return source.replace("if path.is_file():assert path.read_bytes()==(root/'src'/path.relative_to(out/'overlay/src')).read_bytes(),path","if path.is_file():verify(root,'src/'+path.relative_to(out/'overlay/src').as_posix(),hashlib.sha256(path.read_bytes()).hexdigest())")

def blur_history(source):
    # Resolve only the explicit renamed producer when checking the complete applied historical projection
    return source.replace('from stats_successor import verify','from stats_successor import verify,current_path').replace('(target/m.RENAMES.get(name,name)).read_text()','(current_path(root,name) if production else target/m.RENAMES.get(name,name)).read_text()')

update('mblur_migration.py',endpoint)
update('stats_successor.py',successor)
update('mblur_stage_audit.py',blur_audit)
update('mblur_history.py',blur_history)
print('Applied all 13 complete C PostFX operations, four C forwarding operations and typed SDK primitives')
