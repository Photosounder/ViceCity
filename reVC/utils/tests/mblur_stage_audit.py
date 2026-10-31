"""Audit complete C blur ownership, live scalar references, SDK scope lifetime and integration."""
from pathlib import Path
import hashlib,json,re,sys,xml.etree.ElementTree as ET
import mblur_migration as m
from rect_migration import mask
from stats_successor import verify,current_path

root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/mblur-c-tests';saved=out/'before';production='--production' in sys.argv;tree=root if production else out/'overlay';changes=json.loads((out/'changes.json').read_text());assert len(changes)==19
for name in changes:
    # Preserve the exact original owner and client boundary until all staged checks pass
    if not production:assert (root/name).read_bytes()==(saved/name).read_bytes(),name
    path=current_path(root,name) if production else tree/m.RENAMES.get(name,name);value=path.read_text();assert value==m.calls((saved/name).read_text()) and m.calls(value)==value and b'\r' not in path.read_bytes(),name
for name,digest in json.loads((out/'untouched.json').read_text()).items():
    # Protect every complete project source outside this owner and its explicit native clients
    if production:verify(root,name,digest)
    else:assert hashlib.sha256((root/name).read_bytes()).hexdigest()==digest,name
kernel=(tree/'src/renderer/MBlur.c').read_text();native=(tree/'src/renderer/MBlur_native.cpp').read_text();assert kernel==m.kernel((saved/'src/renderer/MBlur.cpp').read_text()) and native==m.calls(m.native());assert not (tree/'src/renderer/MBlur.cpp').exists()
assert (tree/'src/renderer/MBlur.h').read_text()==m.header((saved/'src/renderer/MBlur.h').read_text())
for name,value in [('MBlur_backend.h',m.backend_header()),('MBlur_native.h',m.native_header())]:assert (tree/'src/renderer'/name).read_text()==value,name
assert (tree/'src/core/Camera_effects.h').read_text()==m.constants((saved/'src/core/Camera.h').read_text())
for value in [kernel,(tree/'src/renderer/MBlur.h').read_text(),m.backend_header()]:
    # Require genuine C without native SDK record declarations or the removed static class
    assert not re.search(r'::|\b(?:class|new|delete|namespace|static_cast|RwRect|RwRGBA|RwIm2DVertex|CMBlur)\b',mask(value))
for name in m.OPERATIONS:
    # Require all 11 complete original policy operations in the C producer
    assert len(re.findall(r'(?m)^(?:void )?MBlur_'+name+r'\([^\n]*\)\n\{',kernel))==1,name
assert not re.search(r'\b(?:for|while)\s*\(',mask(native))
assert 'RwIm2DVertex vertices[4];callback(data,vertices);' in native
assert '#if defined(GTA_PS2) && !defined(EXTENDED_COLOURFILTER)\nvoid *MBlur_HostSubRaster' in native
assert 'MBlur_HostPushGroup("CMBlur::MotionBlurRender")' in kernel
assert '(*rect.x) = Max((*rect.x), 0);' in kernel and 'MBlur_CopyRect(rect)' in kernel
assert 'MBlur_CreateImmediateModeData(cam, MBlur_RectRef(&rect));\n#else' in kernel
report={'status':'staged-verified','production_changed':production,'changed_sources':19,'complete_C_operations':11,'classes_removed':1,'external_native_primitives':51,'native_scope_trampolines':1,'native_algorithm_bridges':0,'ps2_missing_return':'Original successful fresh-open fallthrough is preserved; runtime callers discard only that unspecified result while comparing defined early returns'}
prefixes={
    'extended':'commands=1803 checks=8408 events=79008 rasters=0/0 failures=0/0 draws=2440 refractions=0 textures=0 post=156 prints=0 groups=0 random=16 reentries=1 mutations=18',
    'classic':'commands=1806 checks=9402 events=86442 rasters=6/6 failures=3/0 draws=2710 refractions=0 textures=0 post=0 prints=0 groups=240 random=16 reentries=1 mutations=18',
    'software':'commands=1803 checks=4137 events=43500 rasters=6/6 failures=3/0 draws=1752 refractions=22 textures=813 post=0 prints=0 groups=240 random=8 reentries=0 mutations=18',
    'legacy':'commands=1804 checks=9386 events=86418 rasters=2/2 failures=3/0 draws=2718 refractions=0 textures=0 post=0 prints=0 groups=240 random=16 reentries=1 mutations=18',
    'classic-final':'commands=1806 checks=9282 events=86202 rasters=6/6 failures=3/0 draws=2710 refractions=0 textures=0 post=0 prints=0 groups=0 random=16 reentries=1 mutations=18',
    'unscaled':'commands=1806 checks=9402 events=76880 rasters=6/6 failures=3/0 draws=2710 refractions=0 textures=0 post=0 prints=0 groups=240 random=16 reentries=1 mutations=18',
    'ps2':'commands=1810 checks=8756 events=73181 rasters=9/9 failures=3/2 draws=2591 refractions=0 textures=0 post=0 prints=5 groups=0 random=16 reentries=1 mutations=18',
}
hashes={'Od':dict(zip(prefixes,['dfb1f2600324b8f0','b2d59c513c271171','e96004fd98f1f916','42db7b3ffbf9fb77','66f64492d1adef0d','b1ffb608bba244da','0fa4c7cab4aadb3a'])),'O2':dict(zip(prefixes,['78c266710d4f1fd0','c3d1fe24b2e6bb81','b0077dc5b8699c56','884e14a5b6868527','6072b3abb579f63d','b1ffb608bba244da','0fa4c7cab4aadb3a']))}
for name,count in [('strict-compiles',38),('native-owner-compiles',18),('native-consumer-compiles',34),('native-runs',26),('native-layouts',2)]:
    # Require whole original/C producer and independent interface checks on both actual native ABIs
    rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==count and all(r['exit']==0 for r in rows),name
    if name=='native-runs':
        assert {(r['bits'],r['optimization'],r['profile']) for r in rows}=={(a,b,c) for a in [32,64] for b in ['Od','O2'] for c in prefixes if c!='ps2' or a==32}
        for row in rows:
            # Pin each complete original SDK trace independently of the new producer, including optimization-specific float results
            expected=prefixes[row['profile']]+' hash='+hashes[row['optimization']][row['profile']];assert row['before']==row['after']==expected,row
    if name=='native-layouts':
        for row in rows:
            # Require complete SDK rectangle, colour and borrowed-pointer layouts in C and C++
            bits=row['bits'];expected='MBlurRect 16 4 x=0 y=4 w=8 h=12\nMBlurColour 4 1 red=0 green=1 blue=2 alpha=3\n'+('MBlurRectRef 16 4 x=0 y=4 w=8 h=12\n' if bits==32 else 'MBlurRectRef 32 8 x=0 y=8 w=16 h=24\n');assert row['consumers']==['original-sdk','new-cpp','new-c'] and row['layout']==expected,row
    report[name]=count
