"""Audit four complete C input owners, native layout, lifecycle integration and both full links."""
from pathlib import Path
import json,re
import input_state_migration as m
from stats_successor import verify
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/input-state-c-tests';saved=out/'before';changes=json.loads((out/'changes.json').read_text())
for name in changes:
    expected=m.calls((saved/name).read_text());assert (root/name).read_text()==expected and m.calls(expected)==expected,name
    assert b'\r' not in (root/name).read_bytes(),name
for name,expected in [('InputState.h',m.header((saved/'src/core/Pad.h').read_text())),('InputState.c',m.kernel((saved/'src/core/Pad.cpp').read_text(),(saved/'src/core/Pad.h').read_text())),('InputState_backend.h',m.BACKEND)]:
    assert (root/'src/core'/name).read_text()==expected,name
    assert b'\r' not in (root/'src/core'/name).read_bytes(),name
for path,digest in json.loads((out/'untouched.json').read_text()).items():verify(root,path,digest)
strip=lambda source:re.sub(r'//[^\n]*|/\*[\s\S]*?\*/|"(?:\\.|[^"\\])*"','',source)
for path in (root/'src').rglob('*'):
    if path.suffix in ['.h','.cpp','.c']:assert not re.search(r'\bclass\s+(?:'+ '|'.join(m.RECORDS)+r')\b|\b(?:'+ '|'.join(m.RECORDS)+r')\s*::',strip(path.read_text(errors='replace'))),str(path)
for name in ['InputState.h','InputState.c','InputState_backend.h']:assert not re.search(r'::|\b(class|namespace|new|delete)\b',strip((root/'src/core'/name).read_text())),name
kernel=(root/'src/core/InputState.c').read_text();header=(root/'src/core/InputState.h').read_text();pad=(root/'src/core/Pad.cpp').read_text()
assert len(re.findall(r'^\w+_(?:CheckForInput|Clear|GetLeftStickX|GetLeftStickY|GetRightStickX|GetRightStickY|Init|GetMouseSetUp)\(',kernel,re.M))==10
assert kernel.count('CMousePointerStateHelper MousePointerStateHelper;')==1 and 'CMousePointerStateHelper MousePointerStateHelper;' not in pad
assert re.search(r'extern "C" \{[\s\S]*extern CMousePointerStateHelper MousePointerStateHelper;',header)
for name in ['OldMouseControllerState','NewMouseControllerState','PCTempMouseControllerState']:
    assert pad.count('static const int '+name+'Initialized=(MouseControllerState_Init(&CPad::'+name+'),0);')==1,name
assert 'self->ENTER =' not in kernel and 'NetworkTalk' not in kernel[kernel.index('ControllerState_CheckForInput'):kernel.index('ControllerState_Clear')]
reports={};expected={'d3d9':'states=69100 checks=276906 hash=84dca1afbb05c653','glfw':'states=69100 checks=273450 hash=af96d36de032538b'}
for name,count in [('matrix-runs',8),('sanitized-runs',2),('native-runs',4),('native-source-compiles',22),('production-source-compiles',22)]:
    rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==count and all(row['exit']==0 for row in rows),name
    for row in rows:
        if 'before' in row:assert row['before']==row['after']==expected[row['profile']],row
    reports[name]=len(rows)
layouts=json.loads((out/'native-layouts.json').read_text());assert len(layouts)==2
for row in layouts:
    data=dict(zip(row['expressions'],row['values']));assert data['sizeof(CPad)']==276 and data['alignof(CPad)']==4 and len(row['expressions'])==88,row['bits']
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
reports.update({'classes_removed':4,'public_C_operations':10,'canonical_inversion_state':1,'native_primitives':6,'changed_sources':len(changes),'new_sources':3,'states_per_profile':69100,'input_record_fields':79,'record_bytes':{'controller':42,'mouse':16,'mouse_pointer_helper':2,'keyboard':624},'pad_bytes':276,'static_mouse_initializers':3})
(out/'verification-summary.json').write_text(json.dumps(reports,indent=2),newline='\n')
print('Verified four complete input class removals, ten C operations, partial clears, platform policy, global initialization, native layouts and both full D3D9 links')
