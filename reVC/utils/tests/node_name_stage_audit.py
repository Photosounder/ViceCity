"""Audit complete frame-name C ownership, native callback ABI and full D3D9 integration."""
from pathlib import Path
import hashlib,json,re,sys,xml.etree.ElementTree as ET
import node_name_migration as m
from rect_migration import mask
from stats_successor import current_path,verify
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/node-name-c-tests';saved=out/'before';production='--production' in sys.argv;tree=root if production else out/'overlay';changes=json.loads((out/'changes.json').read_text());assert len(changes)==2
for name in changes:
    # Require complete source projections and protect production until application
    original=(saved/name).read_text()
    if not production:assert (root/name).read_bytes()==(saved/name).read_bytes(),name
    path=current_path(root,name) if production else tree/m.RENAMES.get(name,name);value=path.read_text();assert value==m.calls(original) and m.calls(value)==value and b'\r' not in path.read_bytes(),name
for name,digest in json.loads((out/'untouched.json').read_text()).items():
    # Protect every source outside this complete plugin conversion
    if production:verify(root,name,digest)
    else:assert hashlib.sha256((root/name).read_bytes()).hexdigest()==digest,name
original=(saved/'src/rw/NodeName.cpp').read_text();kernel=(tree/'src/rw/NodeName.c').read_text();header=(tree/'src/rw/NodeName.h').read_text()
assert kernel==m.kernel(original) and header==m.header()
assert (tree/'src/rw/NodeName_native.cpp').read_text()==m.native() and (tree/'src/rw/NodeName_backend.h').read_text()==m.backend_header()
for value in [kernel,header,m.backend_header()]:assert not re.search(r'::|\b(?:class|new|delete|namespace|RwFrame|RwStream|RwInt32|static_cast)\b',mask(value))
for name in m.OPERATIONS:
    # Require every original owner operation once in the complete C producer
    assert len(re.findall(r'(?m)^'+name+r'\(',original))==len(re.findall(r'(?m)^'+name+r'\(',kernel))==1,name
assert 'static int32_t gPluginOffset;' in kernel and 'if(gPluginOffset > 0)' in kernel and 'if(gPluginOffset < 0)' in kernel
assert 'strncpy(NODENAMEEXT(dstObject), NODENAMEEXT(srcObject), 23);' in kernel and "NODENAMEEXT(object)[binaryLength] = '\\0';" in kernel
assert 'return gPluginOffset != -1;' in kernel and kernel.index('gPluginOffset = NodeName_HostRegister(')<kernel.index('NodeName_HostRegisterStream(')
report={'status':'staged-incomplete','changed_sources':2,'complete_C_operations':8,'public_C_operations':2,'native_SDK_primitive_leaves':4,'typed_SDK_callback_trampolines':2,'native_algorithm_bridges':0}
for name,count in [('strict-compiles',12),('native-source-compiles',14),('matrix-runs',4),('sanitized-runs',1),('native-runs',2)]:
    # Match every actual compiler and native ABI result to the complete original producer
    rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==count and all(row['exit']==0 for row in rows),name
    for row in rows:
        if 'before' in row:assert row['before']==row['after']=='cases=4096 checks=1764866 copies=3072 reads=3072 writes=3072 registrations=4608 reentries=512 hash=bbf04eacfc552a4d',row
    report[name]=len(rows)
report['remaining']=['Full build integration and production application']
if '--integration' in sys.argv:
    # Require all evaluated metadata, complete history chains and full Debug/Release links
    configs=json.loads((out/'evaluated-configurations.json').read_text());assert len(configs)==48
    for row in configs:assert row['exit']==0 and row['pairs']==28 and row['c_units']==1 and row['sources']==(416 if row['platform']=='Win32' and 'RW34' in row['configuration'] else 417),row
    project=out/'project/reVC.vcxproj';xml=project.read_text();assert getattr(m,'node_name_project',m.project)(xml)==xml;ns={'m':'http://schemas.microsoft.com/developer/msbuild/2003'}
    items=[node.get('Include') for node in ET.parse(project).findall('.//m:ClCompile',ns) if node.get('Include')];assert len(items)==417 and len(set(items))==417
    for item in items:
        # Prove every producer in both builds used the same coherent candidate tree
        path=(project.parent/item.replace('\\','/')).resolve();assert path.is_relative_to((out/'overlay/src').resolve()) and path.exists(),path
    history=json.loads((out/('production-history.json' if production else 'historical-source-projections.json')).read_text());assert len(history)==74 and all(row['exit']==0 for row in history)
    report['evaluated-configurations']=len(configs);report['historical-source-projections']=len(history)
    for configuration in ['debug','release']:
        # Inspect full compiler and linker success without launching a graphical process
        build=json.loads((out/f'd3d9-{configuration}.json').read_text());assert build['exit']==0;log=Path(build['log']).read_text(errors='replace');assert 'Build succeeded.' in log and '0 Error(s)' in log and 'NodeName.c' in log and 'NodeName_native.cpp' in log
        exe=Path(build['exe']);assert exe.is_file() and exe.stat().st_size==build['size'];report['d3d9-'+configuration]=build['size']
    report['status']='applied-verified' if production else 'ready-to-apply';report['remaining']=[] if production else ['Apply and verify real production consumers']
    if production:
        # Compare all production bytes with the coherent candidate used for the full linked executables
        for path in (out/'overlay/src').rglob('*'):
            if path.is_file():verify(root,'src/'+path.relative_to(out/'overlay/src').as_posix(),hashlib.sha256(path.read_bytes()).hexdigest())
        assert not (root/'src/rw/NodeName.cpp').exists()
        rows=json.loads((out/'production-source-compiles.json').read_text());assert len(rows)==14 and all(row['exit']==0 for row in rows);report['production-source-compiles']=len(rows)
(out/('verification-summary.json' if production else 'stage-summary.json')).write_text(json.dumps(report,indent=2),newline='\n');print(json.dumps(report,indent=2))
