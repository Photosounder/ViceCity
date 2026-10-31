"""Probe actual original/C records and compile the complete wanted caller projection."""
from pathlib import Path
import os,subprocess,json,shutil,sys,ctypes,re
import wanted_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/wanted-c-tests';overlay=out/'native-overlay';production='--production' in sys.argv
probeDir=out/'native-probes';probeDir.mkdir(exist_ok=True)
ctypes.windll.kernel32.SetErrorMode(0x8003)
shutil.copytree(root/'src',overlay/'src',dirs_exist_ok=True)
changes=json.loads((out/'changes.json').read_text())
for path in changes:(overlay/path).write_text(m.calls((out/'before'/path).read_text()),newline='\n')
for name in ['WantedSystem.c','WantedShared.h','Wanted_backend.h']:(overlay/'src/core'/name).write_text((out/'preview'/name).read_text(),newline='\n')
dirs=[root/'src']+[p for p in (root/'src').rglob('*') if p.is_dir()]
deps=[root/'vendor/librw',root/'vendor/openal-soft/include',root/'vendor/libsndfile/include',root/'vendor/mpg123/include',Path('C:/msys/home'),Path('C:/msys/home/github/CITAlloc'),root/'.diagnostics/events-c-tests/native-includes']
probes=[]
for phase,base in [('before',root),('after',root if production else overlay)]:
    text='#include "'+(base/'src/core/common.h').as_posix()+'"\n'
    if phase=='before':text+=(out/'before/src/core/Crime.h').read_text().replace('#pragma once','')+'\n'+(out/'before/src/core/Wanted.h').read_text().replace('#pragma once','').replace('#include "Crime.h"','')+'\n'
    else:text+='#include "'+(base/'src/core/Wanted.h').as_posix()+'"\n'
    text+='#include <type_traits>\n#include <memory>\n#include <stddef.h>\n#include <stdio.h>\n'
    text+='''int main()
{
    // Observe actual complete representations and the exact original bit positions
    printf("wanted=%u align=%u crime=%u crimealign=%u crimes=%u cops=%u flags=",(unsigned)sizeof(CWanted),(unsigned)alignof(CWanted),(unsigned)sizeof(CCrimeBeingQd),(unsigned)alignof(CCrimeBeingQd),(unsigned)offsetof(CWanted,m_aCrimes),(unsigned)offsetof(CWanted,m_pCops));
    CWanted local;
'''
    for name in ['m_bIgnoredByCops','m_bIgnoredByEveryone','m_bSwatRequired','m_bFbiRequired','m_bArmyRequired']:text+='    memset(&local,0,sizeof(local));local.'+name+'=1;printf("%02x",((unsigned char*)&local)[30]);\n'
    text+='    printf("\\n");return 0;\n}\n'
    if phase=='before':text+='static_assert(!std::is_trivially_default_constructible<CWanted>::value&&!std::is_trivially_destructible<CWanted>::value,"Original queued-crime constructor/destructor lifecycle");\n'
    else:text+='static_assert(std::is_trivial<CWanted>::value&&std::is_trivial<CCrimeBeingQd>::value,"Plain C records with explicit initialization and copy");\n'
    text+='static_assert(sizeof(CCrimeBeingQd)==28&&offsetof(CCrimeBeingQd,m_nType)==0&&offsetof(CCrimeBeingQd,m_nId)==4&&offsetof(CCrimeBeingQd,m_nTime)==8&&offsetof(CCrimeBeingQd,m_vecPosn)==12&&offsetof(CCrimeBeingQd,m_bReported)==24&&offsetof(CCrimeBeingQd,m_bPoliceDoesntCare)==25,"Every queued-crime field offset");\n'
    text+='''extern "C" int cmp_double(const double *,const double *)
{
    // Reject unused interpreter-table hooks brought in by the complete native project headers
    abort();
}
extern "C" int cmp_qq(const ddouble_t *,const ddouble_t *)
{
    // Reject unused interpreter-table hooks brought in by the complete native project headers
    abort();
}
extern "C" ddouble_t ddouble(double)
{
    // Reject unused interpreter-table hooks brought in by the complete native project headers
    abort();
}
extern "C" ddouble_t string_to_ddouble(const char *,char **)
{
    // Reject unused interpreter-table hooks brought in by the complete native project headers
    abort();
}
'''
    for field,offset in [('m_nChaos',0),('m_nMinChaos',4),('m_nLastUpdateTime',8),('m_nLastWantedLevelChange',12),('m_nLastTimeSuspended',16),('m_fCrimeSensitivity',20),('m_CurrentCops',24),('m_MaxCops',25),('m_MaximumLawEnforcerVehicles',26),('m_CopsBeatingSuspect',27),('m_RoadblockDensity',28),('m_nWantedLevel',32),('m_nMinWantedLevel',36),('m_aCrimes',40),('m_pCops',488)]:text+='static_assert(offsetof(CWanted,'+field+')=='+str(offset)+',"Original field offset '+field+'");\n'
    (probeDir/('layout-'+phase+'.cpp')).write_text(text,newline='\n')
