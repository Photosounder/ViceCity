"""Audit complete C helper ownership and distinguish compiler evidence from pending behavioral verification."""
from pathlib import Path
import hashlib,json,re,sys,xml.etree.ElementTree as ET
import rw_helper_migration as m
from stats_successor import verify,current_path
from rw_helper_schema import HOST,LIB_HOST
from rw_helper_extension import EXTRA
from mblur_schema import SETTERS
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/rw-helper-c-tests';saved=out/'before';production='--production' in sys.argv;tree=root if production else out/'overlay';report=json.loads((out/'stage-summary.json').read_text());changes=json.loads((out/'changes.json').read_text());assert len(changes)==23
for name,digest in json.loads((out/'baseline-source-digests.json').read_text()).items():
    # Protect every whole production source against accidental application or unrelated mutation
    if not production:assert hashlib.sha256((root/name).read_bytes()).hexdigest()==digest,name
    original=(saved/name).read_text();target=current_path(root,name) if production else tree/m.RENAMES.get(name,name);candidate=target.read_text();assert candidate==m.calls(original) and m.calls(candidate)==candidate,name
    if name in changes:assert b'\r' not in target.read_bytes(),name
kernel=(tree/'src/rw/RwHelper.c').read_text();native=(tree/'src/rw/RwHelper_native.cpp').read_text();assert not (tree/'src/rw/RwHelper.cpp').exists()
assert kernel==m.calls(m.producer((saved/'src/rw/RwHelper.cpp').read_text()))
assert native==m.calls(m.native((saved/'src/rw/RwHelper.cpp').read_text()))
assert (tree/'src/rw/RwHelper_backend.h').read_text()==m.calls(m.backend_header((saved/'src/rw/RwHelper.cpp').read_text()))
assert (tree/'src/rw/RwHelper.h').read_text()==m.calls(m.header((saved/'src/rw/RwHelper.h').read_text()))
assert not re.search(r'::|\b(?:class|new|delete|namespace|CVector|RwMatrix|RwIm2DVertex|RwV3d)\b',m.mask(kernel))
for name in report['operations']:
    # Require a complete C definition for every original operation including all traversal callbacks
    current='RwHelper_'+name if name in ['ConvertPlatformAtomic','GetFirstChild'] else name
    assert len(re.findall(r'(?m)^(?:static\s+)?(?:void|bool|int32_t)\s*\*?\s*'+current+r'\([^;{}]*\)\s*\{',m.mask(kernel)))==1,name
assert not re.search(r'\b(?:for|while)\s*\(',m.mask(native))
assert 'RwHelper_HostStoreCameraRaster(camera,nil);' in kernel and 'RwHelper_HostStoreCameraZRaster(camera,nil);' in kernel
assert 'RwHelper_HostToWide(name,tmp,256*sizeof(uint16_t));' in kernel
assert 'RwHelper_HostStorePlatform(driver);' in kernel and 'RwHelper_HostSetPipeline(atomic,origPipe);' in kernel
for name,count in [('strict-compiles',40),('native-owner-compiles',18),('native-consumer-compiles',42)]:
    # Require complete compiler rows rather than reusing the earlier primitive-only summary
    rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==count and all(row['exit']==0 for row in rows),name;report[name]=count
report.update(status='staged-compiled',production_changed=production,changed_sources=23,complete_C_operations=38,native_algorithm_bridges=0,typed_sdk_callback_forwarders=1)
report['individual_native_primitives_described']=len(HOST)+len(LIB_HOST)
report['external_native_primitives']=len(HOST)+len(LIB_HOST)+len(EXTRA)+len(SETTERS)+1
report.pop('native_boundary_compiles',None);report.pop('strict_boundary_compiles',None)
report['remaining']=['Whole original/C runtime comparisons and ABI layouts','Historical projections and complete Debug/Release links','Production application and predecessor audits']
profiles=['d3d9','gl','neutral','master','final','classic','legacy','legacy-classic']
hashes={
    'Od':['7340fb893dd5c274','bdf5a9d9e5263110','531e4dabb92b5e44','a2404e1c9242106d','ad41eb7ff77edf87','09a85bca9e0866f0','0797dc7b0ddad2d7','8a0ec02e6d593e75'],
    '32-O2':['2ad5f699f5a84d8c','8cf01d4d8dd832d8','61b87024fbe25ddc','3db90e625fdbb64d','d918b9e109f18c97','24cb7894bc713d40','336ae06e30a06167','07e930a8d43a1f5d'],
    '64-O2':['468cd86f764331cc','739ee5a370d7b098','5f6f458a26835e1c','e9448a5fa16955cd','6db5fb58bb4d3697','45a0fa05ae10ca80','2262de13564117a7','abf4056e946c2a1d'],
}
rows=json.loads((out/'native-runs.json').read_text());assert len(rows)==32
assert {(r['bits'],r['optimization'],r['profile']) for r in rows}=={(a,b,c) for a in [32,64] for b in ['Od','O2'] for c in profiles}
for row in rows:
    # Pin complete original traces separately for each compiler ABI and optimization mode
    profile=row['profile'];index=profiles.index(profile);classic=profile=='legacy-classic';legacy=profile.startswith('legacy');fonts=profile not in ['master','final'];commands=135 if classic else 148 if legacy else 406;checks=dict(zip(profiles,[1928,1916,1912,1905,1905,1916,1293,923]))[profile];events=dict(zip(profiles,[1723,1707,1699,1687,1688,1705,1097,886]))[profile]
    prefix=f'commands={commands} checks={checks} events={events} cameras={13 if classic else 17}/{13 if classic else 17} frames={11 if classic else 15}/{11 if classic else 15} rasters={23 if classic else 49}/{23 if classic else 49} fonts={3 if fonts else 0}/{3 if fonts else 0} animations=3 draws=3 transforms=34 lines=10 failures={2 if classic else 6} reentries={3 if legacy else 4} mutations={0 if classic else 1} assertions=0'
    key='Od' if row['optimization']=='Od' else str(row['bits'])+'-O2';expected=prefix+' hash='+hashes[key][index];assert row['exit']==0 and row['before']==row['after']==expected,row
