"""Audit the staged complete C model owner without claiming caller or lifetime integration is finished."""
from pathlib import Path
import hashlib,json,re,sys,xml.etree.ElementTree as ET
import col_model_migration as m
from rect_migration import mask
from stats_successor import current_path,verify
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/col-model-c-tests';saved=out/'before';production='--production' in sys.argv;tree=root if production else out/'overlay';changes=json.loads((out/'changes.json').read_text());assert len(changes)==16
for name in changes:
    # Require every whole staged projection and protect unchanged production throughout this phase
    original=(saved/name).read_text()
    if not production:assert (root/name).read_bytes()==(saved/name).read_bytes(),name
    path=current_path(root,name) if production else tree/m.RENAMES.get(name,name);value=path.read_text();assert value==m.calls(original) and m.calls(value)==value and b'\r' not in path.read_bytes(),name
for name,digest in json.loads((out/'untouched.json').read_text()).items():
    # Protect all real production callers until their lifetime and copy semantics have been adapted
    if production:verify(root,name,digest)
    else:assert hashlib.sha256((root/name).read_bytes()).hexdigest()==digest,name
original=(saved/'src/collision/ColModel.cpp').read_text();kernel=(tree/'src/collision/ColModel.c').read_text();header=(tree/'src/collision/ColModel.h').read_text()
assert kernel==m.kernel(original) and header==m.header()
assert (tree/'src/collision/ColModel_native.cpp').read_text()==(m.calls(m.native()) if production else m.native()) and (tree/'src/collision/ColModel_backend.h').read_text()==m.backend_header()
for value in [kernel,header,m.backend_header()]:assert not re.search(r'::|\b(?:class|new|delete|namespace|CVector|RwMalloc|RwFree|static_cast)\b',mask(value))
for result,name,args in m.DECLARATIONS:
    # Require all nine complete original owner operations once in the C implementation and public API
    assert header.count(result+' ColModel_'+name+'('+args+');')==1,name
    assert kernel.count(result+' ColModel_'+name+'('+args+')\n')==1,name
assert 'self->level = LEVEL_GENERIC;' in kernel and 'self->ownsCollisionVolumes = true;' in kernel
assert 'sizeof(CColTrianglePlane) * (self->numTriangles+1)' in kernel and 'ColModel_BackendRegister(&self->trianglePlanes);' in kernel
assert '*(CLink**)ALIGNPTR(&self->trianglePlanes[self->numTriangles])' in kernel
report={'status':'staged-incomplete','complete_C_model_operations':9,'changed_staged_paths':16,'additional_C_lifetime_operations':2,'public_C_operations':11,'native_primitive_leaves':6,'native_algorithm_bridges':0,'production_changed':production}
for name,count in [('strict-compiles',24),('native-source-compiles',7),('native-layouts',18),('matrix-runs',16),('sanitized-runs',4),('native-runs',8)]:
    # Require complete producer comparisons and real native layout evidence under every tested configuration
    rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==count and all(row['exit']==0 for row in rows),name
    for row in rows:
        if 'before' in row:
            heap=row['profile']=='heap';gold={'default':'373d90698c5aff5c','compressed':'672073b0cd1d77a8','vu':'0f816e877f028608','heap':'27db5955bd20e276'}[row['profile']]
            expected='cases=4096 checks='+('519506' if heap else '501074')+' assignments=4096 planes=4096 allocations='+('49139' if heap else '47091')+' frees='+('49139' if heap else '47091')+' pushes='+('4096' if heap else '0')+' registrations='+('4096' if heap else '0')+' relocations='+('2048' if heap else '0')+' hash='+gold
            assert row['before']==row['after']==expected,row
    report[name]=len(rows)
limitations=json.loads((out/'native-existing-limitations.json').read_text());assert len(limitations)==2 and {row['phase'] for row in limitations}=={'before','after'}
for row in limitations:assert row['bits']==64 and row['profile']=='heap' and row['exit']!=0 and row['diagnostic']=="HeapBlockDesc must have 0x10 size otherwise most of assumptions don't make sense",row
if production:
    # Require the complete heap-header successor while preserving the original descriptor restriction
    verify(root,'src/rw/MemoryHeap.h',hashlib.sha256((saved/'src/rw/MemoryHeap.h').read_bytes()).hexdigest())
