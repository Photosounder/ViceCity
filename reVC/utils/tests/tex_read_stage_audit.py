"""Audit complete C texture policy, runtime behavior, all callers and integration evidence."""
from pathlib import Path
import hashlib,json,re,sys,xml.etree.ElementTree as ET
import tex_read_migration as m
from tex_read_schema import HOST,NAMES
from rect_migration import mask
from stats_successor import verify

root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/tex-read-c-tests';saved=out/'before';production='--production' in sys.argv;tree=root if production else out/'overlay'
changes=json.loads((out/'changes.json').read_text())
assert len(changes)==9
for name in changes:
    # Keep production unchanged and require each complete deterministic source projection
    if not production:assert (root/name).read_bytes()==(saved/name).read_bytes(),name
    path=tree/m.RENAMES.get(name,name);value=path.read_text()
    assert value==m.calls((saved/name).read_text()) and m.calls(value)==value,name
    assert b'\r' not in path.read_bytes(),name
for name,digest in json.loads((out/'untouched.json').read_text()).items():
    # Protect all source outside the exact texture owner and shared constant declarations
    if production:verify(root,name,digest)
    else:assert hashlib.sha256((root/name).read_bytes()).hexdigest()==digest,name
kernel=(tree/'src/rw/TexRead.c').read_text();header=(tree/'src/rw/TexRead.h').read_text();native=(tree/'src/rw/TexRead_native.cpp').read_text()
assert kernel==m.kernel((saved/'src/rw/TexRead.cpp').read_text())
assert header==m.header() and native==m.native() and (tree/'src/rw/TexRead_backend.h').read_text()==m.backend_header()
assert not (tree/'src/rw/TexRead.cpp').exists()
for name,original,kind in [('src/core/CdStream_constants.h','src/core/CdStream.h','cd'),('src/skel/crossplatform_constants.h','src/skel/crossplatform.h','os'),('src/core/Streaming_constants.h','src/core/Streaming.h','stream')]:
    # Preserve exact constant values and spellings instead of supplying invented SDK mappings
    value=(tree/name).read_text();assert value==m.constants((saved/original).read_text(),kind) and m.calls(value)==value,name
for value in [kernel,header,m.backend_header()]:
    # Require genuine C syntax without native SDK types or class operations in the complete policy owner
    assert not re.search(r'::|\b(?:class|new|delete|namespace|static_cast|RwStream|RwTexture|RwTexDictionary|CSprite2d|CRGBA)\b',mask(value))
counts={name:len(re.findall(r'(?m)^TexRead_'+name+r'\([^\n]*\)\n\{',kernel)) for name in NAMES.values()}
assert sum(counts.values())==16 and all(count==2 if name in ['ReadVideoCardCapsFile','CheckVideoCardCaps','WriteVideoCardCapsFile'] else count==1 for name,count in counts.items()),counts
assert 'static int32_t numberTextures = -1;' in kernel and 'static int32_t streamPosition;' in kernel
assert 'TexRead_ReadVideoCardCapsFile(uint32_t *cap32' in kernel and 'TexRead_ReadVideoCardCapsFile(&fcap32' in kernel
assert 'TexRead_HostSetVsync(vsyncState)' in kernel and 'TexRead_HostSetFrameLimiter(frameLimiterState)' in kernel
assert 'TexRead_HostForAllTextures(texDict, TexRead_DestroyTexture, NULL)' in kernel
assert 'const void * TexRead_HostWriteDictionary' in native
bindings=(tree/'src/rw/TexRead_bindings.c').read_text();assert bindings==m.bindings((saved/'src/rw/TxdStore_native.cpp').read_text()) and m.calls(bindings)==bindings
assert len(m.forwarder_definitions((saved/'src/rw/TxdStore_native.cpp').read_text()))==3
for path in (tree/'src').rglob('*.cpp'):
    # Require every whole native consumer to stop calling removed C++ texture reader functions
    assert not re.search(r'\b(?:RwTextureGtaStreamRead|RwTexDictionaryGtaStreamRead[12]?)\s*\(',mask(path.read_text())),path
