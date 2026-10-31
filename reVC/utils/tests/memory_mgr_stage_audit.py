"""Audit complete C memory policy, exact allocator macros and real native integration."""
from pathlib import Path
import hashlib,json,re,sys,xml.etree.ElementTree as ET
import memory_mgr_migration as m
from rect_migration import mask
from stats_successor import current_path,verify
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/memory-mgr-c-tests';saved=out/'before';production='--production' in sys.argv;tree=root if production else out/'overlay';changes=json.loads((out/'changes.json').read_text());assert len(changes)==9
for name in changes:
    # Require each complete source projection and unchanged production until application
    original=(saved/name).read_text()
    if not production:assert (root/name).read_bytes()==(saved/name).read_bytes(),name
    path=current_path(root,name) if production else tree/m.RENAMES.get(name,name);value=path.read_text();assert value==m.calls(original) and m.calls(value)==value and b'\r' not in path.read_bytes(),name
for name,digest in json.loads((out/'untouched.json').read_text()).items():
    # Protect every source outside the complete memory owner and its explicit native callers
    if production:verify(root,name,digest)
    else:assert hashlib.sha256((root/name).read_bytes()).hexdigest()==digest,name
original=(saved/'src/rw/MemoryMgr.cpp').read_text();kernel=(tree/'src/rw/MemoryMgr.c').read_text();header=(tree/'src/rw/MemoryMgr.h').read_text();native=(tree/'src/rw/MemoryMgr_native.cpp').read_text()
assert kernel==m.kernel(original) and header==m.header() and native==(m.calls(m.native(original)) if production else m.native(original))
assert (tree/'src/rw/MemoryMgr_backend.h').read_text()==m.backend_header()
assert (tree/'src/rw/MemoryMgr_native.h').read_text()==m.native_header((saved/'src/rw/MemoryMgr.h').read_text())
for value in [kernel,header,m.backend_header()]:assert not re.search(r'::|\b(?:class|new|delete|namespace|RwMemoryFunctions|gMainHeap|static_cast)\b',mask(value))
for name in m.OPERATIONS:
    # Require all seven complete original memory operations exactly once in the C owner
    assert len(re.findall(r'(?m)^'+name+r'\(',original))==len(re.findall(r'(?m)^'+name+r'\(',kernel))==1,name
assert '#include <cita_windows.h>' in kernel and 'uint8_t *pMemoryTop;' in kernel
assert 'uintptr_t ptralign = align-1;' in kernel and 'size + sizeof(uintptr_t) + ptralign' in kernel and 'size + align' in kernel
assert '*(((void **)addr) - 1) = mem;' in kernel and '*(((void **)mem) - 1)' in kernel
table=re.search(r'RwMemoryFunctions memFuncs = \{[\s\S]*?\n\};',original)[0];assert table in native and table not in kernel
report={'status':'staged-incomplete','changed_sources':9,'complete_C_operations':7,'native_custom_heap_primitives':4,'native_SDK_callback_tables':1,'native_algorithm_bridges':0,'production_changed':production}
for name,count in [('strict-compiles',20),('native-consumer-compiles',18),('native-source-compiles',7),('matrix-runs',16),('sanitized-runs',4),('native-runs',8)]:
    # Require complete original/C producers and both actual native callback ABIs under every tested profile
    rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==count and all(row['exit']==0 for row in rows),name
    for row in rows:
        if 'before' in row:
            bits=row.get('bits',64);profile=row['profile'];custom=profile.startswith('custom');classic=profile.endswith('classic');checks=502373 if custom and classic else 500325 if custom else 525185
            gold32={'default':'8e0302ee8ef6ddca','classic':'c2733943b624f35b','custom':'91b531c64187312a','custom-classic':'19cd48cc7acd2429'}
            gold64={'default':'db11797266254738','classic':'db11797266254738','custom':'a06a8c6ea9e3a394','custom-classic':'c607dbdd39da0055'}
            high=5115 if bits==64 else 5108 if classic else 5111
            expected=f'cases=2048 checks={checks} allocations=8288 frees=8288 table=4096 reentries=96 init={2 if custom else 0} high={high} hash='+(gold32 if bits==32 else gold64)[profile]
            assert row['before']==row['after']==expected,row
    report[name]=len(rows)
limitations=json.loads((out/'native-existing-limitations.json').read_text());assert len(limitations)==2 and {row['phase'] for row in limitations}=={'before','after'}
for row in limitations:assert row['bits']==64 and row['profile']=='heap' and row['exit']!=0 and row['diagnostic']=="HeapBlockDesc must have 0x10 size otherwise most of assumptions don't make sense",row
report['existing_native_limitations']=limitations
report['remaining']=['Full integration and production application']
if '--integration' in sys.argv:
    # Require every evaluated configuration, exact historical proof and full Debug/Release linker result
    configs=json.loads((out/'evaluated-configurations.json').read_text());assert len(configs)==48
    for row in configs:assert row['exit']==0 and row['pairs']==28 and row['c_units']==1 and row['sources']==(418 if row['platform']=='Win32' and 'RW34' in row['configuration'] else 419),row
    project=out/'project/reVC.vcxproj';xml=project.read_text();assert getattr(m,'memory_mgr_project',m.project)(xml)==xml;ns={'m':'http://schemas.microsoft.com/developer/msbuild/2003'}
    items=[node.get('Include') for node in ET.parse(project).findall('.//m:ClCompile',ns) if node.get('Include')];assert len(items)==419 and len(set(items))==419
    for item in items:
        # Prove that both linked executables used the same coherent staged source tree
        path=(project.parent/item.replace('\\','/')).resolve();assert path.is_relative_to((out/'overlay/src').resolve()) and path.exists(),path
    history=json.loads((out/('production-history.json' if production else 'historical-source-projections.json')).read_text());assert len(history)==351 and all(row['exit']==0 for row in history)
    report['evaluated-configurations']=len(configs);report['historical-source-projections']=len(history)
    for configuration in ['debug','release']:
        # Inspect actual full compiler and linker success without running the game
        build=json.loads((out/f'd3d9-{configuration}.json').read_text());assert build['exit']==0;log=Path(build['log']).read_text(errors='replace');assert 'Build succeeded.' in log and '0 Error(s)' in log and 'MemoryMgr.c' in log and 'MemoryMgr_native.cpp' in log
        exe=Path(build['exe']);assert exe.is_file() and exe.stat().st_size==build['size'];report['d3d9-'+configuration]=build['size']
    report['status']='applied-verified' if production else 'ready-to-apply';report['remaining']=[] if production else ['Apply and verify production consumers']
    if production:
        # Require all production bytes to match the complete candidate used for both full links
        for path in (out/'overlay/src').rglob('*'):
            if path.is_file():verify(root,'src/'+path.relative_to(out/'overlay/src').as_posix(),hashlib.sha256(path.read_bytes()).hexdigest())
        assert not (root/'src/rw/MemoryMgr.cpp').exists()
        for name,count in [('production-consumer-compiles',18),('production-source-compiles',7)]:
            rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==count and all(row['exit']==0 for row in rows);report[name]=len(rows)
        assert json.loads((out/'production-existing-limitations.json').read_text())==limitations
(out/('verification-summary.json' if production else 'stage-summary.json')).write_text(json.dumps(report,indent=2),newline='\n');print(json.dumps(report,indent=2))