limits=json.loads((out/'strict-existing-limitations.json').read_text());assert len(limits)==2 and all(r['bits']==64 and r['profile']=='ps2' and r['exit']!=0 for r in limits);report['original_configuration_guards']=len(limits);report['remaining']=['Full Debug/Release integration and historical source projections','Production application and checks']
if '--integration' in sys.argv:
    # Inspect actual C11 metadata and distinct object names in all evaluated MSBuild configurations
    rows=json.loads((out/'evaluated-configurations.json').read_text());assert len(rows)==48
    for row in rows:assert row['exit']==0 and row['pairs']==28 and row['c_units']==1 and row['sources']==(427 if row['platform']=='Win32' and 'RW34' in row['configuration'] else 428),row
    project=out/'project/reVC.vcxproj';assert getattr(m,'mblur_project',m.project)(project.read_text())==project.read_text();ns={'m':'http://schemas.microsoft.com/developer/msbuild/2003'};items=[n.get('Include') for n in ET.parse(project).findall('.//m:ClCompile',ns) if n.get('Include')];assert len(items)==len(set(items))==428
    for item in items:
        # Require one coherent complete candidate rather than a mixed source and header build
        path=(project.parent/item.replace('\\','/')).resolve();assert path.is_relative_to((out/'overlay/src').resolve()) and path.exists(),path
    for configuration in ['debug','release']:
        # Inspect actual compiler tracking records and linked object timestamps without launching the executable
        build=json.loads((out/f'd3d9-{configuration}.json').read_text());log=Path(build['log']).read_text(errors='replace');assert build['exit']==0 and 'Build succeeded.' in log and '0 Error(s)' in log;exe=Path(build['exe']);assert exe.is_file() and exe.stat().st_size==build['size'];objdir=out/'project/obj/win-amd64-librw_d3d9-oal'/configuration.capitalize();commands=(objdir/'reVC.tlog/CL.command.1.tlog').read_text(encoding='utf-16').splitlines()
        for name in ['MBlur.c','MBlur_native.cpp']:
            # Require the actual C policy and SDK primitive units in each complete link
            source=(out/'overlay/src/renderer'/name).resolve();index=commands.index('^'+str(source).upper());command=commands[index+1];assert str(source).upper() in command
            if name.endswith('.c'):assert '/TC ' in command and '/std:c11 ' in command
            obj=objdir/(source.stem+'.obj');assert obj.exists() and obj.stat().st_mtime>=source.stat().st_mtime and exe.stat().st_mtime>=obj.stat().st_mtime
        report['d3d9-'+configuration]=build['size']
    rows=json.loads((out/'historical-source-projections.json').read_text());assert len(rows)==874 and all(r['exit']==0 for r in rows);report['history']=len(rows);report['status']='ready-to-apply';report['remaining']=['Production application and checks']
    if production:
        # Require every applied source byte to match the exact candidate used for both complete executable links
        for path in (out/'overlay/src').rglob('*'):
            if path.is_file():verify(root,'src/'+path.relative_to(out/'overlay/src').as_posix(),hashlib.sha256(path.read_bytes()).hexdigest())
        for name,count in [('production-owner-compiles',18),('production-consumer-compiles',34),('production-history',874),('preceding-owner-audits',3)]:
            # Require full production and predecessor checks rather than trusting prior summary status
            rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==count and all(r['exit']==0 for r in rows),name;report[name]=count
        report['status']='applied-verified';report['remaining']=[]
(out/('verification-summary.json' if production else 'stage-summary.json')).write_text(json.dumps(report,indent=2),newline='\n');print(json.dumps(report,indent=2))
