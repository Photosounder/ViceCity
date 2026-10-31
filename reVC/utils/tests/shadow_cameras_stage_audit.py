"""Audit complete C shadow ownership against compiler, behavior, history, and integration evidence."""
from pathlib import Path
import hashlib,json,re,sys,xml.etree.ElementTree as ET
import shadow_cameras_migration as m
import shadow_cameras_schema as schema
from rect_migration import mask
from stats_successor import verify,current_path
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/shadow-cameras-c-tests';production='--production' in sys.argv;tree=root if production else out/'overlay';report=json.loads((out/'stage-summary.json').read_text());changes=json.loads((out/'changes.json').read_text());assert len(changes)==12
for name,digest in json.loads((out/'baseline-source-digests.json').read_text()).items():
    # Verify every whole owner and consumer projection against the same immutable source baseline
    if not production:assert hashlib.sha256((root/name).read_bytes()).hexdigest()==digest,name
    original=(out/'before'/name).read_text();target=current_path(root,name) if production else tree/m.RENAMES.get(name,name);assert target.read_text()==m.calls(original) and m.calls(target.read_text())==target.read_text(),name
    if name in changes:assert b'\r' not in target.read_bytes(),name
header,native=m.boundary();assert (tree/'src/renderer/Shadow_backend.h').read_text()==header;assert (tree/'src/renderer/Shadow_native.cpp').read_text()==native
assert not re.search(r'\b(?:for|while)\s*\(',mask(native))
for kind,operations in [('ShadowCamera',schema.CAMERA_OPERATIONS),('CutsceneShadow',schema.CUTSCENE_OPERATIONS)]:
    # Require complete C definitions for every original class operation rather than C++ forwarding wrappers
    value=(tree/f'src/renderer/{kind}.c').read_text();assert not (tree/f'src/renderer/{kind}.cpp').exists();assert not re.search(r'::|\b(?:class|new|delete|namespace|RwMatrix|RwV3d|RwSphere|RwIm2DVertex)\b',mask(value))
    for name in operations:assert len(re.findall(r'(?m)^[\w *]+\b'+kind+'_'+name+r'\([^;{}]*\)\s*\{',mask(value)))==1,(kind,name)
for name,count in [('strict-partial-compiles',56),('native-partial-owner-compiles',36),('native-consumer-compiles',12),('peer-native-runs',32),('native-layouts',2),('historical-source-projections',588)]:
    # Inspect complete evidence rows and require both independent behavior outputs where applicable
    rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==count and all(row['exit']==0 for row in rows),name
    if name.endswith('runs'):assert all(row['before']==row['after'] for row in rows)
    report[name]=count
peer=json.loads((root/'.diagnostics/rw-helper-c-tests/native-runs.json').read_text());assert json.loads((out/'peer-native-runs.json').read_text())==peer
rows=json.loads((out/'native-runs.json').read_text());profiles=['d3d9','gl','software','master','final','legacy'];assert len(rows)==24 and {(r['bits'],r['optimization'],r['profile']) for r in rows}=={(a,b,c) for a in [32,64] for b in ['Od','O2'] for c in profiles}
pinned=json.loads((out/'original-runtime-traces.json').read_text())
for row in rows:
    # Pin original complete traces with all operations, defined failures, and native storage reentry covered
    key=f"{row['bits']}-{row['optimization']}-{row['profile']}";assert row['exit']==0 and row['before']==row['after']==pinned[key],row
    prefix=f"commands=429 checks={5571 if row['profile']=='software' else 5379} events={11991 if row['profile']=='software' else 11391 if row['profile']=='master' else 11413} draws=224 renders=71 reentries=2 cameras=81/81 frames=113/113 rasters=146/146 textures=65/65 lights=33/33 assertions={0 if row['profile']=='master' else 22} hash=";assert row['before'].startswith(prefix),row
report.update(status='staged-verified',production_changed=production,complete_C_operations=35,changed_sources=12,native_algorithm_bridges=0,native_runtime_comparisons=24,remaining=['Complete integration, production application and predecessor audits'])
if '--integration' in sys.argv:
    # Inspect actual evaluated source language and unique object metadata for every native configuration
    rows=json.loads((out/'evaluated-configurations.json').read_text());assert len(rows)==48
    for row in rows:assert row['exit']==0 and row['pairs']==28 and row['c_units']==2 and row['sources']==(431 if row['platform']=='Win32' and 'RW34' in row['configuration'] else 432),row
    project=out/'project/reVC.vcxproj';assert getattr(m,'shadow_cameras_project',m.project)(project.read_text())==project.read_text();ns={'m':'http://schemas.microsoft.com/developer/msbuild/2003'};items=[n.get('Include') for n in ET.parse(project).findall('.//m:ClCompile',ns) if n.get('Include')];assert len(items)==len(set(items))==432
    for item in items:
        # Keep all compilation units and source dependencies on the same candidate tree
        path=(project.parent/item.replace('\\','/')).resolve();assert path.is_relative_to((out/'overlay/src').resolve()) and path.exists(),path
    for configuration in ['debug','release']:
        # Require complete linked executables and actual C11 compiler commands without launching the game
        build=json.loads((out/f'd3d9-{configuration}.json').read_text());log=Path(build['log']).read_text(errors='replace');assert build['exit']==0 and 'Build succeeded.' in log and '0 Error(s)' in log;exe=Path(build['exe']);assert exe.is_file() and exe.stat().st_size==build['size'];objdir=out/'project/obj/win-amd64-librw_d3d9-oal'/configuration.capitalize();commands=(objdir/'reVC.tlog/CL.command.1.tlog').read_text(encoding='utf-16').splitlines()
        for relative in ['renderer/ShadowCamera.c','renderer/CutsceneShadow.c','renderer/Shadow_native.cpp','rw/RwHelper.c']:
            # Verify that each converted producer and its complete C peer participated in the real link
            source=(out/'overlay/src'/relative).resolve();indices=[i for i,v in enumerate(commands) if v.startswith('^') and str(source).upper() in v];assert len(indices)==1,(relative,indices);command=commands[indices[0]+1];assert str(source).upper() in command
            if source.suffix=='.c':assert '/TC ' in command and '/std:c11 ' in command
            obj=objdir/(source.stem+'.obj');assert obj.exists() and obj.stat().st_mtime>=source.stat().st_mtime and exe.stat().st_mtime>=obj.stat().st_mtime
        report['d3d9-'+configuration]=build['size']
    report.update(status='ready-to-apply',remaining=['Production application and predecessor audits'])
    if production:
        # Require production bytes to match every complete candidate source used in the verified links
        for path in (out/'overlay/src').rglob('*'):
            if path.is_file():verify(root,'src/'+path.relative_to(out/'overlay/src').as_posix(),hashlib.sha256(path.read_bytes()).hexdigest())
        for name,count in [('production-owner-compiles',36),('production-consumer-compiles',12),('production-history',588),('preceding-owner-audits',3)]:
            # Keep the full predecessor proof chain as required production verification evidence
            rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==count and all(row['exit']==0 for row in rows),name
        report.update(status='applied-verified',remaining=[])
(out/('verification-summary.json' if production else 'stage-summary.json')).write_text(json.dumps(report,indent=2),newline='\n');print(json.dumps(report,indent=2))
