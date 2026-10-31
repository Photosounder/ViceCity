"""Audit complete C TXD ownership, all callers, forwarding ABIs and integration evidence."""
from pathlib import Path
import hashlib,json,re,sys,xml.etree.ElementTree as ET
import txd_store_migration as m
from rect_migration import mask
from stats_successor import verify,current_path

root=Path(__file__).resolve().parents[2]
out=root/'.diagnostics/txd-store-c-tests'
saved=out/'before';production='--production' in sys.argv;tree=root if production else out/'overlay'
changes=json.loads((out/'changes.json').read_text())
assert len(changes)==29
for name in changes:
    # Protect the complete original producer while the candidate has outstanding consumer work
    if not production:assert (root/name).read_bytes()==(saved/name).read_bytes(),name
    path=current_path(root,name) if production else tree/m.RENAMES.get(name,name);source=path.read_text()
    assert source==m.calls((saved/name).read_text()) and m.calls(source)==source,name
    assert b'\r' not in path.read_bytes(),name
for name,digest in json.loads((out/'untouched.json').read_text()).items():
    # Reject concurrent source changes outside the explicit owner boundary
    if production:verify(root,name,digest)
    else:assert hashlib.sha256((root/name).read_bytes()).hexdigest()==digest,name
kernel=(tree/'src/rw/TxdStore.c').read_text();header=(tree/'src/rw/TxdStore.h').read_text()
assert kernel==m.kernel((saved/'src/rw/TxdStore.cpp').read_text(),(saved/'src/rw/TxdStore.h').read_text())
assert header==m.header()
assert (tree/'src/rw/TxdStore_backend.h').read_text()==m.backend_header()
assert (tree/'src/rw/TxdStore_native.cpp').read_text()==(m.calls(m.native()) if production else m.native())
assert not (tree/'src/rw/TxdStore.cpp').exists()
originals=[(saved/name).read_text() for name in m.BINDING_PATHS]
bindings=(tree/'src/rw/TxdStore_bindings.c').read_text()
assert bindings==m.bindings(originals) and m.calls(bindings)==bindings
assert (tree/'src/rw/TxdStore_bindings.h').read_text()==m.bindings_header(originals)
for path in (tree/'src').rglob('*.cpp'):
    # Require all whole consumers to stop referring to the removed class and forwarding definitions
    source=path.read_text()
    assert not re.search(r'\bCTxdStore\b',mask(source)),path
    assert not m.forwarder_definitions(source),path
for value in [kernel,header,m.backend_header(),bindings,m.bindings_header(originals)]:
    # Require C-only ownership and SDK-independent public record declarations
    assert not re.search(r'::|\b(?:class|new|delete|namespace|static_cast|RwStream|RwTexDictionary)\b',mask(value))
for _,name,_,_ in m.OPERATIONS:
    # Check every complete original out-of-line operation in the actual C owner
    assert len(re.findall(r'(?m)^.*\bTxdStore_'+name+r'\([^\n]*\)\n\{',mask(kernel)))==1,name
assert len(re.findall(r'(?m)^TxdDef \*TxdStore_GetSlot\(int slot\)\n\{',kernel))==1
assert 'bool TxdStore_isTxdLoaded(int slot);' in header
assert not re.search(r'TxdStore_isTxdLoaded\([^;]*\)\s*\{',mask(kernel))
report={'status':'staged-verified','production_changed':production,'changed_sources':29,'complete_C_operations':21,'C_forwarding_ABIs':27,'external_native_primitives':12,'existing_undefined_declarations_preserved':1}
for name,count in [('strict-compiles',32),('native-consumer-compiles',60),('native-owner-compiles',16),('native-runs',8),('native-layouts',2)]:
    # Inspect all native ABIs and runtime variants using complete producers and real C dependencies
    rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==count and all(row['exit']==0 for row in rows),name
    if name=='native-runs':
        assert {(r['bits'],r['optimization'],r['profile']) for r in rows}=={(a,b,c) for a in [32,64] for b in ['Od','O2'] for c in ['default','master']}
        for row in rows:
            gold='55113da228a6e98b' if row['bits']==32 else 'b938d732aafc1bc9'
            expected='Failed to load TXD\nFailed to load TXD\ncommands=36052 checks=2662 snapshots=23 bindings=27 created=528 removed=2 opens=5 closes=2 events=1135 hash='+gold
            assert row['before']==row['after']==expected,row
    if name=='native-layouts':
        assert {r['bits'] for r in rows}=={32,64}
        for row in rows:
            assert row['consumers']==['original-cpp','new-cpp','new-c']
            assert row['layout']==('TxdDef 28 4 texDict=0 refCount=4 name=8\n' if row['bits']==32 else 'TxdDef 32 8 texDict=0 refCount=8 name=12\n')
    report[name]=count