report={'status':'staged-verified','production_changed':production,'changed_sources':9,'C_forwarding_ABIs':3,'original_function_definitions_across_backends':16,'native_external_primitives':len(HOST)+2,'typed_SDK_callback_trampolines':1,'shared_constant_headers':3}
for name,count in [('strict-compiles',56),('native-owner-compiles',16),('native-consumer-compiles',14),('native-runs',20),('native-layouts',2)]:
    # Require complete original/C producer compilation rather than only generated declarations
    rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==count and all(row['exit']==0 for row in rows),name
    if name=='native-runs':
        assert {(r['bits'],r['optimization'],r['profile']) for r in rows}=={(a,b,c) for a in [32,64] for b in ['Od','O2'] for c in ['librw','classic','gl3','legacy','master']}
        gold32={'Od':{'librw':'e521e7ee012b8d28','classic':'f19a615d5861b591','gl3':'09dba80edcaba3d6','legacy':'dc0b5e06ae29be18','master':'e521e7ee012b8d28'},'O2':{'librw':'041224b2a5b5864e','classic':'f19a615d5861b591','gl3':'d493940b6dd2d92c','legacy':'3aa47e2a6cc4975e','master':'041224b2a5b5864e'}}
        gold64={'librw':'ee4e3ab03fd904c0','classic':'6be0418d77e91ec1','gl3':'0a11758ce93b4ec6','legacy':'732f231acdbeaab8','master':'ee4e3ab03fd904c0'}
        for row in rows:
            profile=row['profile'];classic=profile=='classic';legacy=profile=='legacy'
            checks=34679 if classic else 34804 if legacy else 34759;events=105614 if classic else 105906 if legacy else 105891;dictionaries=45 if classic else 41
            expected=f'commands=111 checks={checks} events={events} textures=107 dictionaries={dictionaries} destroyed=107/{dictionaries} visits=5 allocations=18 frees=18 hash='+(gold32[row['optimization']][profile] if row['bits']==32 else gold64[profile])
            assert row['before']==row['after']==expected,row
    if name=='native-layouts':
        assert {r['bits'] for r in rows}=={32,64}
        assert all(r['layout']=='GPUcaps 16 4 version=0 platform=4 subplatform=8 dxtSupport=12\n' and r['consumers']==['original-cpp','new-cpp','new-c'] for r in rows)
    report[name]=count
assert not json.loads((out/'strict-existing-limitations.json').read_text())
report['remaining']=['Full Debug/Release integration and historical source projections','Production application and checks']
if '--integration' in sys.argv:
    # Require complete evaluated source metadata and actual compiler records for both executable builds
    rows=json.loads((out/'evaluated-configurations.json').read_text());assert len(rows)==48
    for row in rows:
        assert row['exit']==0 and row['pairs']==28 and row['c_units']==2,row
        assert row['sources']==(424 if row['platform']=='Win32' and 'RW34' in row['configuration'] else 425),row
    project=out/'project/reVC.vcxproj';assert getattr(m,'tex_read_project',m.project)(project.read_text())==project.read_text()
    ns={'m':'http://schemas.microsoft.com/developer/msbuild/2003'}
    items=[node.get('Include') for node in ET.parse(project).findall('.//m:ClCompile',ns) if node.get('Include')]
    assert len(items)==len(set(items))==425
    for item in items:
        path=(project.parent/item.replace('\\','/')).resolve()
        assert path.is_relative_to((out/'overlay/src').resolve()) and path.exists(),path
    for configuration in ['debug','release']:
        build=json.loads((out/f'd3d9-{configuration}.json').read_text());log=Path(build['log']).read_text(errors='replace')
        assert build['exit']==0 and 'Build succeeded.' in log and '0 Error(s)' in log
        exe=Path(build['exe']);assert exe.is_file() and exe.stat().st_size==build['size']
        objdir=out/'project/obj/win-amd64-librw_d3d9-oal'/configuration.capitalize()
        commands=(objdir/'reVC.tlog/CL.command.1.tlog').read_text(encoding='utf-16').splitlines()
        for name in ['TexRead.c','TexRead_bindings.c','TexRead_native.cpp']:
            # Inspect actual C11 compiler tracking records and the linked object timestamps
            source=(out/'overlay/src/rw'/name).resolve();index=commands.index('^'+str(source).upper());command=commands[index+1]
            assert str(source).upper() in command
            if name.endswith('.c'):assert '/TC ' in command and '/std:c11 ' in command
            obj=objdir/(source.stem+'.obj');assert obj.exists() and obj.stat().st_mtime>=source.stat().st_mtime
            assert exe.stat().st_mtime>=obj.stat().st_mtime
        report['d3d9-'+configuration]=build['size']
    history=json.loads((out/'historical-source-projections.json').read_text())
    assert len(history)==378 and all(row['exit']==0 for row in history)
    report['history']=len(history);report['status']='ready-to-apply';report['remaining']=['Production application and checks']
    if production:
        # Require every applied source byte to match the complete candidate used for the two links
        for path in (out/'overlay/src').rglob('*'):
            if path.is_file():verify(root,'src/'+path.relative_to(out/'overlay/src').as_posix(),hashlib.sha256(path.read_bytes()).hexdigest())
        for name,count in [('production-consumer-compiles',14),('production-owner-compiles',16),('production-history',378),('preceding-owner-audits',3)]:
            rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==count and all(row['exit']==0 for row in rows),name
            report[name]=count
        report['status']='applied-verified';report['remaining']=[]
(out/('verification-summary.json' if production else 'stage-summary.json')).write_text(json.dumps(report,indent=2),newline='\n')
print(json.dumps(report,indent=2))
