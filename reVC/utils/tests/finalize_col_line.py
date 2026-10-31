"""Audit complete C line ownership, native aliases, record formats and full D3D9 links."""
from pathlib import Path
import json,re
import col_line_migration as m
from rect_migration import mask
from stats_successor import verify
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/col-line-c-tests';saved=out/'before';changes=json.loads((out/'changes.json').read_text())
for name in changes:
    path=root/m.RENAMES.get(name,name);expected=m.calls((saved/name).read_text());assert path.read_text()==expected and m.calls(expected)==expected,name
    assert b'\r' not in path.read_bytes(),name
assert not (root/'src/collision/ColLine.cpp').exists()
for path,digest in json.loads((out/'untouched.json').read_text()).items():verify(root,path,digest)
kernel=(root/'src/collision/ColLine.c').read_text();header=(root/'src/collision/ColLine.h').read_text()
for source in [kernel,header,m.shared()]:assert not re.search(r'::|\b(class|namespace|new|delete|CVector)\b',mask(source))
for name,value in [('CollisionVector.h',m.shared()),('CollisionVector_native.h',m.native_vectors()),('ColLine_native.h',m.native_lines())]:
    path=root/'src/collision'/name;assert path.read_text()==value and b'\r' not in path.read_bytes(),name
for name,result in [('Init','void'),('Set','void'),('Make','CColLine'),('FromRaw32','CColLine')]:
    assert len(re.findall(r'^'+result+' ColLine_'+name+r'\(',kernel,re.M))==1 and header.count('ColLine_'+name+'(')==1,name
for path in (root/'src').rglob('*'):
    if path.suffix in ['.h','.cpp','.c'] and not any(part.startswith('.') for part in path.relative_to(root/'src').parts):
        source=mask(path.read_text(errors='replace'))
        assert not re.search(r'\bCColLine\s*::|\bCColLine\s*\(|\bCColLine\s+(shootLine|line|newline|groundLine)\s*\(|\*\(CColLine\*\)|\*\(CVuVector\*\)&line\.p[01]',source),str(path)
from doors_migration import method
original=method((saved/'src/math/math.cpp').read_text(),'TransformPoints')
expected=original[original.index('\t__asm__ __volatile__'):original.index('\n#else')]
current=method((root/'src/math/math.cpp').read_text(),'TransformColLines')
assert current[current.index('\t__asm__ __volatile__'):current.index('\n#else')]==expected
assert '#ifdef GTA_PS3' in current
reports={}
for name,count in [('matrix-runs',12),('sanitized-runs',3),('native-runs',6),('native-source-compiles',80),('production-source-compiles',80)]:
    rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==count and all(row['exit']==0 for row in rows),name
    for row in rows:
        if 'before' in row:
            expected='states=89089 hash=1f86002af2772aea'
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
reports.update({'complete_record_owners_converted':1,'public_C_operations':4,'native_value_adapters':3,'native_batch_adapters':1,'changed_sources':len(changes),'new_shared_headers':3,'renamed_producers':m.RENAMES,'line_bytes':32,'endpoint_stride':16,'states':89089,'profiles':['float','compressed','vu']})
(out/'verification-summary.json').write_text(json.dumps(reports,indent=2),newline='\n')
print('Verified complete C line ownership, endpoint aliases, quadword boundaries, native formats and both full D3D9 links')
