"""Audit both complete C door owners, explicit lifetimes, native layout and full native links."""
from pathlib import Path
import json,re
import doors_migration as m
from stats_successor import verify
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/doors-c-tests';saved=out/'before';changes=json.loads((out/'changes.json').read_text())
for name in changes:
    expected=m.calls((saved/name).read_text());assert (root/name).read_text()==expected and m.calls(expected)==expected,name
    assert b'\r' not in (root/name).read_bytes(),name
for name,expected in [('DoorSystem.c',m.kernel((saved/'src/vehicles/Door.cpp').read_text(),(saved/'src/vehicles/Door.h').read_text())),('Door_backend.h',m.BACKEND)]:
    assert (root/'src/vehicles'/name).read_text()==expected,name
    assert b'\r' not in (root/'src/vehicles'/name).read_bytes(),name
for path,digest in json.loads((out/'untouched.json').read_text()).items():verify(root,path,digest)
strip=lambda source:re.sub(r'//[^\n]*|/\*[\s\S]*?\*/|"(?:\\.|[^"\\])*"','',source)
for path in (root/'src').rglob('*'):
    if path.suffix in ['.h','.cpp','.c']:assert not re.search(r'\bclass\s+(?:CDoor|CTrainDoor)\b|\b(?:CDoor|CTrainDoor)\s*::',strip(path.read_text(errors='replace'))),str(path)
kernel=(root/'src/vehicles/DoorSystem.c').read_text();assert not re.search(r'::|\b(class|namespace|new|delete)\b',strip(kernel))
for prefix,names in [('Door',m.CAR),('TrainDoor',m.TRAIN)]:
    for name in ['Init','SetLimits']+names:assert len(re.findall(r'^'+prefix+'_'+name+r'\(',kernel,re.M))==1,(prefix,name)
assert len(re.findall(r'^DoorVector Door_Backend\w+\(', (root/'src/vehicles/Door.cpp').read_text(),re.M))==2
for name,block in [('Automobile',m.CAR_INIT),('Boat',m.BOAT_INIT),('Train',m.TRAIN_INIT)]:assert (root/'src/vehicles'/f'{name}.cpp').read_text().count(block)==1,name
train=(root/'src/vehicles/Train.cpp').read_text();assert train.index(m.TRAIN_INIT)<train.index('#ifdef GTA_TRAIN')
for name in ['Automobile.cpp','Boat.cpp']:assert 'TrainDoor_' not in (root/'src/vehicles'/name).read_text(),name
reports={}
for name,count in [('matrix-runs',4),('sanitized-runs',1),('native-runs',2),('native-source-compiles',20),('production-source-compiles',20)]:
    rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==count and all(row['exit']==0 for row in rows),name
    for row in rows:
        if 'before' in row:assert row['before']==row['after']=='states=59469 checks=4104 hash=348d227964ac557f',row
    reports[name]=len(rows)
layouts=json.loads((out/'native-layouts.json').read_text());assert len(layouts)==2
for row in layouts:
    data=dict(zip(row['expressions'],row['values']));bits=row['bits'];assert data['sizeof(CDoor)']==36 and data['sizeof(CTrainDoor)']==24 and data['offsetof(CDoor,m_vecSpeed)']==24
    for record,sizes in [('CAutomobile',{32:1544,64:1816}),('CBoat',{32:1256,64:1480}),('CTrain',{32:808,64:984})]:assert data['sizeof('+record+')']==sizes[bits],(bits,record)
configs=json.loads((out/'evaluated-configurations.json').read_text());assert len(configs)==48
for row in configs:
    # The six legacy Win32 RW34 configurations deliberately exclude the librw compatibility producer
    expectedSources=407 if row['platform']=='Win32' and 'RW34' in row['configuration'] else 408
    assert row['exit']==0 and row['pairs']==28 and row['c_units']==1 and row['sources']==expectedSources,row
reports['evaluated-configurations']=len(configs)
for configuration in ['debug','release']:
    row=json.loads((out/f'd3d9-{configuration}.json').read_text());assert row['exit']==0
    log=Path(row['log']).read_text(errors='replace');assert '0 Error(s)' in log and 'Build succeeded.' in log
    exe=Path(row['exe']);assert exe.exists() and exe.stat().st_size==row['size'];reports['d3d9-'+configuration]=row['size']
reports.update({'classes_removed':2,'public_C_operations':16,'native_geometry_primitives':2,'changed_sources':len(changes),'new_sources':2,'owner_initialization_sites':3,'states':59469,'constructor_and_contract_checks':4104,'car_door_bytes':36,'train_door_bytes':24,'cached_velocity_bytes':12})
(out/'verification-summary.json').write_text(json.dumps(reports,indent=2),newline='\n')
print('Verified both complete C door owners, all 16 operations, geometry/lifecycle comparisons, native layouts and both full D3D9 links')