else:assert (tree/'src/rw/MemoryHeap.h').read_bytes()==(saved/'src/rw/MemoryHeap.h').read_bytes()==(root/'src/rw/MemoryHeap.h').read_bytes()
report['existing_native_limitations']=limitations
for name,count in [('native-consumer-compiles',30),('lifetime-matrix-runs',16),('lifetime-sanitized-runs',4),('lifetime-native-runs',8)]:
    # Require complete native consumers and exact parent assignment and process-exit observations
    rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==count and all(row['exit']==0 for row in rows),name
    for row in rows:
        if 'before' in row:
            gold='4a282c0502b5b80f' if row['profile'] in ['compressed','vu'] else '357783df940efe8b'
            assert row['before']==row['after']=='parents=1024 static=20 boundaries=4515 checks=136493 allocations=10286 exit=20 hash='+gold,row
    report[name]=len(rows)
parent=json.loads((out/'native-parent-layouts.json').read_text());assert len(parent)==2 and all(row['exit']==0 and row['before']==row['after'] and len(row['before']['fields'])==60 and row['before']['size']==(368 if row['bits']==32 else 416) for row in parent);report['native-parent-layouts']=len(parent)
report['remaining']=['Full integration audit and production application']
if '--integration' in sys.argv:
    # Require all evaluated metadata, complete history chains and full Debug/Release links
    configs=json.loads((out/'evaluated-configurations.json').read_text());assert len(configs)==48
    for row in configs:assert row['exit']==0 and row['pairs']==28 and row['c_units']==1 and row['sources']==(417 if row['platform']=='Win32' and 'RW34' in row['configuration'] else 418),row
    project=out/'project/reVC.vcxproj';xml=project.read_text();assert getattr(m,'col_model_project',m.project)(xml)==xml;ns={'m':'http://schemas.microsoft.com/developer/msbuild/2003'}
    items=[node.get('Include') for node in ET.parse(project).findall('.//m:ClCompile',ns) if node.get('Include')];assert len(items)==418 and len(set(items))==418
    for item in items:
        # Prove every producer in both builds used the same coherent candidate tree
        path=(project.parent/item.replace('\\','/')).resolve();assert path.is_relative_to((out/'overlay/src').resolve()) and path.exists(),path
    history=json.loads((out/('production-history.json' if production else 'historical-source-projections.json')).read_text());assert len(history)==608 and all(row['exit']==0 for row in history)
    report['evaluated-configurations']=len(configs);report['historical-source-projections']=len(history)
    for configuration in ['debug','release']:
        # Inspect full compiler and linker success without launching a graphical process
        build=json.loads((out/f'd3d9-{configuration}.json').read_text());assert build['exit']==0;log=Path(build['log']).read_text(errors='replace');assert 'Build succeeded.' in log and '0 Error(s)' in log and 'ColModel.c' in log and 'ColModel_native.cpp' in log
        exe=Path(build['exe']);assert exe.is_file() and exe.stat().st_size==build['size'];report['d3d9-'+configuration]=build['size']
    report['status']='applied-verified' if production else 'ready-to-apply';report['remaining']=[] if production else ['Apply and verify real production consumers']
    if production:
        # Compare all production bytes with the coherent candidate used for the full linked executables
        for path in (out/'overlay/src').rglob('*'):
            if path.is_file():verify(root,'src/'+path.relative_to(out/'overlay/src').as_posix(),hashlib.sha256(path.read_bytes()).hexdigest())
        assert not (root/'src/collision/ColModel.cpp').exists()
        rows=json.loads((out/'production-consumer-compiles.json').read_text());assert len(rows)==30 and all(row['exit']==0 for row in rows);report['production-consumer-compiles']=len(rows)
        rows=json.loads((out/'production-source-compiles.json').read_text());assert len(rows)==7 and all(row['exit']==0 for row in rows);report['production-source-compiles']=len(rows)
        assert json.loads((out/'production-existing-limitations.json').read_text())==limitations
(out/('verification-summary.json' if production else 'stage-summary.json')).write_text(json.dumps(report,indent=2),newline='\n');print(json.dumps(report,indent=2))
