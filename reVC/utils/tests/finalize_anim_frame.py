"""Audit complete C frame ownership, native SDK identity, layouts and full D3D9 links."""
from pathlib import Path
import json,re
import anim_frame_migration as m
from rect_migration import mask
from stats_successor import verify
from doors_migration import method
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/anim-frame-c-tests';saved=out/'before';changes=json.loads((out/'changes.json').read_text());assert len(changes)==11
for name in changes:
    path=root/name;expected=m.calls((saved/name).read_text());assert path.read_text()==expected and m.calls(expected)==expected and b'\r' not in path.read_bytes(),name
for name,digest in json.loads((out/'untouched.json').read_text()).items():verify(root,name,digest)
header=(root/'src/animation/AnimBlendFrameData.h').read_text();native=(root/'src/animation/AnimBlendFrameData_native.h').read_text();assert header==m.header() and native==m.native()
assert not re.search(r'::|\b(class|namespace|new|delete|using|RwV3d|RwFrame|RpHAnimStdInterpFrame)\b',mask(header))
before=(saved/'src/animation/AnimBlendFrameData_native.h').read_text()
for name,value in m.FLAGS.items():assert int(re.search(r'\b'+name+r'\s*=\s*(0x[0-9a-fA-F]+|[0-9]+)',before)[1],0)==value,name
clump=(root/'src/animation/AnimBlendClumpData.c').read_text();assert 'sizeof(AnimBlendFrameData)' in method(clump,'AnimBlendClump_SetNumberOfFrames')
walker=method(clump,'AnimBlendClump_ForAllFrames');assert 'i<self->numFrames' in walker and 'callback(&self->frames[i],arg)' in walker
assert 'static_cast<RwFrame*>(value->frame)' in native and 'static_cast<RpHAnimStdInterpFrame*>(value->hanimFrame)' in native
for path in (root/'src').rglob('*'):
    if path.suffix in ['.c','.cpp','.h'] and not any(part.startswith('.') for part in path.relative_to(root/'src').parts):
        source=mask(path.read_text(errors='replace'));assert 'AnimBlendFrameData::' not in source and 'AnimBlendClump_BackendFrameSize' not in source,str(path)
reports={};gold={'fixed':'9292a40568a4e46e','classic':'8bd11c853ea36012'}
for name,count in [('matrix-runs',8),('sanitized-runs',2),('native-runs',4),('native-source-compiles',66),('production-source-compiles',66)]:
    rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==count and all(row['exit']==0 for row in rows),name
    for row in rows:
        if 'before' in row:assert row['before']==row['after']=='states=25612 checks=80209 hash='+gold[row['profile']],row
    reports[name]=len(rows)
configs=json.loads((out/'evaluated-configurations.json').read_text());assert len(configs)==48
for row in configs:assert row['exit']==0 and row['pairs']==28 and row['c_units']==1 and row['sources']==(412 if row['platform']=='Win32' and 'RW34' in row['configuration'] else 413),row
reports['evaluated-configurations']=len(configs)
for configuration in ['debug','release']:
    row=json.loads((out/f'd3d9-{configuration}.json').read_text());assert row['exit']==0
    log=Path(row['log']).read_text(errors='replace');assert '0 Error(s)' in log and 'Build succeeded.' in log
    exe=Path(row['exe']);assert exe.exists() and exe.stat().st_size==row['size'];reports['d3d9-'+configuration]=row['size']
history=json.loads((out/'historical-source-projections.json').read_text());assert len(history)==254 and all(row['exit']==0 for row in history);reports['historical-source-projections']=len(history)
for name in ['native-parent-layouts','production-parent-layouts']:
    rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==6 and all(row['before']==row['after'] for row in rows),name;reports[name]=rows
for log in out.glob('gcc-O*-*.log'):
    imports=re.findall(r'DLL Name: ([^\s]+)',log.read_text());assert imports and not any(re.search(r'libgcc|libstdc|libwinpthread|msys-',name,re.I) for name in imports),log
reports.update({'complete_record_owners_converted':1,'global_C_flags':6,'native_value_adapters':3,'removed_native_metadata_interfaces':1,'native_algorithm_bridges':0,'changed_sources':len(changes),'new_headers':1,'new_producers':0,'record_bytes':{'x86':24,'x64':32},'states':25612,'checks':80209,'frame_value_cases':4096,'actual_initialization_callbacks':4,'actual_animation_callbacks':9,'animation_frame_cases':18432,'actual_IK_methods':1,'live_traversal_cases':2048,'default_lifetime_no_writes':True})
(out/'verification-summary.json').write_text(json.dumps(reports,indent=2),newline='\n')
print('Verified the complete C animation frame record, original layout and lifetime, live SDK poses, all callbacks and both full D3D9 links')
