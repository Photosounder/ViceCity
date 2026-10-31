"""Compare extracted global groups and the lazy HUD guard with complete sprite producers."""
from pathlib import Path
import json,re,subprocess,os,ctypes
import sprite2d_lifetimes as life

root=Path(__file__).resolve().parents[2]
out=root/'.diagnostics/sprite2d-c-tests'
probe=out/'runtime'
pattern=r'(?m)^(?:static\s+)?CSprite2d\s+([A-Za-z_]\w*(?:::\w+)*)(\s*\[([^\]\n]+)\])?\s*;'
groups=[]
for name in json.loads((out/'changes.json').read_text()):
    # Extract each actual namespace-scope definition without substituting a hand-picked group list
    source=(out/'before'/name).read_text()
    groups.extend((name,m[0],m[1],m[2] or '') for m in re.finditer(pattern,source))
assert len(groups)==45,len(groups)
font=(out/'before/src/renderer/Font.h').read_text()
button=re.search(r'enum\s*\{\s*BUTTON_NONE\s*=\s*-1,.*?\};',font,re.S)[0]
prefix='#include "owners.h"\n#include <stdlib.h>\n'+button+'\nenum {MAX_FONTS=2,NUM_HUD_SPRITES=69,MAX_NUM_SCRIPT_SRPITES=16};\n'
sdk=(root/'utils/tests/sprite2d_sdk.cpp.in').read_text()
sdk=sdk.replace('need(texture&&texture->alive);event(3,texture->id);', 'need(texture&&texture->alive);need(expectedCount!=0&&expected[--expectedCount]==texture->id);event(3,texture->id);')
sdk='static unsigned expected[1024],expectedCount;\n'+sdk
rows=[]
ctypes.windll.kernel32.SetErrorMode(0x8003)
for phase in ['before','after']:
    # Compile actual generated cleanup registration against the original implicit C++ object lifetimes
    before=phase=='before'
    code=prefix+('#include "original-Sprite2d.h"\n' if before else '#include "Sprite2d.h"\n')+sdk
    code+='static void Report(void)\n{\n    // Observe teardown only after every original global and lazy-local cleanup has run\n    need(expectedCount==0&&reads==destroys);printf("groups=45 reads=%u destroys=%u events=%u hash=%016llx\\n",reads,destroys,events,(unsigned long long)hashValue);\n}\nstatic const int registered=(atexit(Report),0);\n'
    classes={}
    for _,decl,name,extent in groups:
        # Keep static member declarations separate from the actual original definitions
        if '::' in name:
            owner,member=name.split('::');classes.setdefault(owner,[]).append('static '+('CSprite2d' if before else 'Sprite2d')+' '+member+extent+';')
    code+='\n'.join('struct '+owner+' {'+''.join(members)+'};' for owner,members in classes.items())+'\n'
    for _,decl,_,_ in groups:
        # Register cleanup at each extracted definition boundary using the production lifetime projection
        code+=(decl if before else life.global_lifetimes(decl).replace('CSprite2d','Sprite2d'))+'\n'
    lazy='#include "Sprite2d.h"\nstatic CSprite2d *Lazy(void)\n{\n    // Return the same originally guarded HUD local record on every invocation\n    static CSprite2d sprite;return &sprite;\n}\n'
    code+=(lazy.replace('#include "Sprite2d.h"','') if before else life.calls(lazy).replace('CSprite2d','Sprite2d'))
    code+='static void Fill('+('CSprite2d' if before else 'Sprite2d')+' *objects,size_t count)\n{\n    // Leave selected elements empty and record independent reverse resource destruction expectations\n    for(size_t i=0;i<count;++i){need(objects[i].m_pTexture==NULL);if(count>1&&i%7==0)continue;objects[i].m_pTexture=RwTextureRead("lifetime",NULL);expected[expectedCount++]=reads+1000;}\n}\nint main(void)\n{\n    // Populate every extracted group before initializing and revisiting the guarded HUD local\n'
    for _,_,name,extent in groups:
        # Use the actual declared group extent for every array rather than a duplicate runtime size table
        code+='Fill('+ (name+',sizeof('+name+')/sizeof('+name+'[0])' if extent else '&'+name+',1')+');\n'
    code+='auto *local=Lazy();need(local->m_pTexture==NULL);local->m_pTexture=RwTextureRead("lazy",NULL);expected[expectedCount++]=reads+1000;need(Lazy()==local&&Lazy()==local);return 0;\n}\n'
    (probe/f'lifetime-{phase}.cpp').write_text(code,newline='\n')

for bits in [32,64]:
    for opt in ['Od','O2']:
        # Compare actual compiler-managed implicit destruction with the explicit C lifetime projection
        outputs=[]
        for phase in ['before','after']:
            label=f'lifetime-{bits}-{opt}-{phase}'
            existing=f'msvc-{bits}-{opt}-d3d9-{phase}'
            tree=out/('before' if phase=='before' else 'overlay')
            obj=probe/(label+'.obj');exe=probe/(label+'.exe')
            units=['Rect','original-owner','before-caller'] if phase=='before' else ['Rect','after-owner','after-caller','native','peers-native']
            objects=[probe/(existing+'-'+u+'.obj') for u in units]
            assert all(p.exists() for p in objects)
            flags='/nologo /c /MT /EHsc /std:c++14 /fp:precise /'+opt+' /DLIBRW /DRW_D3D9 /D_CRT_SECURE_NO_WARNINGS '+ ' '.join('/I"'+str(p)+'"' for p in [probe]+[p for p in (tree/'src').rglob('*') if p.is_dir()])
            lines=['@echo off',f'call "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Auxiliary/Build/vcvars{bits}.bat" >nul','if errorlevel 1 exit /b 1',f'cl {flags} /Fo"{obj}" "{probe / ("lifetime-"+phase+".cpp")}"','if errorlevel 1 exit /b 1','link /nologo '+' '.join('"'+str(p)+'"' for p in [obj]+objects)+' /OUT:"'+str(exe)+'"','exit /b %errorlevel%']
            driver=probe/(label+'.cmd');driver.write_text('\n'.join(lines)+'\n',newline='\n')
            run=subprocess.run(['cmd.exe','/c',str(driver)],capture_output=True,text=True)
            (probe/(label+'.log')).write_text(run.stdout+run.stderr,newline='\n')
            assert run.returncode==0,(label,run.stdout[-4000:],run.stderr)
            env=dict(os.environ);key=next(k for k in env if k.lower()=='path');env[key]='C:/Windows/System32;C:/Windows'
            run=subprocess.run([str(exe)],capture_output=True,text=True,env=env,timeout=30)
            assert run.returncode==0,(label,run.returncode,run.stdout,run.stderr)
            outputs.append(run.stdout.strip())
        assert outputs[0]==outputs[1],(bits,opt,outputs)
        rows.append({'bits':bits,'optimization':opt,'before':outputs[0],'after':outputs[1],'exit':0})
        print(bits,opt,outputs[0],flush=True)
(out/'global-lifetime-runs.json').write_text(json.dumps(rows,indent=2),newline='\n')
