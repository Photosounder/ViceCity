"""Audit the staged manager projection without claiming its remaining runtime coverage is complete."""
from pathlib import Path
import hashlib,json,re,sys,xml.etree.ElementTree as ET
import anim_manager_migration as m
from rect_migration import mask
from stats_successor import current_path,verify
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/anim-manager-c-tests';saved=out/'before';production='--production' in sys.argv;tree=root if production else out/'overlay'
changes=json.loads((out/'changes.json').read_text())
for name in changes:
    # Require unchanged production sources and the exact complete staged projection
    original=(saved/name).read_text()
    if not production:assert (root/name).read_bytes()==(saved/name).read_bytes(),name
    candidate=(tree/m.RENAMES.get(name,name)).read_text();assert candidate==m.calls(original),name
    assert m.calls(candidate)==candidate,name
for name,digest in json.loads((out/'untouched.json').read_text()).items():
    # Reject changes to unrelated production source files during staging
    if production:verify(root,name,digest)
    else:assert hashlib.sha256((root/name).read_bytes()).hexdigest()==digest,name
original=(saved/'src/animation/AnimManager.cpp').read_text()
for name,value in [('AnimManagerData.h',m.data_header()),('AnimManager_backend.h',m.backend_header()),('AnimManager_native.cpp',m.native(original))]:
    # Verify primitive leaves and C descriptor records against their complete deterministic projection
    assert (tree/'src/animation'/name).read_text()==value,name
candidate=(tree/'src/animation/AnimManager.c').read_text()
assert not re.search(r'\b(?:class|new|delete|static_cast|CVector|CAnimManager)\b|::',mask(candidate))
for _,name,args in m.DECLARATIONS:
    # Require exactly one implementation of each declared C operation
    assert len(re.findall(r'\bAnimManager_'+name+r'\([^;{}]*\)\s*\{',mask(candidate)))==1,name
strict=json.loads((out/'strict-c11-compiles.json').read_text());native=json.loads((out/'native-manager-compiles.json').read_text());consumers=json.loads((out/'native-consumer-compiles.json').read_text());policy=json.loads((out/'policy-runs.json').read_text())
expectedConsumers=2*(2+sum(name.endswith(('.cpp','.c')) and name!='src/animation/AnimManager.cpp' for name in changes))
assert len(strict)==16 and len(native)==12 and len(consumers)==expectedConsumers and len(policy)==4
for row in strict+native+consumers:assert row['exit']==0,row
for row in policy:
    # Restrict behavioral evidence to the cache/reference operations actually compared
    assert row['before']==row['after'] and 'operations=32768 checks=852292' in row['after'],row
runtime={}
for name,count in [('loader-runs',16),('loader-sanitized-runs',4),('loader-native-runs',8),('full-matrix-runs',16),('full-sanitized-runs',4),('full-native-runs',8)]:
    # Check complete loader and manager results against independently compared original implementations
    rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==count,(name,len(rows));runtime[name]=len(rows)
    for row in rows:
        # Require each profile's exact current behavior and real ownership counters to agree
        assert row['before']==row['after'],(name,row)
        compressed='compression' in row['profile'];result=row['after']
        if name.startswith('loader'):
            # Retain broad parser and real cache-eviction coverage alongside the full-manager scenarios
            assert 'loads=1024' in result and 'reads=69300 opens=128' in result and ('hash=b9d4c85cf4948a68' if compressed else 'hash=0605650ecf697958') in result,(name,row)
        else:
            # Include all created ped groups, blend cases, continuing assertions and balanced shutdown ownership
            assert 'blends=4096 groups=33' in result and 'instances=66 skins=33 removals=36 nodes=17720' in result and ('checks=211012' if compressed else 'checks=173267') in result and ('hash=56c38a3a648f7abb' if compressed else 'hash=38354bbdd2a1ce0d') in result,(name,row)
layouts=json.loads((out/'native-layouts.json').read_text());assert len(layouts)==6 and all(row['exit']==0 for row in layouts)
report={'status':'staged-incomplete','changed-paths':len(changes),'c-operations':len(m.DECLARATIONS),'strict-c11-compiles':len(strict),'native-manager-compiles':len(native),'native-consumer-compiles':len(consumers),'policy-runs':len(policy),**runtime,'native-layouts':len(layouts),'remaining':['Evaluated build configurations and historical source proofs','Full D3D9 Debug/Release links and production application']}
if '--integration' in sys.argv:
    # Require the full configuration and link scope before the owner conversion can be applied
    project=out/'project/reVC.vcxproj';xml=project.read_text();projection=getattr(m,'anim_manager_project',m.project);assert projection(xml)==xml
    ns={'m':'http://schemas.microsoft.com/developer/msbuild/2003'};items=[node.get('Include') for node in ET.parse(project).findall('.//m:ClCompile',ns) if node.get('Include')]
    assert len(items)==414 and len(set(items))==414
    for item in items:
        # Require every full-build producer to come from the coherent staged tree that is being applied
        path=(project.parent/item.replace('\\','/')).resolve();assert path.is_relative_to((out/'overlay/src').resolve()) and path.exists(),path
    assert sum(item.endswith('\\AnimManager.c') for item in items)==1 and sum(item.endswith('\\AnimManager_native.cpp') for item in items)==1
    configs=json.loads((out/'evaluated-configurations.json').read_text());assert len(configs)==48
    for row in configs:assert row['exit']==0 and row['pairs']==28 and row['c_units']==1 and row['sources']==(413 if row['platform']=='Win32' and 'RW34' in row['configuration'] else 414),row
    report['evaluated-configurations']=len(configs)
    history=json.loads((out/('production-history.json' if production else 'historical-source-projections.json')).read_text());assert len(history)==35*33 and all(row['exit']==0 for row in history)
    report['historical-source-projections']=len(history)
    for configuration in ['debug','release']:
        # Inspect authoritative complete build logs and actual linked executable artifacts
        build=json.loads((out/f'd3d9-{configuration}.json').read_text());assert build['exit']==0
        log=Path(build['log']).read_text(errors='replace');assert 'Build succeeded.' in log and '0 Error(s)' in log and 'AnimManager.c' in log and 'AnimManager_native.cpp' in log
        exe=Path(build['exe']);assert exe.is_file() and exe.stat().st_size==build['size'];report['d3d9-'+configuration]=build['size']
    report['status']='applied-verified' if production else 'ready-to-apply';report['remaining']=[] if production else ['Apply production sources and verify real production consumers']
    if production:
        # Prove production uses the exact sources and headers compiled in the staged full builds
        for path in (out/'overlay/src').rglob('*'):
            if path.is_file():
                # Accept only exact bytes or a complete explicitly proven later owner conversion
                name='src/'+path.relative_to(out/'overlay/src').as_posix();verify(root,name,hashlib.sha256(path.read_bytes()).hexdigest())
        assert not (root/'src/animation/AnimManager.cpp').exists()
        sources=json.loads((out/'production-source-compiles.json').read_text());assert len(sources)==70 and all(row['exit']==0 for row in sources)
        report['production-source-compiles']=len(sources)
(out/('verification-summary.json' if production else 'stage-summary.json')).write_text(json.dumps(report,indent=2),newline='\n')
print(json.dumps(report,indent=2))