report['native-runs']=32
rows=json.loads((out/'native-layouts.json').read_text());assert len(rows)==2
for row in rows:
    # Check every field of the C records and SDK-derived borrowed reference view
    reference='RwHelperRectRef 16 4 x=0 y=4 w=8 h=12\n' if row['bits']==32 else 'RwHelperRectRef 32 8 x=0 y=8 w=16 h=24\n'
    expected='RwHelperRect 16 4 x=0 y=4 w=8 h=12\nRwHelperColour 4 1 red=0 green=1 blue=2 alpha=3\n'+reference+'RwHelperVector2 8 4 x=0 y=4\nRwHelperVideoMode 12 4 width=0 height=4 flags=8\n';assert row['exit']==0 and row['consumers']==['original-sdk','new-cpp','new-c'] and row['layout']==expected,row
report['native-layouts']=2;report['status']='staged-verified';report['remaining']=['Complete integration, production application and predecessor audits']
if '--integration' in sys.argv:
    # Require actual C11 metadata and unique object names in all evaluated native project configurations
    rows=json.loads((out/'evaluated-configurations.json').read_text());assert len(rows)==48
    for row in rows:assert row['exit']==0 and row['pairs']==28 and row['c_units']==1 and row['sources']==(430 if row['platform']=='Win32' and 'RW34' in row['configuration'] else 431),row
    project=out/'project/reVC.vcxproj';assert getattr(m,'rw_helper_project',m.project)(project.read_text())==project.read_text();ns={'m':'http://schemas.microsoft.com/developer/msbuild/2003'};items=[n.get('Include') for n in ET.parse(project).findall('.//m:ClCompile',ns) if n.get('Include')];assert len(items)==len(set(items))==431
    for item in items:
        # Inspect one coherent candidate rather than a mixture of production sources and staged headers
        path=(project.parent/item.replace('\\','/')).resolve();assert path.is_relative_to((out/'overlay/src').resolve()) and path.exists(),path
    for configuration in ['debug','release']:
        # Inspect the actual compilation tracking records and complete linked executables without launching them
        build=json.loads((out/f'd3d9-{configuration}.json').read_text());log=Path(build['log']).read_text(errors='replace');assert build['exit']==0 and 'Build succeeded.' in log and '0 Error(s)' in log;exe=Path(build['exe']);assert exe.is_file() and exe.stat().st_size==build['size'];objdir=out/'project/obj/win-amd64-librw_d3d9-oal'/configuration.capitalize();commands=(objdir/'reVC.tlog/CL.command.1.tlog').read_text(encoding='utf-16').splitlines()
        for name in ['RwHelper.c','RwHelper_native.cpp']:
            # Require the complete C owner and its typed SDK boundary in each actual executable link
            source=(out/'overlay/src/rw'/name).resolve();indices=[i for i,v in enumerate(commands) if v.startswith('^') and str(source).upper() in v];assert len(indices)==1,(name,indices);command=commands[indices[0]+1];assert str(source).upper() in command
            if name.endswith('.c'):assert '/TC ' in command and '/std:c11 ' in command
            obj=objdir/(source.stem+'.obj');assert obj.exists() and obj.stat().st_mtime>=source.stat().st_mtime and exe.stat().st_mtime>=obj.stat().st_mtime
        report['d3d9-'+configuration]=build['size']
    rows=json.loads((out/'historical-source-projections.json').read_text());assert len(rows)==1104 and all(r['exit']==0 for r in rows);report['history']=1104;report['status']='ready-to-apply';report['remaining']=['Production application and predecessor audits']
    if production:
        # Require production to contain every exact byte used in both complete candidate links
        for path in (out/'overlay/src').rglob('*'):
            if path.is_file():verify(root,'src/'+path.relative_to(out/'overlay/src').as_posix(),hashlib.sha256(path.read_bytes()).hexdigest())
        for name,count in [('production-owner-compiles',18),('production-consumer-compiles',42),('production-history',1104),('preceding-owner-audits',3)]:
            # Require complete production and predecessor audits before recording this owner's verification
            rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==count and all(row['exit']==0 for row in rows),name;report[name]=count
        report['status']='applied-verified';report['remaining']=[]
(out/('verification-summary.json' if production else 'stage-summary.json')).write_text(json.dumps(report,indent=2),newline='\n');print(json.dumps({key:value for key,value in report.items() if key!='operations'},indent=2))
