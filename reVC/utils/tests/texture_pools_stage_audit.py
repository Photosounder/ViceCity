"""Audit complete C texture/palette ownership, real COM calls and whole-project integration."""
from pathlib import Path
import hashlib,json,re,sys,xml.etree.ElementTree as ET
import texture_pools_migration as m
from rect_migration import mask
from stats_successor import verify

root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/texture-pools-c-tests';saved=out/'before';production='--production' in sys.argv;tree=root if production else out/'overlay';changes=json.loads((out/'changes.json').read_text());assert len(changes)==3
for name in changes:
    # Preserve immutable source bytes until complete staged runtime and integration checks pass
    if not production:assert (root/name).read_bytes()==(saved/name).read_bytes(),name
    path=tree/m.RENAMES.get(name,name);value=path.read_text();assert value==m.calls((saved/name).read_text()) and m.calls(value)==value and b'\r' not in path.read_bytes(),name
for name,digest in json.loads((out/'untouched.json').read_text()).items():
    # Protect all project sources outside the exact complete owner and two public clients
    if production:verify(root,name,digest)
    else:assert hashlib.sha256((root/name).read_bytes()).hexdigest()==digest,name
kernel=(tree/'src/rw/TexturePools.c').read_text();header=(tree/'src/rw/TexturePools.h').read_text();assert kernel==m.kernel((saved/'src/rw/TexturePools.cpp').read_text()) and header==m.header();assert not (tree/'src/rw/TexturePools.cpp').exists()
for value in [kernel,header]:
    # Require all application records and operations to be genuine C without a C++ SDK bridge
    assert not re.search(r'::|\b(?:class|new|delete|namespace|static_cast)\b',mask(value))
for name in ['CTexturePool_'+name for _,name,_,_ in m.POOL]+['CPaletteList_'+name for _,name,_,_ in m.PALETTE]+['CTexturePool_GetSize',*m.GLOBALS.values()]:
    # Require all 17 original operations exactly once in the complete C producer
    assert len(re.findall(r'(?m)^'+name+r'\([^\n]*\)\n\{',kernel))==1,name
assert 'width != CTexturePool_GetSize(&aTexturePools[i])' in kernel
assert 'malloc(sizeof(int)*4*max)' in kernel and 'self->texturesNum = numTextures' not in kernel
assert 'IDirect3DDevice8_CreateTexture(_RwD3DDevice,width,height,levels,0,Format,D3DPOOL_MANAGED,texture)' in kernel
assert '#include <cita_windows.h>' in kernel and '#define CITA_REPORT_TO_STDERR' in kernel
assert 'CTexturePool aTexturePools[12];' in kernel and 'CPaletteList PaletteList;' in kernel
report={'status':'staged-verified','production_changed':production,'changed_sources':3,'complete_C_operations':17,'classes_removed':2,'new_native_bridges':0,'allocator':'CITAlloc','allocator_calls_per_run':896,'legacy_game_integration':'Existing USE_TEXTURE_POOL integration is disabled and requires unavailable custom RenderWare; active pool behavior is tested through real SDK COM declarations'}
for name,count in [('strict-compiles',56),('native-compiles',6),('native-runs',12),('native-layouts',3)]:
    # Require complete original/C producer checks and independently compiled C and C++ consumers
    rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==count and all(r['exit']==0 for r in rows),name
    if name=='native-runs':
        assert {(r['compiler'],r['bits'],r['optimization'],r['profile']) for r in rows}=={(a,b,c,d) for a,b in [('clang',32),('clang',64),('gcc',64)] for c in ['O0','O2'] for d in ['fixed','classic']}
        for row in rows:
            # Pin whole original traces including actual COM virtual calls and direct C vtable dispatch
            gold=({32:{'fixed':'f5ee9a05e910d2d1','classic':'38b7c5c894d10dbd'},64:{'fixed':'9efd440e6183810d','classic':'d981a89e7f1a3cf9'}}[row['bits']][row['profile']]);expected='commands=10560 checks=11601 events=4960 allocations=432/432 cita=896 textures=832 releases=832 creates=32 reentries=16 hash='+gold;assert row['before']==row['after']==expected,row
    if name=='native-layouts':
        for row in rows:
            # Require complete data-member layouts from both original classes and both new language consumers
            bits=row['bits'];expected=f"CTexturePool {28 if bits==32 else 32} {4 if bits==32 else 8} Format=0 size=4 levels=8 texturesMax=12 texturesUsed=16 texturesNum=20 pTextures=24\nCPaletteList {12 if bits==32 else 16} {4 if bits==32 else 8} Max=0 Num=4 Data=8\n";assert row['consumers']==['original-cpp','new-cpp','new-c'] and row['layout']==expected,row
    report[name]=count
