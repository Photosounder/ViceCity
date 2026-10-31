"""Require complete source, native ABI, behavior and integration evidence for C weapon effects."""
from pathlib import Path
import hashlib,json,re,sys,xml.etree.ElementTree as ET
import weapon_effects_migration as m
from rect_migration import mask
from stats_successor import verify,current_path
from memory_heap_migration import body

root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/weapon-effects-c-tests';production='--production' in sys.argv;tree=root if production else out/'overlay'
digests=json.loads((out/'baseline-source-digests.json').read_text());changes=json.loads((out/'changes.json').read_text());assert len(digests)==898 and len(changes)==6
for name,digest in digests.items():
    # Prove every whole existing source projection and reject concurrent baseline changes before application
    original=out/'before'/name;assert hashlib.sha256(original.read_bytes()).hexdigest()==digest,name
    if not production:assert hashlib.sha256((root/name).read_bytes()).hexdigest()==digest,name
    target=current_path(root,name) if production else tree/m.RENAMES.get(name,name);expected=m.calls(original.read_text());assert target.read_text()==expected and m.calls(expected)==expected,name
    if name in changes:assert b'\r' not in target.read_bytes(),name
original=(out/'before/src/weapons/WeaponEffects.cpp').read_text();header,native=m.boundary(original);assert (tree/'src/weapons/WeaponEffects_backend.h').read_text()==header and (tree/'src/weapons/WeaponEffects_native.cpp').read_text()==native
assert not re.search(r'\b(?:for|while)\s*\(',mask(native))
kernel=(tree/'src/weapons/WeaponEffects.c').read_text();assert not (tree/'src/weapons/WeaponEffects.cpp').exists()
assert not re.search(r'::|\b(?:class|new|delete|namespace|CVector|RwV3d|RwTexture)\b',mask(kernel))
for name in ['Init','Shutdown','MarkTarget','ClearCrossHair','Render','Construct','Deinit']:
    # Require complete C definitions rather than retained C++ operation wrappers
    assert len(re.findall(r'(?m)^void\s+WeaponEffects_'+name+r'\([^;{}]*\)\s*\{',mask(kernel)))==1,name
for old,new in [('CWeaponEffects::CWeaponEffects()','WeaponEffects_Construct('),('CWeaponEffects::~CWeaponEffects()','WeaponEffects_Deinit(')]:
    # Prove that both original empty lifetimes remain empty apart from discarding the unused C receiver
    assert not mask(body(original,old)).strip() and mask(body(kernel,new)).strip()=='(void)self;'
assert not mask(body((out/'before/src/math/Vector.h').read_text(),'CVector(void)')).strip()
for path in (tree/'src').rglob('*'):
    # Reject obsolete class uses across the entire coherent project tree
    if path.suffix in {'.cpp','.c','.h'}:assert not re.search(r'\bCWeaponEffects\b',mask(path.read_text())),path
report={'status':'staged-verified','production_changed':production,'complete_C_operations':7,'changed_sources':6,'native_owner_algorithm_bridges':0}
for name,count in [('strict-compiles',40),('native-owner-compiles',18),('native-consumer-compiles',8),('native-layouts',2),('historical-source-projections',312)]:
    # Inspect every actual compiler and full historical projection row
    rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==count and all(r['exit']==0 for r in rows),name;report[name]=count
for row in json.loads((out/'native-layouts.json').read_text()):
    # Require the complete real SDK layout and both independent public interface languages to agree
    assert row['bits'] in [32,64] and row['consumers']==['original-sdk','new-c','new-cpp'] and row['layout']==[28,4,0,4,16,17,18,19,20,24],row
