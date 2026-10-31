"""Audit the whole staged heap owner, exact caller projections and native integration evidence."""
from pathlib import Path
import hashlib,json,re,sys,xml.etree.ElementTree as ET
import memory_heap_migration as m
from memory_heap_schema import DECLARATIONS
from rect_migration import mask
from stats_successor import verify

root=Path(__file__).resolve().parents[2]
out=root/'.diagnostics/memory-heap-c-tests'
saved=out/'before'
production='--production' in sys.argv
tree=root if production else out/'overlay'
changes=json.loads((out/'changes.json').read_text())
assert len(changes)==8
for name in changes:
    # Require complete deterministic source projections and an unchanged production baseline
    if not production:assert (root/name).read_bytes()==(saved/name).read_bytes(),name
    path=tree/m.RENAMES.get(name,name)
    value=path.read_text()
    assert value==m.calls((saved/name).read_text()) and m.calls(value)==value,name
    assert b'\r' not in path.read_bytes(),name
for name,digest in json.loads((out/'untouched.json').read_text()).items():
    # Protect every production source outside the explicitly recorded conversion boundary
    if production:verify(root,name,digest)
    else:assert hashlib.sha256((root/name).read_bytes()).hexdigest()==digest,name
kernel=(tree/'src/rw/MemoryHeap.c').read_text()
header=(tree/'src/rw/MemoryHeap.h').read_text()
assert kernel==m.kernel((saved/'src/rw/MemoryHeap.cpp').read_text(),(saved/'src/rw/MemoryHeap.h').read_text())
assert header==m.header((saved/'src/rw/MemoryHeap.h').read_text())
assert (tree/'src/rw/MemoryHeap_native.cpp').read_text()==m.native()
assert (tree/'src/rw/MemoryHeap_backend.h').read_text()==m.backend_header()
originals=[(saved/name).read_text() for name in ['src/rw/MemoryMgr_native.cpp','src/collision/ColModel_native.cpp','src/animation/RpAnimBlend_native.cpp']]
bindings=(tree/'src/rw/MemoryHeap_bindings.c').read_text()
assert bindings==m.bindings(originals) and m.calls(bindings)==bindings
for _,name,_ in DECLARATIONS:
    # Require each complete original operation exactly once in the actual C producer
    assert len(re.findall(r'(?m)^.*\b'+name+r'\([^\n]*\)\n\{',mask(kernel)))==1,name
for value in [kernel,bindings]:
    # Reject C++ syntax and algorithm forwarding back into native heap methods
    assert not re.search(r'::|\b(?:class|new|delete|namespace|static_cast)\b',mask(value))
for path in (tree/'src').rglob('*.cpp'):
    # Ensure no whole-source consumer still invokes the removed heap member API
    source=path.read_text()
    assert not re.search(r'\bgMainHeap\.\w+\(',mask(source)),path
    assert not m.forwarder_definitions(source),path
assert not (tree/'src/rw/MemoryHeap.cpp').exists()
report={'status':'staged-verified','changed_sources':len(changes),'complete_C_operations':33,'C_forwarding_ABIs':11,'native_external_primitives':17,'production_changed':production}
for name,count in [('strict-compiles',28),('native-consumer-compiles',18),('native-source-compiles',8),('native-runs',4),('native-layouts',2)]:
    # Check complete real compiler and runtime observations rather than isolated declarations alone
    rows=json.loads((out/(name+'.json')).read_text())
    assert len(rows)==count and all(row['exit']==0 for row in rows),name
    if name=='native-runs':
        expected='commands=23742 checks=113613 snapshots=47 backings=8 pressure=14 dump=73727 moved=0 hash=5b829d3343627211'
        assert {(r['optimization'],r['profile']) for r in rows}=={(a,b) for a in ['Od','O2'] for b in ['fixed','classic']}
        assert all(row['before']==row['after']==expected for row in rows)
    report[name]=count
assert len(json.loads((out/'strict-existing-limitations.json').read_text()))==4
assert len(json.loads((out/'native-existing-limitations.json').read_text()))==4
report['remaining']=['Full Debug/Release integration and historical source projections','Production application and checks']
if '--integration' in sys.argv:
    # Require every evaluated configuration and both full executable links from the coherent candidate
    rows=json.loads((out/'evaluated-configurations.json').read_text())
    assert len(rows)==48
    for row in rows:
        assert row['exit']==0 and row['pairs']==28 and row['c_units']==2
        assert row['sources']==(420 if row['platform']=='Win32' and 'RW34' in row['configuration'] else 421),row
    project=out/'project/reVC.vcxproj'
    assert getattr(m,'memory_heap_project',m.project)(project.read_text())==project.read_text()
    ns={'m':'http://schemas.microsoft.com/developer/msbuild/2003'}
    items=[node.get('Include') for node in ET.parse(project).findall('.//m:ClCompile',ns) if node.get('Include')]
    assert len(items)==len(set(items))==421
    for item in items:
        path=(project.parent/item.replace('\\','/')).resolve()
        assert path.is_relative_to((out/'overlay/src').resolve()) and path.exists(),path
    for configuration in ['debug','release']:
        build=json.loads((out/f'd3d9-{configuration}.json').read_text())
        log=Path(build['log']).read_text(errors='replace')
        assert build['exit']==0 and 'Build succeeded.' in log and '0 Error(s)' in log
        exe=Path(build['exe'])
        assert exe.is_file() and exe.stat().st_size==build['size']
        objdir=out/'project/obj/win-amd64-librw_d3d9-oal'/configuration.capitalize()
        commands=(objdir/'reVC.tlog/CL.command.1.tlog').read_text(encoding='utf-16').splitlines()
        for name in ['MemoryHeap.c','MemoryHeap_bindings.c','MemoryHeap_native.cpp']:
            # Inspect actual compiler tracking records even when the final successful link was incremental
            source=(out/'overlay/src/rw'/name).resolve()
            key='^'+str(source).upper()
            index=commands.index(key)
            command=commands[index+1]
            assert str(source).upper() in command
            if name.endswith('.c'):assert '/TC ' in command and '/std:c11 ' in command
            obj=objdir/(source.stem+'.obj')
            assert obj.exists() and obj.stat().st_mtime>=source.stat().st_mtime
            assert exe.stat().st_mtime>=obj.stat().st_mtime
        report['d3d9-'+configuration]=build['size']
    history=json.loads((out/'historical-source-projections.json').read_text())
    assert history and all(row['exit']==0 for row in history)
    report['history']=len(history)
    report['status']='ready-to-apply'
    report['remaining']=['Production application and checks']
    if production:
        # Require the applied tree to match every candidate byte used for both executable links
        for path in (out/'overlay/src').rglob('*'):
            if path.is_file():verify(root,'src/'+path.relative_to(out/'overlay/src').as_posix(),hashlib.sha256(path.read_bytes()).hexdigest())
        for name,count in [('production-consumer-compiles',18),('production-source-compiles',8),('production-history',320),('preceding-owner-audits',3)]:
            rows=json.loads((out/(name+'.json')).read_text())
            assert len(rows)==count and all(row['exit']==0 for row in rows),name
            report[name]=count
        assert json.loads((out/'production-existing-limitations.json').read_text())==json.loads((out/'native-existing-limitations.json').read_text())
        report['status']='applied-verified'
        report['remaining']=[]
(out/('verification-summary.json' if production else 'stage-summary.json')).write_text(json.dumps(report,indent=2),newline='\n')
print(json.dumps(report,indent=2))