results=[];layouts=[]
for bits in [32,64]:
    vcvars=Path('C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Auxiliary/Build')/f'vcvars{bits}.bat'
    options='/nologo /O2 /Gy /Gw /EHsc /MT /std:c++14 /Zc:sizedDealloc- /DLIBRW /DRW_D3D9 /DAUDIO_OAL /DNOMINMAX '
    native=options+' '.join('/I"'+str(p)+'"' for p in dirs+deps)
    translated=options+' '.join('/I"'+str(p)+'"' for p in [overlay/p.relative_to(root) for p in dirs]+deps)
    units=[('layout-before',probeDir/'layout-before.cpp',native,True),('layout-after',probeDir/'layout-after.cpp',translated,True)]
    if '--layouts-only' not in sys.argv:
        current=root if production else overlay;flags=native if production else translated
        units += [(Path(path).stem,current/path,flags if path.endswith('.cpp') else flags.replace('/std:c++14','/std:c11 /TC').replace('/EHsc',''),False) for path in changes if path.endswith(('.cpp','.c'))]
        units += [('WantedSystem',current/'src/core/WantedSystem.c',flags.replace('/std:c++14','/std:c11 /TC').replace('/EHsc',''),False)]
    if production:units[1]=(units[1][0],units[1][1],native,units[1][3])
    observed=[]
    for name,source,flags,execute in units:
        prefix=f'{"production" if production else "native"}{bits}-{name}';obj=out/(prefix+'.obj');exe=out/(prefix+'.exe');driver=out/(prefix+'.cmd')
        line='cl '+flags+' /Fo"'+str(obj)+'" '+(' /Fe"'+str(exe)+'" ' if execute else ' /c ')+'"'+str(source)+'"'
        if execute:line+=' /link /OPT:REF /LIBPATH:"C:/msys/home/github/quakespasm/Windows/SDL2/'+('lib64' if bits==64 else 'lib')+'"'
        driver.write_text('@echo off\ncall "'+str(vcvars)+'" >nul\nif errorlevel 1 exit /b 1\n'+line+'\nif errorlevel 1 exit /b 1\n'+('"'+str(exe)+'"\n' if execute else '')+'exit /b %errorlevel%\n',newline='\n')
        run=subprocess.run(['cmd.exe','/c',str(driver)],cwd=root,env=dict(os.environ),capture_output=True,text=True);log=out/(prefix+'.log');log.write_text(run.stdout+run.stderr,newline='\n')
        assert run.returncode==0,(prefix,str(log),run.stdout[-3500:])
        if execute:observed.append(next(line for line in run.stdout.splitlines() if line.startswith('wanted=')))
        results.append({'bits':bits,'unit':name,'exit':0});print(bits,name,'passed',flush=True)
    assert observed[0]==observed[1],observed
    layouts.append({'bits':bits,'original':observed[0],'converted':observed[1],'exit':0});print(observed[0],flush=True)
(out/'native-layouts.json').write_text(json.dumps(layouts,indent=2),newline='\n')
if '--layouts-only' not in sys.argv:(out/('production-source-compiles.json' if production else 'native-source-compiles.json')).write_text(json.dumps(results,indent=2),newline='\n')
