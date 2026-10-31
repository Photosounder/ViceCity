"""Audit complete C lighting ownership, native SDK storage and full integration."""
from pathlib import Path
import hashlib,json,re,sys,xml.etree.ElementTree as ET
import lights_migration as m
from rect_migration import mask
from stats_successor import verify

root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/lights-c-tests';saved=out/'before';production='--production' in sys.argv;tree=root if production else out/'overlay'
changes=json.loads((out/'changes.json').read_text());assert len(changes)==15
for name in changes:
    # Preserve the exact original production boundary until all staged checks pass
    if not production:assert (root/name).read_bytes()==(saved/name).read_bytes(),name
    path=tree/m.RENAMES.get(name,name);value=path.read_text()
    assert value==m.calls((saved/name).read_text()) and m.calls(value)==value and b'\r' not in path.read_bytes(),name
for name,digest in json.loads((out/'untouched.json').read_text()).items():
    # Protect every source outside the complete owner and client boundary
    if production:verify(root,name,digest)
    else:assert hashlib.sha256((root/name).read_bytes()).hexdigest()==digest,name
kernel=(tree/'src/rw/Lights.c').read_text();native=(tree/'src/rw/Lights_native.cpp').read_text()
assert kernel==m.kernel((saved/'src/rw/Lights.cpp').read_text()) and native==m.calls(m.native())
assert (tree/'src/rw/Lights.h').read_text()==m.header() and (tree/'src/rw/Lights_backend.h').read_text()==m.backend_header()
assert not (tree/'src/rw/Lights.cpp').exists()
for value in [kernel,m.header(),m.backend_header()]:
    # Require genuine C policy with no SDK record or C++ vector/class dependencies
    assert not re.search(r'::|\b(?:class|new|delete|namespace|static_cast|CVector|RwMatrix|RwRGBAReal|RpWorld|RpLight|RwFrame)\b',mask(value))
for name in [*m.NAMES.values(),'Lights_SetAmbientColour']:
    # Require all 19 entire original operations in the C producer
    assert len(re.findall(r'(?m)^'+name+r'\([^\n]*\)\n\{',kernel))==1,name
assert 'int LightStrengths[4];' in kernel and 'weakest = strength;' in kernel and 'LightStrengths[n] = strength;' in kernel
assert 'Lights_Normalise(&vec2);' in kernel and 'else value->x=1.0f;' in kernel
assert 'LightsMatrix mat;' in kernel and 'memcpy(&native,matrix,sizeof(native));' in native
assert 'const LightsVectorOut value={&at->x,&at->y,&at->z};' in native
report={'status':'staged-verified','production_changed':production,'changed_sources':15,'complete_C_operations':19,'external_native_primitives':len(m.HOST)+5,'native_algorithm_bridges':0}
for name,count in [('strict-compiles',24),('native-owner-compiles',18),('native-consumer-compiles',30),('native-runs',16),('native-layouts',2)]:
    # Require real C and complete native owner/client compilation on both Windows ABIs
    rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==count and all(row['exit']==0 for row in rows),name
    if name=='native-runs':
        assert {(r['bits'],r['optimization'],r['profile']) for r in rows}=={(a,b,c) for a in [32,64] for b in ['Od','O2'] for c in ['fixed','classic','old-filter','classic-old-filter']}
        for row in rows:
            # Pin the whole original trace as well as each pairwise C comparison
            profile=row['profile'];old='old-filter' in profile;gold={'fixed':'9ed4a59ca19d142d','classic':'57e2740e6ba162db','old-filter':'b8cf4970166fc24d','classic-old-filter':'a2c1a17b9b8e7ebb'}[profile]
            expected=f"commands=23979 checks=71211 events={56688 if old else 59760} lights=144/144 frames=120/120 colours=29616 transforms=1536 blur={0 if old else 3072} hash="+gold
            assert row['before']==row['after']==expected,row
    if name=='native-layouts':
        assert {r['bits'] for r in rows}=={32,64}
        expected='LightsColour 16 4 red=0 green=4 blue=8 alpha=12\nLightsVector 12 4 x=0 y=4 z=8\nLightsMatrix 64 4 right=0 flags=12 up=16 pad1=28 at=32 pad2=44 pos=48 pad3=60\n'
        for row in rows:assert row['consumers']==['original-cpp','new-cpp','new-c'] and row['layout']==expected,row
    report[name]=count
report['remaining']=['Full Debug/Release integration and historical source projections','Production application and checks']
if '--integration' in sys.argv:
    # Inspect every evaluated project configuration and both complete executable links
    rows=json.loads((out/'evaluated-configurations.json').read_text());assert len(rows)==48
    for row in rows:assert row['exit']==0 and row['pairs']==28 and row['c_units']==1 and row['sources']==(426 if row['platform']=='Win32' and 'RW34' in row['configuration'] else 427),row
    project=out/'project/reVC.vcxproj';assert getattr(m,'lights_project',m.project)(project.read_text())==project.read_text()
    ns={'m':'http://schemas.microsoft.com/developer/msbuild/2003'};items=[node.get('Include') for node in ET.parse(project).findall('.//m:ClCompile',ns) if node.get('Include')]
    assert len(items)==len(set(items))==427
    for item in items:
        # Require all compiled sources to come from the same complete staged tree
        path=(project.parent/item.replace('\\','/')).resolve();assert path.is_relative_to((out/'overlay/src').resolve()) and path.exists(),path
    for configuration in ['debug','release']:
        # Inspect actual C11 compiler tracking records and the linked object timestamps
        build=json.loads((out/f'd3d9-{configuration}.json').read_text());log=Path(build['log']).read_text(errors='replace');assert build['exit']==0 and 'Build succeeded.' in log and '0 Error(s)' in log
        exe=Path(build['exe']);assert exe.is_file() and exe.stat().st_size==build['size'];objdir=out/'project/obj/win-amd64-librw_d3d9-oal'/configuration.capitalize();commands=(objdir/'reVC.tlog/CL.command.1.tlog').read_text(encoding='utf-16').splitlines()
        for name in ['Lights.c','Lights_native.cpp']:
            # Require both complete lighting units to be present in the actual linked build
            source=(out/'overlay/src/rw'/name).resolve();index=commands.index('^'+str(source).upper());command=commands[index+1];assert str(source).upper() in command
            if name.endswith('.c'):assert '/TC ' in command and '/std:c11 ' in command
            obj=objdir/(source.stem+'.obj');assert obj.exists() and obj.stat().st_mtime>=source.stat().st_mtime and exe.stat().st_mtime>=obj.stat().st_mtime
        report['d3d9-'+configuration]=build['size']
    history=json.loads((out/'historical-source-projections.json').read_text());assert len(history)==660 and all(r['exit']==0 for r in history);report['history']=len(history)
    report['status']='ready-to-apply';report['remaining']=['Production application and checks']
    if production:
        # Require the applied whole project to match the exact candidate that passed both links
        for path in (out/'overlay/src').rglob('*'):
            if path.is_file():verify(root,'src/'+path.relative_to(out/'overlay/src').as_posix(),hashlib.sha256(path.read_bytes()).hexdigest())
        for name,count in [('production-consumer-compiles',30),('production-owner-compiles',18),('production-history',660),('preceding-owner-audits',3)]:
            # Require full production and predecessor verification rather than trusting prior status summaries
            rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==count and all(r['exit']==0 for r in rows),name;report[name]=count
        report['status']='applied-verified';report['remaining']=[]
(out/('verification-summary.json' if production else 'stage-summary.json')).write_text(json.dumps(report,indent=2),newline='\n')
print(json.dumps(report,indent=2))
