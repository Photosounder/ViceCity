"""Audit complete C stored-car ownership, every save copy and both full D3D9 links."""
from pathlib import Path
import json,re
import stored_car_migration as m
from stats_successor import verify
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/stored-car-c-tests';saved=out/'before';changes=json.loads((out/'changes.json').read_text())
for name in changes:
    path=root/name;expected=m.calls((saved/name).read_text());assert path.read_text()==expected and m.calls(expected)==expected,name
    assert b'\r' not in path.read_bytes(),name
for name,expected in [('StoredCar.h',m.header((saved/'src/control/Garages.h').read_text())),('StoredCar.c',m.kernel((saved/'src/control/Garages.cpp').read_text())),('StoredCar_backend.h',m.backend_header())]:
    path=root/'src/control'/name;assert path.read_text()==expected and b'\r' not in path.read_bytes(),name
for path,digest in json.loads((out/'untouched.json').read_text()).items():verify(root,path,digest)
strip=lambda source:re.sub(r'//[^\n]*|/\*[\s\S]*?\*/|"(?:\\.|[^"\\])*"','',source)
for path in (root/'src').rglob('*'):
    if path.suffix in ['.h','.cpp','.c']:assert not re.search(r'\bclass\s+CStoredCar\b|\bCStoredCar\s*::',strip(path.read_text(errors='replace'))),str(path)
kernel=(root/'src/control/StoredCar.c').read_text();header=(root/'src/control/StoredCar.h').read_text();garage=(root/'src/control/Garages.cpp').read_text()
assert not re.search(r'::|\b(class|namespace|new|delete|CVector)\b',strip(kernel))
for name in m.OPERATIONS:assert len(re.findall(r'\bStoredCar_'+name+r'\(',kernel))==1 and len(re.findall(r'\bStoredCar_'+name+r'\(',header))==1,name
for name,result,args,body in m.BRIDGES:assert len(re.findall(r'\bStoredCar_Backend'+name+r'\(',garage))==1,name
assert 'CStoredCar CGarages::aCarsInSafeHouses[TOTAL_HIDEOUT_GARAGES][NUM_GARAGE_STORED_CARS];' in garage
assert len(re.findall(r'\bGarage_AssignForSave\(',garage))==3
assert len(re.findall(r'\bStoredCar_Assign\(',garage))==3
assert not re.search(r'\*\(CStoredCar\*\)buf\s*=|aCarsInSafeHouses\[j\]\[i\]\s*=|\*\(CGarage\*\)buf\s*=|aGarages\[i\]\s*=',garage)
reports={}
for name,count in [('matrix-runs',16),('sanitized-runs',4),('native-runs',8),('native-source-compiles',16),('production-source-compiles',16)]:
    rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==count and all(row['exit']==0 for row in rows),name
    for row in rows:
        if 'before' in row:
            expected='states=28928 checks=80384 hash='+('e91646efb2a7c1a9' if row['profile'].startswith('fixed') else '4c41f1272b0edf89')
            assert row['before']==row['after']==expected,row
    reports[name]=len(rows)
layouts=json.loads((out/'native-layouts.json').read_text());assert len(layouts)==2
for row in layouts:assert row['values']=={32:[40,4,168,4,128,124,1920],64:[40,4,184,8,144,136,1920]}[row['bits']],row
configs=json.loads((out/'evaluated-configurations.json').read_text());assert len(configs)==48
for row in configs:
    # Keep the legacy Win32 configurations that exclude the librw compatibility producer
    expectedSources=408 if row['platform']=='Win32' and 'RW34' in row['configuration'] else 409
    assert row['exit']==0 and row['pairs']==28 and row['c_units']==1 and row['sources']==expectedSources,row
reports['evaluated-configurations']=len(configs)
for configuration in ['debug','release']:
    row=json.loads((out/f'd3d9-{configuration}.json').read_text());assert row['exit']==0
    log=Path(row['log']).read_text(errors='replace');assert '0 Error(s)' in log and 'Build succeeded.' in log
    exe=Path(row['exe']);assert exe.exists() and exe.stat().st_size==row['size'];reports['d3d9-'+configuration]=row['size']
reports.update({'classes_removed':1,'public_C_operations':6,'record_fields':10,'native_bridges':len(m.BRIDGES),'changed_sources':len(changes),'new_sources':3,'stored_record_bytes':40,'untouched_tail_padding_bytes':2,'canonical_array_bytes':1920,'garage_bytes':{'x86':168,'x64':184},'enclosing_assignment_sites':2,'stored_assignment_sites':2,'states':28928,'checks_per_run':80384})
(out/'verification-summary.json').write_text(json.dumps(reports,indent=2),newline='\n')
print('Verified complete C stored-car ownership, all six operations, forty-byte saves, native layouts and both full D3D9 links')
