"""Audit the complete 2D sprite C owner, all consumers, lifetimes, and native build evidence."""
from pathlib import Path
import hashlib,json,re,sys,xml.etree.ElementTree as ET
import sprite2d_migration as m,sprite2d_schema as schema,sprite2d_boundary as boundary
from rect_migration import mask
from stats_successor import verify,current_path

root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/sprite2d-c-tests';production='--production' in sys.argv;tree=root if production else out/'overlay'
changes=json.loads((out/'changes.json').read_text());assert len(changes)==27
digests=json.loads((out/'baseline-source-digests.json').read_text());assert len(digests)==896
for name,digest in digests.items():
    # Compare every original source and entire consumer projection with the coherent candidate
    original=out/'before'/name;assert hashlib.sha256(original.read_bytes()).hexdigest()==digest,name
    if not production:assert hashlib.sha256((root/name).read_bytes()).hexdigest()==digest,name
    target=current_path(root,name) if production else tree/m.RENAMES.get(name,name);expected=m.calls(original.read_text());assert target.read_text()==expected and m.calls(expected)==expected,name
    if name in changes:assert b'\r' not in target.read_bytes(),name
original=(out/'before/src/renderer/Sprite2d.cpp').read_text();header,native=boundary.build(original)
assert (tree/'src/renderer/Sprite2d_backend.h').read_text()==header and (tree/'src/renderer/Sprite2d_native.cpp').read_text()==native
assert not re.search(r'\b(?:for|while)\s*\(',mask(native)) and 'static RwIm2DVertex sprite2dVertices[8];' in native
kernel=(tree/'src/renderer/Sprite2d.c').read_text();assert not (tree/'src/renderer/Sprite2d.cpp').exists()
assert not re.search(r'::|\b(?:class|new|delete|namespace|RwIm2DVertex|RwV3d|CVector|CRGBA)\b',mask(kernel))
operations=['Init','Deinit','GetVertices']+[r[1] for r in schema.signatures(original)];assert len(operations)==len(set(operations))==29
for name in operations+['InitArray','DeinitArray']:
    # Require genuine C definitions for all original operations and both explicit array lifecycle helpers
    assert len(re.findall(r'(?m)^(?:void\s*\*\s*|(?:void|bool)\s+)Sprite2d_'+name+r'\([^;{}]*\)\s*\{',mask(kernel)))==1,name
oldheader=(out/'before/src/renderer/Sprite2d.h').read_text();assert 'CSprite2d(void) : m_pTexture(nil)' in oldheader and '~CSprite2d(void) { Delete(); }' in oldheader and 'GetVertices() { return maVertices; }' in oldheader
for path in (tree/'src').rglob('*'):
    # Reject obsolete owner declarations or references anywhere in the complete project source tree
    if path.suffix in {'.cpp','.c','.h'}:assert not re.search(r'\bCSprite2d\b',mask(path.read_text())),path
report={'status':'staged-verified','production_changed':production,'complete_C_operations':29,'C_array_lifecycle_helpers':2,'changed_sources':27,'global_lifetime_groups':45,'lazy_lifetime_groups':1,'native_owner_algorithm_bridges':0}
for name,count in [('strict-compiles',40),('native-owner-compiles',18),('native-consumer-compiles',32),('native-layouts',6),('historical-source-projections',1377)]:
    # Inspect every complete compiler and projection row rather than accepting previous status summaries
    rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==count and all(r['exit']==0 for r in rows),name;report[name]=count
rows=json.loads((out/'native-runs.json').read_text());profiles=['d3d9','gl','software','master','final','legacy','classic','rwlibs','ps2','ps2-pal'];pins=json.loads((out/'original-runtime-traces.json').read_text())
assert len(rows)==40 and {(r['bits'],r['optimization'],r['profile']) for r in rows}=={(a,b,c) for a in [32,64] for b in ['Od','O2'] for c in profiles}
for row in rows:
    # Preserve pinned complete original traces for every ABI optimization and feature profile
    key=f"{row['bits']}-{row['optimization']}-{row['profile']}";assert row['exit']==0 and row['before']==row['after']==pins[key],row
    assert row['before'].startswith('commands=273 checks=763 events=8069 reads=6 destroys=5 failures=1 draws=25 reentries=1 mutations=8 hash='),row