rows=json.loads((out/'native-runs.json').read_text());profiles=['d3d9','gl','software','master','final','legacy','classic','rwlibs','ps2','ps2-pal']
assert len(rows)==40 and {(r['bits'],r['optimization'],r['profile']) for r in rows}=={(a,b,c) for a in [32,64] for b in ['Od','O2'] for c in profiles}
for row in rows:
    # Pin full original traces covering every valid weapon, failure, reentry and late rotation mutation
    quiet=row['profile'] in ['master','final'];trace='checks='+('57 events=612' if quiet else '141 events=696')+' reads=2 destroys=1 draws=84 projections=43 queries=43 hash='+('437b271bc0ea8e90' if quiet else '651e25d724825fbb' if row['profile']=='classic' else 'b4d4c8acfee355c8')
    assert row['exit']==0 and row['before']==row['after']==trace,row
report.update(native_runtime_comparisons=40,remaining=['Full integration audit and production application'])
if '--integration' in sys.argv:
    # Require real evaluated project metadata and both complete full executable links
    rows=json.loads((out/'evaluated-configurations.json').read_text());assert len(rows)==48
    for row in rows:assert row['exit']==0 and row['pairs']==28 and row['c_units']==1 and row['sources']==(435 if row['platform']=='Win32' and 'RW34' in row['configuration'] else 436),row
    project=out/'project/reVC.vcxproj';assert getattr(m,'weapon_effects_project',m.project)(project.read_text())==project.read_text();ns={'m':'http://schemas.microsoft.com/developer/msbuild/2003'};items=[n.get('Include') for n in ET.parse(project).findall('.//m:ClCompile',ns) if n.get('Include')];assert len(items)==len(set(items))==436
    for item in items:
        # Verify that every source in the full build comes from the same candidate tree
        path=(project.parent/item.replace('\\','/')).resolve();assert path.is_relative_to((out/'overlay/src').resolve()) and path.exists(),path
    for configuration in ['debug','release']:
        # Confirm actual object records, explicit C11 compilation and linked executable freshness
        build=json.loads((out/f'd3d9-{configuration}.json').read_text());log=Path(build['log']).read_text(errors='replace');assert build['exit']==0 and 'Build succeeded.' in log and '0 Error(s)' in log;exe=Path(build['exe']);assert exe.exists() and exe.stat().st_size==build['size'];objdir=out/'project/obj/win-amd64-librw_d3d9-oal'/configuration.capitalize();commands=(objdir/'reVC.tlog/CL.command.1.tlog').read_text(encoding='utf-16').splitlines()
        for relative in ['WeaponEffects.c','WeaponEffects_native.cpp']:
            # Require the entire C owner and only its native primitive boundary in each full build
            source=(out/'overlay/src/weapons'/relative).resolve();indices=[i for i,v in enumerate(commands) if v.startswith('^') and str(source).upper() in v];assert len(indices)==1,(relative,indices);command=commands[indices[0]+1]
            if source.suffix=='.c':assert '/TC ' in command and '/std:c11 ' in command
            obj=objdir/(source.stem+'.obj');assert obj.exists() and obj.stat().st_mtime>=source.stat().st_mtime and exe.stat().st_mtime>=obj.stat().st_mtime
        report['d3d9-'+configuration]=build['size']
    report.update(status='ready-to-apply',remaining=['Production application and predecessor audits'])
    if production:
        # Verify all applied source bytes and current production compiler and predecessor evidence
        for path in (out/'overlay/src').rglob('*'):
            if path.is_file():verify(root,'src/'+path.relative_to(out/'overlay/src').as_posix(),hashlib.sha256(path.read_bytes()).hexdigest())
        for name,count in [('production-owner-compiles',18),('production-consumer-compiles',8),('production-history',312),('preceding-owner-audits',3)]:
            # Require complete production checks before declaring the owner applied and verified
            rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==count and all(r['exit']==0 for r in rows),name
        report.update(status='applied-verified',remaining=[])
(out/('verification-summary.json' if production else 'stage-summary.json')).write_text(json.dumps(report,indent=2),newline='\n');print(json.dumps(report,indent=2))
