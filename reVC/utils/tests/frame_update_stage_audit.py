"""Audit complete staged C frame callbacks without claiming remaining integration is finished."""
from pathlib import Path
import hashlib,json,re,sys,xml.etree.ElementTree as ET
import frame_update_migration as m
from rect_migration import mask
from stats_successor import current_path,verify
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/frame-update-c-tests';saved=out/'before';production='--production' in sys.argv;tree=root if production else out/'overlay';changes=json.loads((out/'changes.json').read_text());assert len(changes)==2
for name in changes:
    # Require unchanged production and the exact complete isolated source projection
    original=(saved/name).read_text()
    if not production:assert (root/name).read_bytes()==(saved/name).read_bytes(),name
    path=current_path(root,name) if production else tree/m.RENAMES.get(name,name);assert path.read_text()==m.calls(original) and m.calls(path.read_text())==path.read_text() and b'\r' not in path.read_bytes(),name
for name,digest in json.loads((out/'untouched.json').read_text()).items():
    # Protect all production owners outside this explicitly staged callback and API boundary
    if production:verify(root,name,digest)
    else:assert hashlib.sha256((root/name).read_bytes()).hexdigest()==digest,name
for name,value in [('AnimBlendFrameUpdate.h',m.header()),('FrameUpdate_backend.h',m.backend_header()),('FrameUpdate_native.cpp',m.native())]:
    # Keep C dispatch data and all native field primitives on the same audited version
    assert (tree/'src/animation'/name).read_text()==value,name
kernel=(tree/'src/animation/FrameUpdate.c').read_text();original=(saved/'src/animation/FrameUpdate.cpp').read_text()
assert not re.search(r'::|\b(?:class|new|delete|namespace|CVector|RwMatrix|RpHAnimStdInterpFrame)\b',mask(kernel))
assert kernel.count('++*node;')==original.count('++*node;')
for name in m.OPERATIONS:
    # Require the entire exact callback definition once, including offscreen delegation and both packed branches
    assert len(re.findall(r'(?m)^void FrameUpdateCallBack'+name+r'\(',kernel))==1,name
report={'status':'staged-incomplete','complete_C_callbacks':len(m.OPERATIONS),'changed_sources':len(changes),'new_headers':2,'new_native_units':1,'native_primitive_leaves':9,'native_algorithm_bridges':0}
for name,count in [('matrix-runs',16),('sanitized-runs',4),('native-runs',8),('mutation-matrix-runs',16),('mutation-sanitized-runs',4),('mutation-native-runs',8)]:
    # Match every compiler/configuration and native ABI to the original complete algorithms
    rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==count and all(row['exit']==0 for row in rows),name
    for row in rows:
        mutation=name.startswith('mutation-');gold=('0f8ca137f0aadcf4' if 'classic' in row['profile'] else 'faf0470ab17bca14') if mutation else ('1e5a375672fe99b3' if 'classic' in row['profile'] else 'f6aeff59d0708f9f')
        assert row['before']==row['after']=='frames=18432 checks='+('55297' if mutation else '36865')+' hash='+gold,row
    report[name]=len(rows)
for name,count in [('native-source-compiles',6),('native-layout-compiles',6)]:
    # Require actual SDK/project compilation and both original/C dispatch layouts in each native width
    rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==count and all(row['exit']==0 for row in rows),name;report[name]=len(rows)
report['remaining']=['Full integration and production application']
if '--integration' in sys.argv:
    # Require complete evaluated metadata, historical proofs and successful full D3D9 links before application
    configs=json.loads((out/'evaluated-configurations.json').read_text());assert len(configs)==48
    for row in configs:assert row['exit']==0 and row['pairs']==28 and row['c_units']==1 and row['sources']==(414 if row['platform']=='Win32' and 'RW34' in row['configuration'] else 415),row
    project=out/'project/reVC.vcxproj';xml=project.read_text();assert getattr(m,'frame_update_project',m.project)(xml)==xml;ns={'m':'http://schemas.microsoft.com/developer/msbuild/2003'}
    items=[node.get('Include') for node in ET.parse(project).findall('.//m:ClCompile',ns) if node.get('Include')];assert len(items)==415 and len(set(items))==415
    for item in items:
        # Prove every linked producer used the same complete staged source tree
        path=(project.parent/item.replace('\\','/')).resolve();assert path.is_relative_to((out/'overlay/src').resolve()) and path.exists(),path
    report['evaluated-configurations']=len(configs)
    history=json.loads((out/('production-history.json' if production else 'historical-source-projections.json')).read_text());assert len(history)==70 and all(row['exit']==0 for row in history);report['historical-source-projections']=len(history)
    for configuration in ['debug','release']:
        # Inspect full build success and linked executables without launching the game
        build=json.loads((out/f'd3d9-{configuration}.json').read_text());assert build['exit']==0;log=Path(build['log']).read_text(errors='replace');assert 'Build succeeded.' in log and '0 Error(s)' in log and 'FrameUpdate.c' in log and 'FrameUpdate_native.cpp' in log
        exe=Path(build['exe']);assert exe.is_file() and exe.stat().st_size==build['size'];report['d3d9-'+configuration]=build['size']
    report['status']='applied-verified' if production else 'ready-to-apply';report['remaining']=[] if production else ['Apply and verify real production consumers']
    if production:
        # Require production source/header bytes to equal the complete source tree used by both full builds
        for path in (out/'overlay/src').rglob('*'):
            if path.is_file():verify(root,'src/'+path.relative_to(out/'overlay/src').as_posix(),hashlib.sha256(path.read_bytes()).hexdigest())
        assert not (root/'src/animation/FrameUpdate.cpp').exists()
        rows=json.loads((out/'production-source-compiles.json').read_text());assert len(rows)==6 and all(row['exit']==0 for row in rows);report['production-source-compiles']=len(rows)
(out/('verification-summary.json' if production else 'stage-summary.json')).write_text(json.dumps(report,indent=2),newline='\n');print(json.dumps(report,indent=2))
