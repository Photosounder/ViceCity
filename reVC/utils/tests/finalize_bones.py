"""Verify complete C bone mappings, native model layouts, public linkage and full project integration."""
from pathlib import Path
import hashlib,json,re,sys,xml.etree.ElementTree as ET
import bones_migration as m
from rect_migration import mask
from stats_successor import current_path,verify
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/bones-c-tests';saved=out/'before';production='--production' in sys.argv;tree=root if production else out/'overlay';changes=json.loads((out/'changes.json').read_text());assert len(changes)==3
for name in changes:
    # Require exact complete projections and LF line endings at each owner source boundary
    path=current_path(root,name) if production else tree/m.RENAMES.get(name,name);expected=m.calls((saved/name).read_text());assert path.read_text()==expected and m.calls(expected)==expected and b'\r' not in path.read_bytes(),name
    if not production:assert (root/name).read_bytes()==(saved/name).read_bytes(),name
for name,digest in json.loads((out/'untouched.json').read_text()).items():
    # Reject unrelated edits or accept only a complete explicitly verified later conversion
    if production:verify(root,name,digest)
    else:assert hashlib.sha256((root/name).read_bytes()).hexdigest()==digest,name
ped=(tree/'src/modelinfo/PedNode.h').read_text();assert ped==m.ped_header((saved/'src/modelinfo/PedModelInfo.h').read_text())
kernel=(tree/'src/animation/Bones.c').read_text();header=(tree/'src/animation/Bones.h').read_text()
assert not (tree/'src/animation/Bones.cpp').exists()
for source in [ped,kernel,header]:assert not re.search(r'::|\b(?:class|new|delete|virtual|namespace|CVector|RpClump|CPedModelInfo)\b',mask(source))
assert 're3_assert(#expression,__FILE__,__LINE__,__func__)' in kernel and '#define Bones_Assert(expression) (expression)' in kernel
assert 'Bones_Assert(0 && "this node has no bone");\n\treturn -1;' in kernel
assert kernel.count('return NULL;')==1 and header.count('extern "C" {')==1
gold={'default':'inputs=4101 checks=24577 reports=4084 hash=0c0682c36e8e1281','master':'inputs=4101 checks=12325 reports=0 hash=f73567a2526e1b1a'};report={}
for name,count in [('matrix-runs',8),('sanitized-runs',2),('native-runs',4)]+([('production-native-runs',4)] if production else []):
    # Require matching complete runtime outputs and the original continuing-assertion behavior
    rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==count and all(row['exit']==0 for row in rows),name
    for row in rows:assert row['before']==row['after']==gold[row['profile']],row
    report[name]=len(rows)
for prefix in ['', 'production-'] if production else ['']:
    # Verify every real caller plus both native model/enum layouts rather than relying on runtime fixture declarations
    sources=json.loads((out/(prefix+'native-source-compiles.json')).read_text());assert len(sources)==14 and all(row['exit']==0 for row in sources)
    layouts=json.loads((out/(prefix+'native-layouts.json')).read_text());assert len(layouts)==2 and all(row['exit']==0 and row['before']==row['after'] and row['after'][6:]==[4,4,18,35] for row in layouts)
    report[prefix+'native-source-compiles']=len(sources);report[prefix+'native-layouts']=layouts
configs=json.loads((out/'evaluated-configurations.json').read_text());assert len(configs)==48
for row in configs:assert row['exit']==0 and row['pairs']==28 and row['c_units']==1 and row['sources']==(413 if row['platform']=='Win32' and 'RW34' in row['configuration'] else 414),row
report['evaluated-configurations']=len(configs)
project=out/'project/reVC.vcxproj';assert getattr(m,'bones_project',m.project)(project.read_text())==project.read_text();ns={'m':'http://schemas.microsoft.com/developer/msbuild/2003'}
items=[node.get('Include') for node in ET.parse(project).findall('.//m:ClCompile',ns) if node.get('Include')];assert len(items)==414 and len(set(items))==414
for item in items:
    # Prove the full build used the coherent staged tree before comparing its exact applied source bytes
    path=(project.parent/item.replace('\\','/')).resolve();assert path.is_relative_to((out/'overlay/src').resolve()) and path.exists(),path
for configuration in ['debug','release']:
    # Inspect complete successful build logs and actual linked executables without launching a game window
    build=json.loads((out/f'd3d9-{configuration}.json').read_text());assert build['exit']==0;log=Path(build['log']).read_text(errors='replace');assert 'Build succeeded.' in log and '0 Error(s)' in log and 'Bones.c' in log
    exe=Path(build['exe']);assert exe.is_file() and exe.stat().st_size==build['size'];report['d3d9-'+configuration]=build['size']
history=json.loads((out/('production-history.json' if production else 'historical-source-projections.json')).read_text());assert len(history)==102 and all(row['exit']==0 for row in history);report['historical-source-projections']=len(history)
if production:
    # Require the exact production source/header tree to match the complete sources that passed both native links
    for path in (out/'overlay/src').rglob('*'):
        if path.is_file():verify(root,'src/'+path.relative_to(out/'overlay/src').as_posix(),hashlib.sha256(path.read_bytes()).hexdigest())
report.update({'status':'applied-verified' if production else 'ready-to-apply','complete_operations':2,'shared_enums':1,'native_algorithm_bridges':0,'new_headers':1,'new_producers':0,'renamed_producers':m.RENAMES,'changed_sources':len(changes)})
(out/('verification-summary.json' if production else 'stage-summary.json')).write_text(json.dumps(report,indent=2),newline='\n')
print('Verified complete C bone mapping and shared ped-node constants:',report['status'])
