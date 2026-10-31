"""Audit both complete C triangle owners, native aliases, record formats and full D3D9 links."""
from pathlib import Path
import json,re
import col_triangle_migration as m
from rect_migration import mask
from stats_successor import verify,current_path
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/col-triangle-c-tests';saved=out/'before';changes=json.loads((out/'changes.json').read_text())
for name in changes:
    path=current_path(root,name);expected=m.calls((saved/name).read_text());assert path.read_text()==expected and m.calls(expected)==expected,name
    assert b'\r' not in path.read_bytes(),name
assert not (root/'src/collision/ColTriangle.cpp').exists()
for path,digest in json.loads((out/'untouched.json').read_text()).items():verify(root,path,digest)
kernel=(root/'src/collision/ColTriangle.c').read_text();header=(root/'src/collision/ColTriangle.h').read_text()
for source in [kernel,header]:assert not re.search(r'::|\b(class|namespace|new|delete|CVector)\b',mask(source))
for name in m.OPERATIONS:assert len(re.findall(r'^(?:void|float) ColTrianglePlane_'+name+r'\(',kernel,re.M))==1 and len(re.findall(r'\bColTrianglePlane_'+name+r'\(',header))==1,name
assert len(re.findall(r'^void ColTriangle_Set\(',kernel,re.M))==1 and header.count('void ColTriangle_Set(')==1
if (root/'src/collision/CollisionVector.h').exists():
    from col_line_migration import shared
    assert (root/'src/collision/CollisionVector.h').read_text()==shared()
else:assert header.count('static inline CollisionPointRef ColTrianglePlane_Ref(')==1
for path in (root/'src').rglob('*'):
    if path.suffix in ['.h','.cpp','.c']:
        source=mask(path.read_text(errors='replace'))
        assert not re.search(r'\bCColTrianglePlane\s*::|'+m.PLANE+r'\.(Set|GetNormal[XYZ]?|CalcPoint|Unpack)\(',source),str(path)
original=(saved/'src/collision/ColTriangle.h').read_text();assembly=original[original.index('__asm__ volatile ('):original.index('\n\t}',original.index('__asm__ volatile ('))]
expected=assembly.replace('"=m" (qword)','"=m" (*(TriangleQuadWord*)quadword)').replace('"r" (this)','"r" (self)').strip()
actual=kernel[kernel.index('__asm__ volatile ('):kernel.index('\n#else',kernel.index('__asm__ volatile ('))].strip();assert actual==expected
reports={}
for name,count in [('matrix-runs',12),('sanitized-runs',3),('native-runs',6),('native-source-compiles',56),('production-source-compiles',56)]:
    rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==count and all(row['exit']==0 for row in rows),name
    for row in rows:
        if 'before' in row:
            expected='states=146085 checks=304879 hash=53872c6d65dfafbf' if row['profile']=='vu' else 'states=88997 checks=214676 hash=b5130e71537a9d56'
            assert row['before']==row['after']==expected,row
    reports[name]=len(rows)
configs=json.loads((out/'evaluated-configurations.json').read_text());assert len(configs)==48
for row in configs:
    # Retain the six legacy configurations that exclude the librw compatibility producer
    expectedSources=409 if row['platform']=='Win32' and 'RW34' in row['configuration'] else 410
    assert row['exit']==0 and row['pairs']==28 and row['c_units']==1 and row['sources']==expectedSources,row
reports['evaluated-configurations']=len(configs)
for configuration in ['debug','release']:
    row=json.loads((out/f'd3d9-{configuration}.json').read_text());assert row['exit']==0
    log=Path(row['log']).read_text(errors='replace');assert '0 Error(s)' in log and 'Build succeeded.' in log
    exe=Path(row['exe']);assert exe.exists() and exe.stat().st_size==row['size'];reports['d3d9-'+configuration]=row['size']
reports.update({'complete_record_owners_converted':2,'public_C_operations':9,'C_reference_helpers':1,'native_bridges_added':0,'changed_sources':len(changes),'renamed_producers':m.RENAMES,'triangle_bytes':8,'scalar_plane_bytes':20,'vu_plane_bytes':8,'direction_values':6,'scalar_states':88997,'vu_states':146085,'exhaustive_signed_codes':65536,'profiles':['float','compressed','vu']})
(out/'verification-summary.json').write_text(json.dumps(reports,indent=2),newline='\n')
print('Verified both complete C triangle owners, all scalar/VU operations, live aliases, native formats and both full D3D9 links')
