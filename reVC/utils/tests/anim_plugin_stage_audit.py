"""Audit complete staged plugin ownership and native compilation without claiming runtime/integration completion."""
from pathlib import Path
import hashlib,json,re,sys,xml.etree.ElementTree as ET
import anim_plugin_migration as m
from rect_migration import mask
from stats_successor import current_path,verify
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/anim-plugin-c-tests';saved=out/'before';production='--production' in sys.argv;tree=root if production else out/'overlay';changes=json.loads((out/'changes.json').read_text());assert len(changes)==10
for name in changes:
    # Require exact complete projections and unchanged production throughout this staged conversion
    original=(saved/name).read_text()
    if not production:assert (root/name).read_bytes()==(saved/name).read_bytes(),name
    path=current_path(root,name) if production else tree/m.RENAMES.get(name,name);value=path.read_text();assert value==m.calls(original) and m.calls(value)==value and b'\r' not in path.read_bytes(),name
for name,digest in json.loads((out/'untouched.json').read_text()).items():
    # Protect every other source while the complete plugin owner is staged and tested
    if production:verify(root,name,digest)
    else:assert hashlib.sha256((root/name).read_bytes()).hexdigest()==digest,name
original=(saved/'src/animation/RpAnimBlend.cpp').read_text();kernel=(tree/'src/animation/RpAnimBlend.c').read_text();header=(tree/'src/animation/RpAnimBlend.h').read_text()
assert kernel==m.kernel(original) and header==m.header()
assert (tree/'src/animation/RpAnimBlend_native.cpp').read_text()==(m.calls(m.native(original)) if production else m.native(original))
assert (tree/'src/animation/RpAnimBlend_backend.h').read_text()==m.backend_header()
for value in [kernel,header,m.backend_header()]:assert not re.search(r'::|\b(?:class|new|delete|namespace|RpClump|RwFrame|RpSkin|RpAtomic|RpHAnimHierarchy|RwV3d|RwInt32|static_cast)\b',mask(value))
for result,name,args in m.DECLARATIONS:
    # Require each public C operation once and preserve every complete original owner operation
    assert header.count(result+' '+name+'('+args+');')==1,name
    assert len(re.findall(r'(?m)^'+name+r'\(',kernel))==1,name
prefix=original[:original.index('void *AnimBlendClump_BackendAllocate(')]
definitions=re.findall(r'(?m)^(?:void\*|void|bool|CAnimBlendAssociation\*|AnimBlendFrameData\*|RwFrame\*)\n(\w+)\(',prefix)+['AnimBlendClumpCopy']
assert len(definitions)==35
assert kernel.count('next = link->next;')==prefix.count('next = link->next;')
assert 'if(i < 11)' in kernel and 'if(assocRet) *assocRet = secondAssoc;' in kernel
assert 'if(blendRet) *blendRet = secondBlend;' in kernel and 'totalLength == 0.0f ? 1.0f : totalBlend/totalLength' in kernel
strict=json.loads((out/'strict-compiles.json').read_text());native=json.loads((out/'native-source-compiles.json').read_text())
assert len(strict)==24 and len(native)==20 and all(row['exit']==0 for row in strict+native)
report={'status':'staged-incomplete','changed_paths':len(changes),'public_C_operations':len(m.DECLARATIONS),'complete_original_owner_operations':len(definitions),'C_forwarders_from_previous_native_interfaces':3,'native_SDK_primitive_leaves':16,'native_existing_allocator_leaves':8,'SDK_callback_trampolines':1,'strict_producer_API_compiles':len(strict),'native_source_compiles':len(native)}
for name,count in [('matrix-runs',16),('sanitized-runs',4),('native-runs',8)]:
    # Require complete original/C producer comparisons with real underlying animation algorithms
    rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==count and all(row['exit']==0 for row in rows),name
    for row in rows:
        # Preserve registration, hierarchy setup, lookups, masked deletion, callback order and playback results
        gold='056368dc66581caf' if 'compression' in row['profile'] else '5a59951492017532'
        expected='queries=4096 hierarchies=128 removals=256 playback=1536 checks=62499 registrations=2 enumerations=5320 allocations=5387 delete=256 finish=1153 hash='+gold
        assert row['before']==row['after']==expected,row
    report[name]=len(rows)
layouts=json.loads((out/'native-layouts.json').read_text());assert len(layouts)==6 and all(row['exit']==0 for row in layouts);report['native-layouts']=len(layouts)
report['remaining']=['Build configurations, historical integration and full D3D9 Debug/Release links','Earlier animation fixture and native allocator boundary compatibility','Production application and verification']
if '--integration' in sys.argv:
    # Require evaluated C11 metadata, whole historical proofs and both complete D3D9 links
    configs=json.loads((out/'evaluated-configurations.json').read_text());assert len(configs)==48
    for row in configs:assert row['exit']==0 and row['pairs']==28 and row['c_units']==1 and row['sources']==(415 if row['platform']=='Win32' and 'RW34' in row['configuration'] else 416),row
    project=out/'project/reVC.vcxproj';xml=project.read_text();assert getattr(m,'anim_plugin_project',m.project)(xml)==xml;ns={'m':'http://schemas.microsoft.com/developer/msbuild/2003'}
    items=[node.get('Include') for node in ET.parse(project).findall('.//m:ClCompile',ns) if node.get('Include')];assert len(items)==416 and len(set(items))==416
    for item in items:
        # Prove every linked source used the same coherent and fully tested candidate tree
        path=(project.parent/item.replace('\\','/')).resolve();assert path.is_relative_to((out/'overlay/src').resolve()) and path.exists(),path
    history=json.loads((out/('production-history.json' if production else 'historical-source-projections.json')).read_text());assert len(history)==360 and all(row['exit']==0 for row in history)
    report['evaluated-configurations']=len(configs);report['historical-source-projections']=len(history)
    for configuration in ['debug','release']:
        # Inspect full compiler and linker success without launching any game executable
        build=json.loads((out/f'd3d9-{configuration}.json').read_text());assert build['exit']==0;log=Path(build['log']).read_text(errors='replace');assert 'Build succeeded.' in log and '0 Error(s)' in log and 'RpAnimBlend.c' in log and 'RpAnimBlend_native.cpp' in log
        exe=Path(build['exe']);assert exe.is_file() and exe.stat().st_size==build['size'];report['d3d9-'+configuration]=build['size']
    report['status']='applied-verified' if production else 'ready-to-apply';report['remaining']=[] if production else ['Apply and verify real production consumers']
    if production:
        # Require every actual source byte to match the coherent candidate used for both full links
        for path in (out/'overlay/src').rglob('*'):
            if path.is_file():verify(root,'src/'+path.relative_to(out/'overlay/src').as_posix(),hashlib.sha256(path.read_bytes()).hexdigest())
        assert not (root/'src/animation/RpAnimBlend.cpp').exists()
        rows=json.loads((out/'production-source-compiles.json').read_text());assert len(rows)==20 and all(row['exit']==0 for row in rows);report['production-source-compiles']=len(rows)
(out/('verification-summary.json' if production else 'stage-summary.json')).write_text(json.dumps(report,indent=2),newline='\n');print(json.dumps(report,indent=2))
