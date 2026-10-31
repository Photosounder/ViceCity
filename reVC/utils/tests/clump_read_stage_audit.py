"""Audit complete C clump loading, live SDK state, runtime comparisons and full integration."""
from pathlib import Path
import hashlib,json,re,sys,xml.etree.ElementTree as ET
import clump_read_migration as m
from rect_migration import mask
from stats_successor import verify

root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/clump-read-c-tests';saved=out/'before';production='--production' in sys.argv;tree=root if production else out/'overlay'
changes=json.loads((out/'changes.json').read_text());assert len(changes)==4
for name in changes:
    # Protect the original production owner until runtime and whole-project integration are complete
    if not production:assert (root/name).read_bytes()==(saved/name).read_bytes(),name
    path=tree/m.RENAMES.get(name,name);value=path.read_text()
    assert value==m.calls((saved/name).read_text()) and m.calls(value)==value and b'\r' not in path.read_bytes(),name
for name,digest in json.loads((out/'untouched.json').read_text()).items():
    # Preserve every source outside the exact complete clump owner boundary
    if production:verify(root,name,digest)
    else:assert hashlib.sha256((root/name).read_bytes()).hexdigest()==digest,name
kernel=(tree/'src/rw/ClumpRead.c').read_text();native=(tree/'src/rw/ClumpRead_native.cpp').read_text()
assert kernel==m.kernel((saved/'src/rw/ClumpRead.cpp').read_text())
assert native==m.native() and (tree/'src/rw/ClumpRead.h').read_text()==m.header()
assert (tree/'src/rw/ClumpRead_backend.h').read_text()==m.backend_header()
assert not (tree/'src/rw/ClumpRead.cpp').exists()
for value in [kernel,m.header(),m.backend_header()]:
    # Require genuine C ownership without native SDK class or record declarations leaking into policy
    assert not re.search(r'::|\b(?:class|new|delete|namespace|static_cast|RpGeometry|RpAtomic|RpClump|RwStream|rwFrameList)\b',mask(value))
for name in m.NAMES.values():
    # Require every complete original operation exactly once in the C producer
    assert len(re.findall(r'(?m)^ClumpRead_'+name+r'\([^\n]*\)\n\{',kernel))==1,name
assert 'static rwFrameList nativeFrames;' in native and 'return &nativeFrames;' in native
assert 'rwFrameListStreamRead((RwStream*)stream,(rwFrameList*)frames)' in native
assert 'rwFrameListDeinitialize((rwFrameList*)frames)' in native
assert 'ClumpRead_HostSetFrameCount(ClumpRead_HostFrames(),0)' in kernel
assert 'static int32_t numberGeometrys;' in kernel and 'static int32_t streamPosition;' in kernel
assert 'numberGeometrys = 0;' in kernel[kernel.index('ClumpRead_AtomicStreamRead('):]
for path in (tree/'src').rglob('*.cpp'):
    # Require native clients to use the C owner rather than removed C++ helper entry points
    assert not re.search(r'\bRpClumpGta(?:StreamRead[12]|CancelStream)\s*\(',mask(path.read_text())),path
report={'status':'staged-verified','production_changed':production,'changed_sources':4,'complete_C_operations':7,'external_native_primitives':len(m.HOST)+1,'native_SDK_frame_list_objects':1,'native_algorithm_bridges':0}
for name,count in [('strict-compiles',24),('native-owner-compiles',12),('native-consumer-compiles',8),('native-runs',12),('native-layouts',2)]:
    # Require complete original/C compilation and real SDK-backed record layouts on both native ABIs
    rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==count and all(row['exit']==0 for row in rows),name
    if name=='native-layouts':
        assert {row['bits'] for row in rows}=={32,64}
        for row in rows:
            assert row['consumers']==['original-cpp','new-cpp','new-c']
            expected=('ClumpReadGeometryList 8 4 geometries=0 numGeoms=4\n' if row['bits']==32 else 'ClumpReadGeometryList 16 8 geometries=0 numGeoms=8\n')+'ClumpReadAtomicBinary 16 4 frameIndex=0 geomIndex=4 flags=8 unused=12\nClumpReadChunkInfo 12 4 numAtomics=0 numLights=4 numCameras=8\n'
            assert row['layout']==expected,row
    if name=='native-runs':
        assert {(r['bits'],r['optimization'],r['profile']) for r in rows}=={(a,b,c) for a in [32,64] for b in ['Od','O2'] for c in ['librw','legacy','master']}
        for row in rows:
            legacy=row['profile']=='legacy';events=1745 if legacy else 1818
            gold=({'librw':'4a0f1a9152122009','legacy':'bb3f6f0c0b9d8080','master':'4a0f1a9152122009'} if row['bits']==32 else {'librw':'93e67ab742e6fd71','legacy':'2926c83e5722a680','master':'93e67ab742e6fd71'})[row['profile']]
            expected=f'commands=200 checks=2521 events={events} geometries=123 atomics=49 clumps=20 frames=126 retained=72/0 reentries=1 allocations=96 frees=96 failures=2 hash='+gold
            assert row['before']==row['after']==expected,row
    report[name]=count
