"""Audit complete C sprite ownership against exact source, runtime, ABI, and integration evidence."""
from pathlib import Path
import hashlib,json,re,sys,xml.etree.ElementTree as ET
import sprite_migration as m,sprite_schema as schema,sprite_boundary as boundary,sprite_peers as peers
from rect_migration import mask
from stats_successor import verify,current_path
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/sprite-c-tests';production='--production' in sys.argv;tree=root if production else out/'overlay';report=json.loads((out/'stage-summary.json').read_text());changes=json.loads((out/'changes.json').read_text());assert len(changes)==16
for name,digest in json.loads((out/'baseline-source-digests.json').read_text()).items():
    # Verify every complete existing source against its immutable full-source projection
    if not production:assert hashlib.sha256((root/name).read_bytes()).hexdigest()==digest,name
    original=(out/'before'/name).read_text();target=current_path(root,name) if production else tree/m.RENAMES.get(name,name);assert target.read_text()==m.calls(original) and m.calls(target.read_text())==target.read_text(),name
    if name in changes:assert b'\r' not in target.read_bytes(),name
header,native=boundary.build();assert (tree/'src/renderer/Sprite_backend.h').read_text()==header;assert (tree/'src/renderer/Sprite_native.cpp').read_text()==native
assert not re.search(r'\b(?:for|while)\s*\(',mask(native)) and 'static RwIm2DVertex spriteBufferedVertices[64*6];' in native and 'static RwIm2DVertex spriteImmediateVertices[4];' in native
kernel=(tree/'src/renderer/Sprite.c').read_text();bindings=(tree/'src/renderer/Sprite_bindings.c').read_text();assert not (tree/'src/renderer/Sprite.cpp').exists();assert not re.search(r'::|\b(?:class|new|delete|namespace|RwIm2DVertex|RwV3d|CVector|CRGBA)\b',mask(kernel+'\n'+bindings))
for name in schema.OPERATIONS:
    # Require complete C definitions for all original operations including both former overloads and both inline getters
    assert len(re.findall(r'(?m)^(?:float|bool|void) Sprite_'+name+r'\([^;{}]*\)\s*\{',mask(kernel)))==1,name
originals=[(out/f'before/src/renderer/{name}.cpp').read_text() for name in peers.SOURCES];assert bindings==peers.bindings(originals)
for name in peers.NAMES:
    # Require every obsolete native forwarding operation and point lifetime to have a genuine C definition
    assert len(re.findall(r'(?m)^(?:float|bool|void) '+name+r'\([^;{}]*\)\s*\{',mask(bindings)))==1,name
for path in (tree/'src').rglob('*'):
    # Reject remaining owner class definitions or member calls anywhere in the complete source tree
    if path.suffix in {'.cpp','.c','.h'}:assert not re.search(r'\bCSprite::|\bclass\s+CSprite\b',mask(path.read_text())),path
for name,count in [('strict-compiles',72),('native-owner-compiles',24),('native-consumer-compiles',26),('native-layouts',2),('historical-source-projections',800)]:
    # Inspect complete compiler and historical rows instead of accepting a stage status alone
    rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==count and all(row['exit']==0 for row in rows),name;report[name]=count
rows=json.loads((out/'native-runs.json').read_text());profiles=['d3d9','gl','software','master','final','legacy','no-fix','no-aspect','ps2','ps2-pal'];assert len(rows)==40 and {(r['bits'],r['optimization'],r['profile']) for r in rows}=={(a,b,c) for a in [32,64] for b in ['Od','O2'] for c in profiles};pins=json.loads((out/'original-runtime-traces.json').read_text())
for row in rows:
    # Pin complete original traces with every operation every C peer and shared-buffer reentry covered
    key=f"{row['bits']}-{row['optimization']}-{row['profile']}";assert row['exit']==0 and row['before']==row['after']==pins[key],row
    events=13314 if row['profile']=='software' else 13296 if row['profile']=='no-fix' else 13236 if row['profile']=='no-aspect' else 13254;assert row['before'].startswith(f'commands=451 checks=859 events={events} draws=52 reentries=3 mutations=4 hash='),row
