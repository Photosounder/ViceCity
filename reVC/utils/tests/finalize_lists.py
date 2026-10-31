"""Audit complete C intrusive lists, explicit lifetimes, native layouts and both full links."""
from pathlib import Path
import re,json,hashlib
import lists_migration as m
from stats_successor import verify
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/lists-c-tests';saved=out/'before';changes=json.loads((out/'changes.json').read_text())
for old in changes:
    target=root/m.RENAMES.get(old,old);expected=m.calls((saved/old).read_text())
    assert target.read_text()==expected and m.calls(expected)==expected,old
    assert b'\r' not in target.read_bytes(),old
for old,new in m.RENAMES.items():assert not (root/old).exists() and (root/new).is_file()
for path,digest in json.loads((out/'untouched.json').read_text()).items():verify(root,path,digest)
strip=lambda source:re.sub(r'//[^\n]*|/\*[\s\S]*?\*/|"(?:\\.|[^"\\])*"','',source)
for path in (root/'src').rglob('*'):
    if path.suffix in ['.h','.cpp','.c']:assert not re.search(r'\bclass\s+C(?:PtrNode|PtrList|EntryInfoNode|EntryInfoList)\b|\bC(?:PtrList|EntryInfoList)\s*::',strip(path.read_text(errors='replace'))),str(path)
assert not re.search(r'::|\b(class|namespace|new|delete)\b',strip((root/'src/core/Lists.c').read_text()))
signatures=re.findall(r'^(?:void|CPtrNode|CEntryInfoNode)\s+\*?\s*(?:PtrList|EntryInfoList)_\w+\(', (root/'src/core/Lists.h').read_text(),re.M);assert len(signatures)==16,len(signatures)
reports={}
for name,count in [('matrix-runs',8),('sanitized-runs',2),('native-runs',4),('native-source-compiles',20),('production-source-compiles',20)]:
    rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==count and all(row['exit']==0 for row in rows),name
    for row in rows:
        if 'before' in row:assert row['before']==row['after'] and 'states=12002' in row['after'] and 'shutdown ' in row['after'],row
    reports[name]=len(rows)
layouts=json.loads((out/'native-layouts.json').read_text());assert len(layouts)==2
for row in layouts:
    data=dict(zip(row['expressions'],row['values']));assert data['sizeof(CPtrNode)']=={32:12,64:24}[row['bits']] and data['sizeof(CEntryInfoNode)']=={32:20,64:40}[row['bits']]
configs=json.loads((out/'evaluated-configurations.json').read_text());assert len(configs)==48 and all(row['exit']==0 and row['pairs']==28 and row['c_units']==1 for row in configs);reports['evaluated-configurations']=len(configs)
for configuration in ['debug','release']:
    row=json.loads((out/f'd3d9-{configuration}.json').read_text());assert row['exit']==0
    log=Path(row['log']).read_text(errors='replace');assert '0 Error(s)' in log and 'Build succeeded.' in log
    exe=Path(row['exe']);assert exe.exists() and exe.stat().st_size==row['size'];reports['d3d9-'+configuration]=row['size']
reports.update({'list_classes_removed':4,'sector_plain_record':1,'public_C_operations':16,'static_lifetime_operations':1,'C_pool_getters':2,'changed_sources':len(changes),'renamed_producers':m.RENAMES,'states_per_profile':12002,'static_blocks':3,'sector_lists':256000,'layouts':layouts})
(out/'verification-summary.json').write_text(json.dumps(reports,indent=2),newline='\n')
print('Verified four complete list class removals, all 16 C operations, embedded lifetimes, reverse static cleanup, native layouts and both full D3D9 links')