report['remaining']=['Full Debug/Release integration and historical source projections','Production application and checks']
if '--integration' in sys.argv:
    # Require complete evaluated source metadata and actual compiler records for both executable builds
    rows=json.loads((out/'evaluated-configurations.json').read_text());assert len(rows)==48
    for row in rows:
        assert row['exit']==0 and row['pairs']==28 and row['c_units']==1,row
        assert row['sources']==(425 if row['platform']=='Win32' and 'RW34' in row['configuration'] else 426),row
    project=out/'project/reVC.vcxproj';assert getattr(m,'clump_read_project',m.project)(project.read_text())==project.read_text()
    ns={'m':'http://schemas.microsoft.com/developer/msbuild/2003'}
    items=[node.get('Include') for node in ET.parse(project).findall('.//m:ClCompile',ns) if node.get('Include')]
    assert len(items)==len(set(items))==426
    for item in items:
        path=(project.parent/item.replace('\\','/')).resolve()
        assert path.is_relative_to((out/'overlay/src').resolve()) and path.exists(),path
    for configuration in ['debug','release']:
        build=json.loads((out/f'd3d9-{configuration}.json').read_text());log=Path(build['log']).read_text(errors='replace')
        assert build['exit']==0 and 'Build succeeded.' in log and '0 Error(s)' in log
        exe=Path(build['exe']);assert exe.is_file() and exe.stat().st_size==build['size']
        objdir=out/'project/obj/win-amd64-librw_d3d9-oal'/configuration.capitalize()
        commands=(objdir/'reVC.tlog/CL.command.1.tlog').read_text(encoding='utf-16').splitlines()
        for name in ['ClumpRead.c','ClumpRead_native.cpp']:
            # Inspect actual C11 compiler tracking records and the linked object timestamps
            source=(out/'overlay/src/rw'/name).resolve();index=commands.index('^'+str(source).upper());command=commands[index+1]
            assert str(source).upper() in command
            if name.endswith('.c'):assert '/TC ' in command and '/std:c11 ' in command
            obj=objdir/(source.stem+'.obj');assert obj.exists() and obj.stat().st_mtime>=source.stat().st_mtime
            assert exe.stat().st_mtime>=obj.stat().st_mtime
        report['d3d9-'+configuration]=build['size']
    history=json.loads((out/'historical-source-projections.json').read_text())
    assert len(history)==172 and all(row['exit']==0 for row in history)
    report['history']=len(history);report['status']='ready-to-apply';report['remaining']=['Production application and checks']
    if production:
        # Require every applied source byte to match the complete candidate used for the two links
        for path in (out/'overlay/src').rglob('*'):
            if path.is_file():verify(root,'src/'+path.relative_to(out/'overlay/src').as_posix(),hashlib.sha256(path.read_bytes()).hexdigest())
        for name,count in [('production-consumer-compiles',8),('production-owner-compiles',12),('production-history',172),('preceding-owner-audits',3)]:
            rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==count and all(row['exit']==0 for row in rows),name
            report[name]=count
        report['status']='applied-verified';report['remaining']=[]
(out/('verification-summary.json' if production else 'stage-summary.json')).write_text(json.dumps(report,indent=2),newline='\n')
print(json.dumps(report,indent=2))