rows=json.loads((out/'native-layouts.json').read_text());assert {r['bits'] for r in rows}=={32,64}
for row in rows:
    # Inspect independently compiled native SDK derived and C/C++ public scalar-reference layouts
    bits=row['bits'];assert row['consumers']==['original-sdk','new-cpp','new-c'];layout=row['layout'];assert 'SpriteVector 12 4 x=0 y=4 z=8\n' in layout and 'SpriteColour 4 1 r=0 g=1 b=2 a=3\n' in layout
    assert ('ScrollBarPoint 16 4 owner=0 x=4 y=8 z=12\n' if bits==32 else 'ScrollBarPoint 32 8 owner=0 x=8 y=16 z=24\n') in layout
report.update(status='staged-verified',production_changed=production,complete_C_operations=17,C_peer_forwarders=18,changed_sources=16,native_owner_algorithm_bridges=0,native_runtime_comparisons=40,remaining=['Complete integration, production application and predecessor audits'])
if '--integration' in sys.argv:
    # Inspect both actual C11 units and all unique object paths across every evaluated native configuration
    rows=json.loads((out/'evaluated-configurations.json').read_text());assert len(rows)==48
    for row in rows:assert row['exit']==0 and row['pairs']==28 and row['c_units']==2 and row['sources']==(433 if row['platform']=='Win32' and 'RW34' in row['configuration'] else 434),row
    project=out/'project/reVC.vcxproj';assert getattr(m,'sprite_project',m.project)(project.read_text())==project.read_text();ns={'m':'http://schemas.microsoft.com/developer/msbuild/2003'};items=[n.get('Include') for n in ET.parse(project).findall('.//m:ClCompile',ns) if n.get('Include')];assert len(items)==len(set(items))==434
    for item in items:
        # Keep the entire real game build on one coherent candidate tree
        path=(project.parent/item.replace('\\','/')).resolve();assert path.is_relative_to((out/'overlay/src').resolve()) and path.exists(),path
    for configuration in ['debug','release']:
        # Require actual full linked executables and the compiler records for both complete C units
        build=json.loads((out/f'd3d9-{configuration}.json').read_text());log=Path(build['log']).read_text(errors='replace');assert build['exit']==0 and 'Build succeeded.' in log and '0 Error(s)' in log;exe=Path(build['exe']);assert exe.is_file() and exe.stat().st_size==build['size'];objdir=out/'project/obj/win-amd64-librw_d3d9-oal'/configuration.capitalize();commands=(objdir/'reVC.tlog/CL.command.1.tlog').read_text(encoding='utf-16').splitlines()
        for relative in ['Sprite.c','Sprite_bindings.c','Sprite_native.cpp','ScrollBar.c']:
            # Verify the C owner and moved peer lifetimes participated in each real executable link
            source=(out/'overlay/src/renderer'/relative).resolve();indices=[i for i,v in enumerate(commands) if v.startswith('^') and str(source).upper() in v];assert len(indices)==1,(relative,indices);command=commands[indices[0]+1];assert str(source).upper() in command
            if source.suffix=='.c':assert '/TC ' in command and '/std:c11 ' in command
            obj=objdir/(source.stem+'.obj');assert obj.exists() and obj.stat().st_mtime>=source.stat().st_mtime and exe.stat().st_mtime>=obj.stat().st_mtime
        report['d3d9-'+configuration]=build['size']
    report.update(status='ready-to-apply',remaining=['Production application and predecessor audits'])
    if production:
        # Require exact production bytes and complete predecessor proofs after applying the compiled candidate
        for path in (out/'overlay/src').rglob('*'):
            if path.is_file():verify(root,'src/'+path.relative_to(out/'overlay/src').as_posix(),hashlib.sha256(path.read_bytes()).hexdigest())
        for name,count in [('production-owner-compiles',24),('production-consumer-compiles',26),('production-history',800),('preceding-owner-audits',3)]:
            # Preserve actual full audits rather than merely reusing previous summary claims
            rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==count and all(row['exit']==0 for row in rows),name
        report.update(status='applied-verified',remaining=[])
(out/('verification-summary.json' if production else 'stage-summary.json')).write_text(json.dumps(report,indent=2),newline='\n');print(json.dumps(report,indent=2))
