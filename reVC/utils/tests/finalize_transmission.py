"""Audit complete C transmission relocation, audio bridge removal, native layouts and both full links."""
from pathlib import Path
import json,re
import transmission_migration as m
from stats_successor import verify
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/transmission-c-tests';saved=out/'before';changes=json.loads((out/'changes.json').read_text())
for name in changes:
    current=root/m.RENAMES.get(name,name);expected=m.calls((saved/name).read_text())
    assert current.read_text()==expected and m.calls(expected)==expected,name
    assert b'\r' not in current.read_bytes(),name
assert not (root/'src/vehicles/Transmission.cpp').exists()
flags=(root/'src/vehicles/HandlingFlags.h').read_text();assert flags==m.flags((saved/'src/vehicles/HandlingMgr.h').read_text()) and m.calls(flags)==flags
assert b'\r' not in (root/'src/vehicles/HandlingFlags.h').read_bytes()
for path,digest in json.loads((out/'untouched.json').read_text()).items():verify(root,path,digest)
strip=lambda source:re.sub(r'//[^\n]*|/\*[\s\S]*?\*/|"(?:\\.|[^"\\])*"','',source)
for path in (root/'src').rglob('*'):
    if path.suffix in ['.h','.cpp','.c']:
        text=strip(path.read_text(errors='replace'));assert not re.search(r'\bclass\s+cTransmission\b|\bcTransmission\s*::',text),str(path)
        assert not any(re.search(r'\b'+name+r'\b',text) for name in m.READERS),str(path)
for name in ['Transmission.h','Transmission.c','HandlingFlags.h']:assert not re.search(r'::|\b(class|namespace|new|delete)\b',strip((root/'src/vehicles'/name).read_text())),name
kernel=(root/'src/vehicles/Transmission.c').read_text();header=(root/'src/vehicles/Transmission.h').read_text()
for name in m.OPERATIONS:assert len(re.findall(r'^Transmission_'+name+r'\(',kernel,re.M))==1 and len(re.findall(r'\bTransmission_'+name+r'\(',header))==1,name
assert kernel.count('return Transmission_CalculateDriveAcceleration(self,gasPedal,gear,time,&fVelocity,false);')==2
assert '((*gear)-1)/(self->nNumberOfGears-1)' in kernel and '(*gasPedal)' in kernel
assert len(m.READERS)==6 and len(re.findall(r'\bHANDLING_\w+\s*=',flags))==28
reports={}
for name,count in [('matrix-runs',8),('sanitized-runs',2),('native-runs',4),('native-source-compiles',28),('production-source-compiles',28)]:
    rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==count and all(row['exit']==0 for row in rows),name
    for row in rows:
        if 'before' in row:
            assert row['before']==row['after']=='states=85741 checks=183520 hash=7fb764e7891f4dd8',row
            assert row['assertion']==('transmission assertion' if row['profile']=='normal' else 'assertion-disabled states=1 hash=e32cc0c7a2e543ac'),row
    reports[name]=len(rows)
layouts=json.loads((out/'native-layouts.json').read_text());assert len(layouts)==2
for row in layouts:
    data=dict(zip(row['expressions'],row['values']));assert data['sizeof(cTransmission)']==96 and data['alignof(cTransmission)']==4 and data['sizeof(tHandlingData)']==220 and data['offsetof(tHandlingData,Transmission)']==52
    assert data['sizeof(cVehicleParams)']=={32:28,64:40}[row['bits']]
configs=json.loads((out/'evaluated-configurations.json').read_text());assert len(configs)==48
for row in configs:
    # The six legacy Win32 RW34 configurations deliberately exclude the librw compatibility producer
    expectedSources=406 if row['platform']=='Win32' and 'RW34' in row['configuration'] else 407
    assert row['exit']==0 and row['pairs']==28 and row['c_units']==1 and row['sources']==expectedSources,row
reports['evaluated-configurations']=len(configs)
for configuration in ['debug','release']:
    row=json.loads((out/f'd3d9-{configuration}.json').read_text());assert row['exit']==0
    log=Path(row['log']).read_text(errors='replace');assert '0 Error(s)' in log and 'Build succeeded.' in log
    exe=Path(row['exe']);assert exe.exists() and exe.stat().st_size==row['size'];reports['d3d9-'+configuration]=row['size']
reports.update({'classes_removed':1,'public_C_operations':3,'native_primitives_added':0,'audio_bridges_removed':6,'changed_sources':len(changes),'new_headers':1,'renamed_producers':m.RENAMES,'states_per_profile':85741,'checks_per_profile':183520,'handling_flags':28,'transmission_bytes':96,'gear_bytes':12,'handling_bytes':220})
(out/'verification-summary.json').write_text(json.dumps(reports,indent=2),newline='\n')
print('Verified complete C transmission ownership, recursive/reference behavior, six audio bridge removals, native layouts and both full D3D9 links')
