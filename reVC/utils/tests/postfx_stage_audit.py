"""Require complete C PostFX ownership and reproducible producer, ABI, history and link evidence."""
from pathlib import Path
import hashlib,json,re,sys,xml.etree.ElementTree as ET
import postfx_migration as m
from rect_migration import mask
from stats_successor import verify
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/postfx-c-tests';saved=out/'before';production='--production' in sys.argv;tree=root if production else out/'overlay';changes=json.loads((out/'changes.json').read_text());assert len(changes)==9
for name in changes:
    # Preserve original production bytes until the entire staged candidate passes
    if not production:assert (root/name).read_bytes()==(saved/name).read_bytes(),name
    path=tree/m.RENAMES.get(name,name);value=path.read_text();assert value==m.calls((saved/name).read_text()) and m.calls(value)==value and b'\r' not in path.read_bytes(),name
for name,digest in json.loads((out/'untouched.json').read_text()).items():
    # Protect complete source files outside this owner and its explicit native clients
    if production:verify(root,name,digest)
    else:assert hashlib.sha256((root/name).read_bytes()).hexdigest()==digest,name
kernel=(tree/'src/extras/postfx.c').read_text();native=(tree/'src/extras/PostFX_native.cpp').read_text();bindings=(tree/'src/extras/PostFX_bindings.c').read_text();assert kernel==m.kernel((saved/'src/extras/postfx.cpp').read_text()) and native==m.native() and bindings==m.bindings();assert not (tree/'src/extras/postfx.cpp').exists()
assert (tree/'src/extras/postfx.h').read_text()==m.header();assert (tree/'src/extras/PostFX_backend.h').read_text()==m.backend_header()
for value in [kernel,m.header(),m.backend_header(),bindings]:
    # Require actual C policy without the removed C++ class or native SDK objects
    assert not re.search(r'::|\b(?:class|new|delete|namespace|static_cast|RwRaster|RwIm2DVertex|CPostFX)\b',mask(value))
for name in m.OPERATIONS:
    # Require all complete original application operations rather than algorithm forwarding bridges
    assert len(re.findall(r'(?m)^(?:void|bool)\s*\n?PostFX_'+name+r'\([^\n]*\)\n\{',kernel))==1,name
assert not re.search(r'\b(?:for|while)\s*\(',mask(native))
assert len(re.findall(r'(?m)^(?:bool|void) (?:Lights_HostBlurColours|MBlur_HostPost(?:Open|Close|Render))\(',bindings))==4
for name in ['src/rw/Lights_native.cpp','src/renderer/MBlur_native.cpp']:
    # Require the former C++ forwarding definitions to move entirely into the C binding unit
    assert not re.search(r'(?m)^(?:bool|void) (?:Lights_HostBlurColours|MBlur_HostPost(?:Open|Close|Render))\([^;\n]*\)\n\{',(tree/name).read_text())
report={'status':'staged-verified','production_changed':production,'changed_sources':9,'complete_C_operations':13,'C_forwarding_operations':4,'classes_removed':1,'native_algorithm_bridges':0}
expected={
    'd3d9':'commands=5346 checks=11671 events=54605 rasters=80/80 shaders=81/81 failures=2/1 draws=3986 constants=988 uniforms=0 registrations=0 reentries=1 assertions=0 hash=570603fc395d8156',
    'gl':'commands=5346 checks=12529 events=55874 rasters=80/80 shaders=81/81 failures=2/1 draws=3986 constants=0 uniforms=988 registrations=6 reentries=1 assertions=1 hash=7f899e52784b2b95',
    'neutral':'commands=5344 checks=10433 events=53450 rasters=80/80 shaders=0/0 failures=2/0 draws=3984 constants=0 uniforms=0 registrations=0 reentries=0 assertions=0 hash=5dcd5272851bbd79',
    'master':'commands=5346 checks=10519 events=52301 rasters=80/80 shaders=81/81 failures=2/1 draws=3986 constants=988 uniforms=0 registrations=0 reentries=1 assertions=0 hash=a7b0527a5b055586',
    'final':'commands=5346 checks=10519 events=52301 rasters=80/80 shaders=81/81 failures=2/1 draws=3986 constants=988 uniforms=0 registrations=0 reentries=1 assertions=0 hash=a7b0527a5b055586',
    'inactive':'commands=2 checks=3 events=0 rasters=0/0 shaders=0/0 failures=0/0 draws=0 constants=0 uniforms=0 registrations=0 reentries=0 assertions=0 hash=1e2e92958ce1f090',
}
for name,count in [('strict-compiles',64),('native-owner-compiles',24),('native-consumer-compiles',20),('native-runs',24),('native-layouts',2)]:
    # Inspect complete individual evidence rows rather than accepting summary status flags
    rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==count and all(r['exit']==0 for r in rows),name
    if name=='native-runs':
        assert {(r['bits'],r['optimization'],r['profile']) for r in rows}=={(a,b,c) for a in [32,64] for b in ['Od','O2'] for c in expected}
        for row in rows:assert row['before']==row['after']==expected[row['profile']],row
    if name=='native-layouts':
        for row in rows:
            # Pin every public scalar, smoothing-array extent and original enum constant on both ABIs
            pointer=4 if row['bits']==32 else 8;value=''
            for field in m.VARIABLES:
                size,align=(pointer,pointer) if field.startswith('p') else (1,1) if m.VARIABLES[field]=='bool' else (80,4) if field.startswith('Prev') else (4,4);value+=f'{field} {size} {align}\n'
            assert row['consumers']==['original-sdk','new-cpp','new-c'] and row['layout']==value+'enum 0 1 2 3 20\n',row
    report[name]=count