report['native_runtime_comparisons']=40
for name,count,profiles in [('global-lifetime-runs',4,[None]),('menu-lifetime-runs',12,['pc','ps2','ps2-classic'])]:
    # Check all original implicit destruction and explicit C cleanup comparisons on both Windows ABIs
    rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==count
    assert {(r['bits'],r['optimization'],r.get('profile')) for r in rows}=={(a,b,c) for a in [32,64] for b in ['Od','O2'] for c in profiles}
    for row in rows:
        # Require complete trace equality plus independently checked texture destruction expectations
        assert row['exit']==0 and row['before']==row['after'],row
        if name=='global-lifetime-runs':assert row['before']=='groups=45 reads=130 destroys=130 events=390 hash=9581d5e80af6ac4a',row
        else:
            # Pin actual unload and destructor traces separately for each menu configuration
            endings={'pc':'scenarios=6 reads=175 destroys=175 events=576 hash=4d68d1daeb71ad6c','ps2':'scenarios=6 reads=158 destroys=158 events=484 hash=12f16ce3ca2b9f91','ps2-classic':'scenarios=6 reads=158 destroys=158 events=476 hash=2bbcf20c36c81811'}
            assert row['before'].splitlines()[-1]==endings[row['profile']],row
    report[name]=count
for row in json.loads((out/'native-layouts.json').read_text()):
    # Inspect complete actual SDK menu layouts rather than the reduced lifetime projection containers
    bits=row['bits'];pc=row['profile']=='pc';assert row['record_size']==row['record_alignment']==bits//8 and row['texture_offset']==0
    assert row['parent_size']==({32:1724,64:1880} if pc else {32:220,64:336})[bits] and row['array_offset']==({32:132,64:144} if pc else {32:60,64:64})[bits] and row['array_count']==(31 if pc else 28),row
report['remaining']=['Full integration audit and production application']
if '--integration' in sys.argv:
    # Inspect language selection and unique object paths across all actual evaluated configurations
    rows=json.loads((out/'evaluated-configurations.json').read_text());assert len(rows)==48
    for row in rows:assert row['exit']==0 and row['pairs']==28 and row['c_units']==1 and row['sources']==(434 if row['platform']=='Win32' and 'RW34' in row['configuration'] else 435),row
    project=out/'project/reVC.vcxproj';assert getattr(m,'sprite2d_project',m.project)(project.read_text())==project.read_text();ns={'m':'http://schemas.microsoft.com/developer/msbuild/2003'};items=[n.get('Include') for n in ET.parse(project).findall('.//m:ClCompile',ns) if n.get('Include')];assert len(items)==len(set(items))==435
    for item in items:
        # Keep the entire full executable build on one coherent candidate tree
        path=(project.parent/item.replace('\\','/')).resolve();assert path.is_relative_to((out/'overlay/src').resolve()) and path.exists(),path
    for configuration in ['debug','release']:
        # Verify both full linked executables contain the actual C owner and its native primitive unit
        build=json.loads((out/f'd3d9-{configuration}.json').read_text());log=Path(build['log']).read_text(errors='replace');assert build['exit']==0 and 'Build succeeded.' in log and '0 Error(s)' in log;exe=Path(build['exe']);assert exe.is_file() and exe.stat().st_size==build['size'];objdir=out/'project/obj/win-amd64-librw_d3d9-oal'/configuration.capitalize();commands=(objdir/'reVC.tlog/CL.command.1.tlog').read_text(encoding='utf-16').splitlines()
        for relative in ['Sprite2d.c','Sprite2d_native.cpp']:
            # Require real compiler records and objects newer than the exact source used in each executable
            source=(out/'overlay/src/renderer'/relative).resolve();indices=[i for i,v in enumerate(commands) if v.startswith('^') and str(source).upper() in v];assert len(indices)==1,(relative,indices);command=commands[indices[0]+1]
            if source.suffix=='.c':assert '/TC ' in command and '/std:c11 ' in command
            obj=objdir/(source.stem+'.obj');assert obj.exists() and obj.stat().st_mtime>=source.stat().st_mtime and exe.stat().st_mtime>=obj.stat().st_mtime
        report['d3d9-'+configuration]=build['size']
    report.update(status='ready-to-apply',remaining=['Production application and predecessor audits'])
    if production:
        # Verify every applied source byte against the fully compiled candidate tree
        for path in (out/'overlay/src').rglob('*'):
            if path.is_file():verify(root,'src/'+path.relative_to(out/'overlay/src').as_posix(),hashlib.sha256(path.read_bytes()).hexdigest())
        for name,count in [('production-owner-compiles',18),('production-consumer-compiles',32),('production-history',1377),('preceding-owner-audits',3)]:
            # Require current applied compiler results and complete predecessor audit executions
            rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==count and all(row['exit']==0 for row in rows),name
        report.update(status='applied-verified',remaining=[])
(out/('verification-summary.json' if production else 'stage-summary.json')).write_text(json.dumps(report,indent=2),newline='\n');print(json.dumps(report,indent=2))
