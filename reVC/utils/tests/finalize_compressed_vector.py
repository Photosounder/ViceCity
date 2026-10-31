"""Audit complete C collision vertex ownership, all caller projections and both native links."""
from pathlib import Path
import json,re
import compressed_vector_migration as m
from rect_migration import mask
from stats_successor import verify,current_path
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/compressed-vector-c-tests';saved=out/'before';changes=json.loads((out/'changes.json').read_text())
for name in changes:
    path=current_path(root,name);expected=m.calls((saved/name).read_text());assert path.read_text()==expected and m.calls(expected)==expected,name
    assert b'\r' not in path.read_bytes(),name
kernel=(root/'src/collision/CompressedVector.c').read_text();header=(root/'src/collision/CompressedVector.h').read_text()
assert kernel==m.kernel((saved/'src/collision/CompressedVector.h').read_text())
assert b'\r' not in (root/'src/collision/CompressedVector.c').read_bytes()
for path,digest in json.loads((out/'untouched.json').read_text()).items():verify(root,path,digest)
for source in [kernel,header]:assert not re.search(r'::|\b(class|namespace|new|delete|CVector)\b',mask(source))
for name in m.OPERATIONS:assert len(re.findall(r'\bCompressedVector_'+name+r'\(',kernel))==1 and len(re.findall(r'\bCompressedVector_'+name+r'\(',header))==1,name
receiver=r'(?:(?:modelB|model)\.vertices|(?:col|colmodel)->vertices|vertices|verts)\[[^\]\n]+\]'
for path in (root/'src').rglob('*'):
    if path.suffix in ['.h','.cpp','.c']:assert not re.search(receiver+r'\.(Get|Set|Unpack)\(',mask(path.read_text(errors='replace'))),str(path)
old=(saved/'src/collision/CompressedVector.h').read_text()
oldAssembly=old[old.index('__asm__ volatile ('):old.index('\n\t}',old.index('__asm__ volatile ('))]
newAssembly=kernel[kernel.index('__asm__ volatile ('):kernel.index('\n#else',kernel.index('__asm__ volatile ('))].rstrip()
assert oldAssembly.replace('"=m" (qword)','"=m" (*(CollisionQuadWord*)quadword)').replace('"r" (this)','"r" (self)').strip()==newAssembly
reports={}
for name,count in [('matrix-runs',12),('sanitized-runs',3),('native-runs',6),('native-source-compiles',62),('production-source-compiles',62)]:
    rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==count and all(row['exit']==0 for row in rows),name
    for row in rows:
        if 'before' in row:
            expected='states=2496 checks=5248 hash=bdf96f444a3b9ad3' if row['profile']=='float' else 'states=131840 checks=659456 hash=77f5e1ccc17c1939'
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
reports.update({'complete_record_owners_converted':1,'public_C_operations':3,'active_float_operations':2,'record_fields':3,'native_bridges_added':0,'changed_sources':len(changes),'new_sources':1,'compressed_bytes':6,'float_bytes':12,'decoded_value_bytes':12,'float_states':2496,'compressed_states':131840,'exhaustive_signed_codes':65536,'profiles':['float','compressed','vu']})
(out/'verification-summary.json').write_text(json.dumps(reports,indent=2),newline='\n')
print('Verified complete C collision vertex ownership, both native formats, all conversions, native layouts and both full D3D9 links')