limits=json.loads((out/'strict-existing-limitations.json').read_text());assert len(limits)==4 and all(r['exit']!=0 for r in limits);report['original_configuration_guards']=4;report['remaining']=['Full integration and production application']
if '--integration' in sys.argv:
    # Require C11 metadata and unique objects across every evaluated configuration
    rows=json.loads((out/'evaluated-configurations.json').read_text());assert len(rows)==48
    for row in rows:assert row['exit']==0 and row['pairs']==28 and row['c_units']==2 and row['sources']==(429 if row['platform']=='Win32' and 'RW34' in row['configuration'] else 430),row
    project=out/'project/reVC.vcxproj';assert getattr(m,'postfx_project',m.project)(project.read_text())==project.read_text();ns={'m':'http://schemas.microsoft.com/developer/msbuild/2003'};items=[n.get('Include') for n in ET.parse(project).findall('.//m:ClCompile',ns) if n.get('Include')];assert len(items)==len(set(items))==430
    for item in items:
        # Require one coherent complete staged tree for the actual executable build
        path=(project.parent/item.replace('\\','/')).resolve();assert path.is_relative_to((out/'overlay/src').resolve()) and path.exists(),path
    for configuration in ['debug','release']:
        # Require actual C producer and native primitive objects in complete executable links without launching them
        build=json.loads((out/f'd3d9-{configuration}.json').read_text());log=Path(build['log']).read_text(errors='replace');assert build['exit']==0 and 'Build succeeded.' in log and '0 Error(s)' in log;exe=Path(build['exe']);assert exe.is_file() and exe.stat().st_size==build['size'];objdir=out/'project/obj/win-amd64-librw_d3d9-oal'/configuration.capitalize();commands=(objdir/'reVC.tlog/CL.command.1.tlog').read_text(encoding='utf-16').splitlines()
        for name in ['postfx.c','PostFX_bindings.c','PostFX_native.cpp']:
            # Find each actual compilation group even when MSBuild batches the two C translation units
            source=(out/'overlay/src/extras'/name).resolve();indices=[i for i,v in enumerate(commands) if v.startswith('^') and str(source).upper() in v];assert len(indices)==1,(name,indices);command=commands[indices[0]+1];assert str(source).upper() in command
            if name.endswith('.c'):assert '/TC ' in command and '/std:c11 ' in command
            obj=objdir/(source.stem+'.obj');assert obj.exists() and obj.stat().st_mtime>=source.stat().st_mtime and exe.stat().st_mtime>=obj.stat().st_mtime
        report['d3d9-'+configuration]=build['size']
    rows=json.loads((out/'historical-source-projections.json').read_text());assert len(rows)==423 and all(r['exit']==0 for r in rows);report['history']=423;report['status']='ready-to-apply';report['remaining']=['Production application and checks']
    if production:
        # Require every applied source byte to equal the exact candidate used in both complete links
        for path in (out/'overlay/src').rglob('*'):
            if path.is_file():verify(root,'src/'+path.relative_to(out/'overlay/src').as_posix(),hashlib.sha256(path.read_bytes()).hexdigest())
        for name,count in [('production-owner-compiles',24),('production-consumer-compiles',20),('production-history',423),('preceding-owner-audits',3)]:
            # Require full production and predecessor checks before recording completion of this owner
            rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==count and all(r['exit']==0 for r in rows),name;report[name]=count
        report['status']='applied-verified';report['remaining']=[]
(out/('verification-summary.json' if production else 'stage-summary.json')).write_text(json.dumps(report,indent=2),newline='\n');print(json.dumps(report,indent=2))