report['remaining']=['Full Debug/Release integration and historical source projections','Production application and checks']
if '--integration' in sys.argv:
    # Require correct C11 metadata and distinct object names in every actual evaluated build configuration
    rows=json.loads((out/'evaluated-configurations.json').read_text());assert len(rows)==48
    for row in rows:assert row['exit']==0 and row['pairs']==28 and row['c_units']==1 and row['sources']==(426 if row['platform']=='Win32' and 'RW34' in row['configuration'] else 427),row
    project=out/'project/reVC.vcxproj';assert getattr(m,'texture_pools_project',m.project)(project.read_text())==project.read_text();ns={'m':'http://schemas.microsoft.com/developer/msbuild/2003'};items=[n.get('Include') for n in ET.parse(project).findall('.//m:ClCompile',ns) if n.get('Include')];assert len(items)==len(set(items))==427
    for item in items:
        # Require the full project to compile one coherent staged tree
        path=(project.parent/item.replace('\\','/')).resolve();assert path.is_relative_to((out/'overlay/src').resolve()) and path.exists(),path
    for configuration in ['debug','release']:
        # Inspect actual compiler records, object timestamps and complete executable links without running the game
        build=json.loads((out/f'd3d9-{configuration}.json').read_text());log=Path(build['log']).read_text(errors='replace');assert build['exit']==0 and 'Build succeeded.' in log and '0 Error(s)' in log;exe=Path(build['exe']);assert exe.is_file() and exe.stat().st_size==build['size'];objdir=out/'project/obj/win-amd64-librw_d3d9-oal'/configuration.capitalize();commands=(objdir/'reVC.tlog/CL.command.1.tlog').read_text(encoding='utf-16').splitlines();source=(out/'overlay/src/rw/TexturePools.c').resolve();index=commands.index('^'+str(source).upper());command=commands[index+1];assert str(source).upper() in command and '/TC ' in command and '/std:c11 ' in command;obj=objdir/'TexturePools.obj';assert obj.exists() and obj.stat().st_mtime>=source.stat().st_mtime and exe.stat().st_mtime>=obj.stat().st_mtime;report['d3d9-'+configuration]=build['size']
    rows=json.loads((out/'historical-source-projections.json').read_text());assert len(rows)==135 and all(r['exit']==0 for r in rows);report['history']=len(rows);report['status']='ready-to-apply';report['remaining']=['Production application and checks']
    if production:
        # Require every applied byte to match the complete candidate that passed both native links
        for path in (out/'overlay/src').rglob('*'):
            if path.is_file():verify(root,'src/'+path.relative_to(out/'overlay/src').as_posix(),hashlib.sha256(path.read_bytes()).hexdigest())
        for name,count in [('production-native-compiles',6),('production-history',135),('preceding-owner-audits',3)]:
            # Require full production and predecessor audits rather than prior summary reads
            rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==count and all(r['exit']==0 for r in rows),name;report[name]=count
        report['status']='applied-verified';report['remaining']=[]
(out/('verification-summary.json' if production else 'stage-summary.json')).write_text(json.dumps(report,indent=2),newline='\n');print(json.dumps(report,indent=2))
