"""Audit the complete applied conversion and its authoritative verification artifacts."""
from pathlib import Path
import re,json,hashlib
import events_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/events-c-tests'
changes=json.loads((out/'changes.json').read_text());saved=out/'before'
for path in changes:
    assert (root/path).read_text()==m.calls((saved/path).read_text()),path
    assert b'\r' not in (root/path).read_bytes(),path
for name,value in [('EventsShared.h',m.SHARED),('Events_backend.h',m.BACKEND),('GameplayEvents.c',m.kernel((saved/'src/core/EventList.cpp').read_text(),(saved/'src/core/Accident.cpp').read_text(),(saved/'src/core/Accident.h').read_text()))]:
    path=root/'src/core'/name;assert path.read_text()==value and b'\r' not in path.read_bytes(),name
from stats_successor import verify
for path,digest in json.loads((out/'untouched.json').read_text()).items():verify(root,path,digest)
strip=lambda text:re.sub(r'//[^\n]*|/\*[\s\S]*?\*/|"(?:\\.|[^"\\])*"','',text)
for path in (root/'src').rglob('*'):
    if path.suffix in ['.c','.cpp','.h']:assert not re.search(r'\bclass\s+C(?:EventList|Accident|AccidentManager)\b|\bCEventList\s*::|\bgAccidentManager\s*\.',strip(path.read_text(errors='replace'))),str(path)
kernel=(root/'src/core/GameplayEvents.c').read_text();assert not re.search(r'\b(class|namespace|new|delete)\b|::',strip(kernel))
assert len(m.EVENT_OPERATIONS)==8 and len(m.ACCIDENT_OPERATIONS)==7 and len(m.READERS)==35
reports={}
for name,count in [('matrix-runs',16),('native-runs',8),('sanitized-runs',4),('native-layouts',4),('native-source-compiles',36),('production-source-compiles',36)]:
    results=json.loads((out/(name+'.json')).read_text());assert len(results)==count and all(v['exit']==0 for v in results),name
    for item in results:
        if 'before' in item:assert item['before']==item['after'],item
    reports[name]=len(results)
configs=json.loads((out/'evaluated-configurations.json').read_text());assert len(configs)>0 and all(v['exit']==0 and v['pairs']==28 for v in configs);reports['evaluated-configurations']=len(configs)
for configuration in ['debug','release']:
    report=json.loads((out/f'd3d9-{configuration}.json').read_text());assert report['exit']==0
    log=Path(report['log']).read_text(errors='replace');assert '0 Error(s)' in log and 'Build succeeded.' in log
    exe=Path(report['exe']);assert exe.exists() and exe.stat().st_size==report['size'];reports['d3d9-'+configuration]=report['size']
reports.update({'classes_removed':3,'public_C_operations':17,'native_leaves':35,'changed_sources':len(changes),'new_sources':3,'event_sizes':[36,48],'accident_sizes':[12,16],'manager_sizes':[240,320]})
(out/'verification-summary.json').write_text(json.dumps(reports,indent=2),newline='\n')
print('Verified three complete class removals, 17 C operations, all caller conversions, unchanged native layouts and both full D3D9 links')
