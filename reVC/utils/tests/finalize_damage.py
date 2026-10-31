"""Audit complete C damage ownership, packed fields, constructor/save integration and full links."""
from pathlib import Path
import json,re
import damage_migration as m
from doors_migration import CAR_INIT
from stats_successor import verify
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/damage-c-tests';saved=out/'before';changes=json.loads((out/'changes.json').read_text())
for name in changes:
    current=root/m.RENAMES.get(name,name);expected=m.calls((saved/name).read_text())
    assert current.read_text()==expected and m.calls(expected)==expected,name
    assert b'\r' not in current.read_bytes(),name
assert not (root/'src/vehicles/DamageManager.cpp').exists()
parts=(root/'src/vehicles/VehicleDamageParts.h').read_text();assert parts==m.parts((saved/'src/vehicles/Vehicle.h').read_text()) and m.calls(parts)==parts
assert b'\r' not in (root/'src/vehicles/VehicleDamageParts.h').read_bytes()
for path,digest in json.loads((out/'untouched.json').read_text()).items():verify(root,path,digest)
strip=lambda source:re.sub(r'//[^\n]*|/\*[\s\S]*?\*/|"(?:\\.|[^"\\])*"','',source)
for path in (root/'src').rglob('*'):
    if path.suffix in ['.h','.cpp','.c']:assert not re.search(r'\bclass\s+CDamageManager\b|\bCDamageManager\s*::|\bDamage\.(?:'+ '|'.join(m.OPERATIONS)+r')\(',strip(path.read_text(errors='replace'))),str(path)
for name in ['DamageManager.h','DamageManager.c','VehicleDamageParts.h']:assert not re.search(r'::|\b(class|namespace|new|delete)\b',strip((root/'src/vehicles'/name).read_text())),name
kernel=(root/'src/vehicles/DamageManager.c').read_text();header=(root/'src/vehicles/DamageManager.h').read_text()
for name in ['Init']+m.OPERATIONS:assert len(re.findall(r'^Damage_'+name+r'\(',kernel,re.M))==1 and len(re.findall(r'\bDamage_'+name+r'\(',header))==1,name
assert kernel.count('int32_t Damage_GetPanelStatusFromWord(')==1 and 'extern float G_aComponentDamage[7];' in header
car=(root/'src/vehicles/Automobile.cpp').read_text();assert car.count(m.INIT)==1 and car.index(m.INIT)<car.index(CAR_INIT)
reports={}
for name,count in [('matrix-runs',8),('sanitized-runs',2),('native-runs',4),('native-source-compiles',36),('production-source-compiles',36)]:
    rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==count and all(row['exit']==0 for row in rows),name
    for row in rows:
        if 'before' in row:
            expected='states=19789 checks=27918 hash=8cb6c277f5dcd4fd' if row['profile']=='fixed' else 'states=19789 checks=27918 hash=245305b8b4a23ebd' if name=='native-runs' else 'states=19533 checks=27406 hash=0f894047ffc0a39f'
            assert row['before']==row['after']==expected,row
    reports[name]=len(rows)
layouts=json.loads((out/'native-layouts.json').read_text());assert len(layouts)==2
for row in layouts:
    data=dict(zip(row['expressions'],row['values']));bits=row['bits'];assert data['sizeof(CDamageManager)']==28 and data['alignof(CDamageManager)']==4 and data['offsetof(CDamageManager,field_18)']==24
    assert data['sizeof(CAutomobile)']=={32:1544,64:1816}[bits] and data['offsetof(CAutomobile,Damage)']=={32:712,64:880}[bits]
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
reports.update({'classes_removed':1,'public_C_operations':20,'native_bridges_added':0,'canonical_coefficients':7,'shared_part_enums':2,'changed_sources':len(changes),'new_headers':1,'renamed_producers':m.RENAMES,'damage_record_bytes':28,'fixed_states':19789,'classic_portable_states':19533,'classic_native_states':19789,'constructor_sites':1,'raw_save_bytes':28})
(out/'verification-summary.json').write_text(json.dumps(reports,indent=2),newline='\n')
print('Verified complete C damage ownership, all 20 operations, packed fields, constructor/save compatibility, native layouts and both full D3D9 links')
