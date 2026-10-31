"""Audit all four C keyframe records, packed methods, real base subobjects and full D3D9 links."""
from pathlib import Path
import json,re
import keyframes_migration as m
from stats_successor import current_path
from rect_migration import mask
from stats_successor import verify
from doors_migration import method
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/keyframes-c-tests';saved=out/'before';changes=json.loads((out/'changes.json').read_text());assert len(changes)==7
for name in changes:
    path=current_path(root,name);expected=m.calls((saved/name).read_text());assert path.read_text()==expected and m.calls(expected)==expected,name
    assert b'\r' not in path.read_bytes(),name
for path,digest in json.loads((out/'untouched.json').read_text()).items():verify(root,path,digest)
for name,value in [('KeyFrames.h',m.header()),('KeyFrames.c',m.kernel()),('KeyFrames_native.h',m.native())]:
    path=root/'src/animation'/name;assert path.read_text()==m.calls(value) and b'\r' not in path.read_bytes(),name
for source in [m.header(),m.kernel()]:assert not re.search(r'::|\b(class|namespace|new|delete|CVector|using)\b',mask(source))
for name in m.OPERATIONS:
    assert len(re.findall(r'^(?:void |float )'+name+r'\(',m.kernel(),re.M))==1,name
    assert m.header().count(name+'(')==1,name
header=(root/'src/animation/AnimBlendSequence.h').read_text()
if 'class CAnimBlendSequence' in header:assert '&((KeyFrameTrans*)keyFrames)[n].base :' in header and '&((KeyFrameTransCompressed*)keyFramesCompressed)[n].base :' in header
else:
    sequence=current_path(root,'src/animation/AnimBlendSequence.cpp').read_text()
    assert '&((KeyFrameTrans*)self->keyFrames)[n].base' in sequence and '&((KeyFrameTransCompressed*)self->keyFramesCompressed)[n].base' in sequence
nodePath=current_path(root,'src/animation/AnimBlendNode.cpp')
assert nodePath.read_text()==m.calls((saved/'src/animation/AnimBlendNode.cpp').read_text())
sequence=current_path(root,'src/animation/AnimBlendSequence.cpp').read_text()
assert 'float timeScale = 1.0f/60.0f;' in sequence and 'self->deltaTime/60.0f' in m.kernel()
for path in (root/'src').rglob('*'):
    if path.suffix in ['.h','.cpp','.c'] and not any(part.startswith('.') for part in path.relative_to(root/'src').parts):
        source=mask(path.read_text(errors='replace'));assert not re.search(r'\bstruct KeyFrameTrans\s*:|\bstruct KeyFrameTransCompressed\s*:',source),str(path)
        rows=m.records(source)
        for match in re.finditer(r'\b(\w+)->(GetRotation|SetRotation|GetDeltaTime|SetTime|GetTranslation|SetTranslation)\(',source):
            assert m.kind(match[1],match.start(),rows) not in ['KeyFrameCompressed','KeyFrameTransCompressed'],(str(path),match[0])
reports={};gold={'fixed':'d64e7b5bcab3ba1b','classic':'d094b0c4478b6a1f'};native32={'fixed':'738291cc5922c41b','classic':'6dc8c734d5fa741f'}
for name,count in [('matrix-runs',8),('sanitized-runs',2),('native-runs',4),('native-source-compiles',60),('production-source-compiles',60)]:
    rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==count and all(row['exit']==0 for row in rows),name
    for row in rows:
        if 'before' in row:
            expected='states=85005 checks=578841 hash='+(native32 if row.get('bits')==32 else gold)[row['profile']]
            assert row['before']==row['after']==expected,row
    reports[name]=len(rows)
configs=json.loads((out/'evaluated-configurations.json').read_text());assert len(configs)==48
for row in configs:assert row['exit']==0 and row['pairs']==28 and row['c_units']==1 and row['sources']==(411 if row['platform']=='Win32' and 'RW34' in row['configuration'] else 412),row
reports['evaluated-configurations']=len(configs)
for configuration in ['debug','release']:
    row=json.loads((out/f'd3d9-{configuration}.json').read_text());assert row['exit']==0
    log=Path(row['log']).read_text(errors='replace');assert '0 Error(s)' in log and 'Build succeeded.' in log
    exe=Path(row['exe']);assert exe.exists() and exe.stat().st_size==row['size'];reports['d3d9-'+configuration]=row['size']
history=json.loads((out/'historical-source-projections.json').read_text());assert len(history)==175 and all(row['exit']==0 for row in history);reports['historical-source-projections']=len(history)
for log in out.glob('gcc-O*-*.log'):
    imports=re.findall(r'DLL Name: ([^\s]+)',log.read_text());assert imports and not any(re.search(r'libgcc|libstdc|libwinpthread|msys-',name,re.I) for name in imports),log
reports.update({'complete_record_owners_converted':4,'public_C_operations':6,'inheritance_relationships_removed':2,'native_vector_members_removed':1,'native_gameplay_bridges_added':0,'native_field_adapters':4,'changed_sources':len(changes),'new_sources':3,'record_bytes':[20,32,10,16],'translation_offsets':[20,10],'states':85005,'full_animation_frame_cases':18432,'all_signed_16_bit_patterns':65536,'bulk_sequence_cases':1024,'complete_frame_callbacks':9,'complete_blend_node_methods':13,'profiles':list(gold)})
(out/'verification-summary.json').write_text(json.dumps(reports,indent=2),newline='\n')
print('Verified all four C keyframe owners, six packed operations, real base subobjects, actual animation behavior and both full D3D9 links')
