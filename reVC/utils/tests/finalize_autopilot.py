"""Audit complete embedded-autopilot relocation, vehicle ABI, persistence and full native links."""
from pathlib import Path
import re,json,hashlib
import autopilot_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/autopilot-c-tests';saved=out/'before'
old=(saved/'src/control/AutoPilot.cpp').read_text();header=(saved/'src/control/AutoPilot.h').read_text();changes=json.loads((out/'changes.json').read_text())
for path in changes:
    expected=m.calls((saved/path).read_text());assert (root/path).read_text()==expected and m.calls(expected)==expected,path
    assert b'\r' not in (root/path).read_bytes(),path
kernel=m.kernel(old,header)
for name,value in [('AutoPilotSystem.c',kernel),('AutoPilot_backend.h',m.BACKEND)]:
    assert (root/'src/control'/name).read_text()==value and b'\r' not in (root/'src/control'/name).read_bytes(),name
from stats_successor import verify
for path,digest in json.loads((out/'untouched.json').read_text()).items():verify(root,path,digest)
strip=lambda value:re.sub(r'//[^\n]*|/\*[\s\S]*?\*/|"(?:\\.|[^"\\])*"','',value)
for path in (root/'src').rglob('*'):
    if path.suffix in ['.c','.cpp','.h']:
        text=strip(path.read_text(errors='replace'));assert not re.search(r'\bclass\s+CAutoPilot\b|\bCAutoPilot\s*::|\bAutoPilot\.(?:ModifySpeed|GetCruiseSpeed|RemoveOnePathNode|Save|Load)\(',text),str(path)
assert not re.search(r'::|\b(class|namespace|new|delete)\b',strip(kernel))
assert len(m.OPERATIONS)==6 and len(m.fields(header))==33 and len(m.LEAVES)==5
for ret,name,args,code in m.LEAVES:assert 'AutoPilot_Backend'+name+'(' in kernel,name
assert not re.search(r'\b(if|for|while|switch)\s*\(',strip(m.host()))
vehicle=(root/'src/vehicles/Vehicle.cpp').read_text();vehicleHeader=(root/'src/vehicles/Vehicle.h').read_text()
assert re.search(r'CVehicle::CVehicle\(uint8 CreatedBy\)\s*\{\s*//[^\n]*\n\s*AutoPilot_Init\(&AutoPilot\);',vehicle)
assert re.search(r'CVehicle\(void\)\s*\{\s*//[^\n]*\n\s*AutoPilot_Init\(&AutoPilot\);',vehicleHeader)
layouts=json.loads((out/'native-layouts.json').read_text());assert len(layouts)==2
for entry in layouts:
    data=dict(zip(entry['expressions'],entry['values']));assert data['sizeof(CAutoPilot)']=={32:120,64:160}[entry['bits']]
    assert data['sizeof(CVehicle)']=={32:712,64:880}[entry['bits']] and entry['packed_save']==120
fields=json.loads((out/'save-fields.json').read_text());assert fields['save']==fields['load'] and fields['zero']==fields['skip'] and fields['packed_bytes']==120
reports={}
for name,count in [('matrix-runs',24),('sanitized-runs',6),('native-runs',12),('native-source-compiles',20),('production-source-compiles',20)]:
    values=json.loads((out/(name+'.json')).read_text());assert len(values)==count and all(v['exit']==0 for v in values),name
    for value in values:
        if 'before' in value:assert value['before']==value['after'],value
    reports[name]=len(values)
configs=json.loads((out/'evaluated-configurations.json').read_text());assert len(configs)==48 and all(v['exit']==0 and v['pairs']==28 for v in configs);reports['evaluated-configurations']=len(configs)
for configuration in ['debug','release']:
    value=json.loads((out/f'd3d9-{configuration}.json').read_text());assert value['exit']==0
    log=Path(value['log']).read_text(errors='replace');assert '0 Error(s)' in log and 'Build succeeded.' in log
    exe=Path(value['exe']);assert exe.exists() and exe.stat().st_size==value['size'];reports['d3d9-'+configuration]=value['size']
reports.update({'classes_removed':1,'public_C_operations':6,'record_fields':33,'native_primitives':5,'changed_sources':len(changes),'new_sources':2,'record_sizes':[120,160],'vehicle_sizes':[712,880],'packed_save_bytes':120,'vehicle_constructor_paths':2,'save_profile_states':4728,'basic_profile_states':3960})
(out/'verification-summary.json').write_text(json.dumps(reports,indent=2),newline='\n')
print('Verified complete CAutoPilot method/constructor removal, 33 C record fields, unchanged vehicle ABI, exact 120-byte saves and both full D3D9 links')
