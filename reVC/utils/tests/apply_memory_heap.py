"""Apply the entire C heap and caller migration after complete staged integration checks."""
from pathlib import Path
import hashlib,json,subprocess,sys
import memory_heap_migration as m

root=Path(__file__).resolve().parents[2]
out=root/'.diagnostics/memory-heap-c-tests'
subprocess.run([sys.executable,str(root/'utils/tests/memory_heap_stage_audit.py'),'--integration'],check=True)
changes=json.loads((out/'changes.json').read_text())
for name in changes:
    # Reject concurrent production edits before applying the exact compiled candidate
    assert (root/name).read_bytes()==(out/'before'/name).read_bytes(),name
for name,digest in json.loads((out/'untouched.json').read_text()).items():
    # Keep every source outside the complete owner and caller boundary unchanged
    assert hashlib.sha256((root/name).read_bytes()).hexdigest()==digest,name
for old,new in m.RENAMES.items():
    # Rename only the verified heap producer inside the intended workspace
    source=(root/old).resolve();target=(root/new).resolve()
    assert source.is_relative_to(root.resolve()) and target.is_relative_to(root.resolve()) and not target.exists()
    source.rename(target)
for name in changes:
    # Apply complete verified source files with LF line endings
    target=root/m.RENAMES.get(name,name)
    target.write_text((out/'overlay'/m.RENAMES.get(name,name)).read_text(),newline='\n')
for name in ['MemoryHeap_backend.h','MemoryHeap_native.cpp','MemoryHeap_bindings.c']:
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
    # Extend the prior owner endpoint with the complete C heap rather than weakening historical checks
    assert 'def memory_mgr_calls(' not in source
    source=source.replace('def calls(source):','def memory_mgr_calls(source):').replace('def project(source):','def memory_mgr_project(source):')
    return source+'''

def calls(source):
    # Retain complete memory-manager ownership before applying the complete C heap successor
    import memory_heap_migration as heap
    return heap.calls(memory_mgr_calls(source))

def project(source):
    # Preserve preceding C metadata before selecting both complete C heap units
    import memory_heap_migration as heap
    return heap.project(memory_mgr_project(source))
'''

def successor(source):
    # Record the new producer rename and require exact whole-source heap projections
    assert 'MEMORY_HEAP_RENAMES' not in source
    source=source.replace('        from memory_mgr_migration import RENAMES as MEMORY_MGR_RENAMES','        from memory_mgr_migration import RENAMES as MEMORY_MGR_RENAMES\n        from memory_heap_migration import RENAMES as MEMORY_HEAP_RENAMES')
    source=source.replace('**MEMORY_MGR_RENAMES}', '**MEMORY_MGR_RENAMES,**MEMORY_HEAP_RENAMES}')
    return source.replace("'col-model','memory-mgr']:","'col-model','memory-mgr','memory-heap']:")

def memory_audit(source):
    # Preserve archived build evidence while checking the complete current native successor
    source=source.replace('native==m.native(original)','native==(m.calls(m.native(original)) if production else m.native(original))')
    source=source.replace('assert m.project(xml)==xml','assert getattr(m,\'memory_mgr_project\',m.project)(xml)==xml')
    source=source.replace("if path.is_file():assert path.read_bytes()==(root/'src'/path.relative_to(out/'overlay/src')).read_bytes(),path","if path.is_file():verify(root,'src/'+path.relative_to(out/'overlay/src').as_posix(),hashlib.sha256(path.read_bytes()).hexdigest())")
    return source

def model_audit(source):
    # Require the complete C heap successor for model forwarders while retaining the archived model fixture
    source=source.replace(".read_text()==m.native() and", ".read_text()==(m.calls(m.native()) if production else m.native()) and")
    original="assert (tree/'src/rw/MemoryHeap.h').read_bytes()==(saved/'src/rw/MemoryHeap.h').read_bytes()==(root/'src/rw/MemoryHeap.h').read_bytes()"
    replacement="if production:\n    # Require the complete heap-header successor while preserving the original descriptor restriction\n    verify(root,'src/rw/MemoryHeap.h',hashlib.sha256((saved/'src/rw/MemoryHeap.h').read_bytes()).hexdigest())\nelse:"+original
    return source.replace(original,replacement)

def sequence_audit(source):
    # Verify relocated sequence forwarding bodies only under the exact complete C heap projection
    original="for name in ['Allocate','Free','PushAnimation','PopMemory','Register','MoveMemory']:assert method(rp,'AnimSequence_Backend'+name)==method(m.backend(),'AnimSequence_Backend'+name),name"
    replacement="if (root/'src/rw/MemoryHeap_bindings.c').exists():\n    # Require the complete C heap successor before checking all relocated sequence forwarding ABIs\n    import memory_heap_migration as heap\n    savedHeap=root/'.diagnostics/memory-heap-c-tests/before'\n    originals=[(savedHeap/name).read_text() for name in ['src/rw/MemoryMgr_native.cpp','src/collision/ColModel_native.cpp','src/animation/RpAnimBlend_native.cpp']]\n    bindings=(root/'src/rw/MemoryHeap_bindings.c').read_text();assert bindings==heap.bindings(originals)\n    for name in ['PushAnimation','PopMemory','Register','MoveMemory']:\n        assert 'AnimSequence_Backend'+name+'(' not in rp,name\n        assert method(bindings,'AnimSequence_Backend'+name)==heap.heap_calls(method(m.backend(),'AnimSequence_Backend'+name)),name\n    names=['Allocate','Free']\nelse:names=['Allocate','Free','PushAnimation','PopMemory','Register','MoveMemory']\nfor name in names:assert method(rp,'AnimSequence_Backend'+name)==method(m.backend(),'AnimSequence_Backend'+name),name"
    return source.replace(original,replacement)

update('memory_mgr_migration.py',endpoint)
update('stats_successor.py',successor)
update('memory_mgr_stage_audit.py',memory_audit)
update('col_model_stage_audit.py',model_audit)
update('finalize_anim_sequence.py',sequence_audit)
print('Applied all 33 complete C heap operations, 11 C forwarding ABIs and their complete callers')