report['remaining']=['Full Debug/Release integration and historical source projections','Production application and checks']
if '--integration' in sys.argv:
    # Require complete evaluated source metadata and actual compiler records for both executable builds
    rows=json.loads((out/'evaluated-configurations.json').read_text());assert len(rows)==48
    for row in rows:
        assert row['exit']==0 and row['pairs']==28 and row['c_units']==2,row
        assert row['sources']==(422 if row['platform']=='Win32' and 'RW34' in row['configuration'] else 423),row
    project=out/'project/reVC.vcxproj';assert getattr(m,'txd_store_project',m.project)(project.read_text())==project.read_text()
    ns={'m':'http://schemas.microsoft.com/developer/msbuild/2003'}
    items=[node.get('Include') for node in ET.parse(project).findall('.//m:ClCompile',ns) if node.get('Include')]
    assert len(items)==len(set(items))==423
    for item in items:
        path=(project.parent/item.replace('\\','/')).resolve()
        assert path.is_relative_to((out/'overlay/src').resolve()) and path.exists(),path
    for configuration in ['debug','release']:
        build=json.loads((out/f'd3d9-{configuration}.json').read_text());log=Path(build['log']).read_text(errors='replace')
        assert build['exit']==0 and 'Build succeeded.' in log and '0 Error(s)' in log
        exe=Path(build['exe']);assert exe.is_file() and exe.stat().st_size==build['size']
        objdir=out/'project/obj/win-amd64-librw_d3d9-oal'/configuration.capitalize()
        commands=(objdir/'reVC.tlog/CL.command.1.tlog').read_text(encoding='utf-16').splitlines()
        for name in ['TxdStore.c','TxdStore_bindings.c','TxdStore_native.cpp']:
            # Inspect actual C11 compiler tracking records and the linked object timestamps
            source=(out/'overlay/src/rw'/name).resolve();index=commands.index('^'+str(source).upper());command=commands[index+1]
            assert str(source).upper() in command
            if name.endswith('.c'):assert '/TC ' in command and '/std:c11 ' in command
            obj=objdir/(source.stem+'.obj');assert obj.exists() and obj.stat().st_mtime>=source.stat().st_mtime
            assert exe.stat().st_mtime>=obj.stat().st_mtime
        report['d3d9-'+configuration]=build['size']
    history=json.loads((out/'historical-source-projections.json').read_text())
    assert len(history)==1189 and all(row['exit']==0 for row in history)
    report['history']=len(history);report['status']='ready-to-apply';report['remaining']=['Production application and checks']
    if production:
        # Require every applied source byte to match the complete candidate used for the two links
        for path in (out/'overlay/src').rglob('*'):
            if path.is_file():verify(root,'src/'+path.relative_to(out/'overlay/src').as_posix(),hashlib.sha256(path.read_bytes()).hexdigest())
        for name,count in [('production-consumer-compiles',60),('production-owner-compiles',16),('production-history',1189),('preceding-owner-audits',3)]:
            rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==count and all(row['exit']==0 for row in rows),name
            report[name]=count
        report['status']='applied-verified';report['remaining']=[]
(out/('verification-summary.json' if production else 'stage-summary.json')).write_text(json.dumps(report,indent=2),newline='\n')
print(json.dumps(report,indent=2))
