"""Compare actual native player record layouts by reading compile-only COFF data sections."""
from pathlib import Path
import json,os,struct,subprocess
from col_model_consumers import player_fields
from col_model_schema import PLAYER_FIELD_SCHEMA
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/col-model-c-tests';probe=out/'parent-layouts';probe.mkdir(exist_ok=True);rows=[]
deps=[root/'vendor/librw',root/'vendor/openal-soft/include',root/'vendor/libsndfile/include',root/'vendor/mpg123/include',Path('C:/msys/home'),Path('C:/msys/home/github/CITAlloc'),root/'.diagnostics/events-c-tests/native-includes']
for bits in [32,64]:
    # Read actual compiler layout constants without linking unused SDK and rouziclib header dependencies
    outputs=[];vcvars=Path('C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Auxiliary/Build')/f'vcvars{bits}.bat'
    for phase in ['before','after']:
        # Preserve conditional parent fields and export only layout constants into a dedicated COFF section
        tree=out/('before' if phase=='before' else 'overlay');dirs=[tree/'src']+[p for p in (tree/'src').rglob('*') if p.is_dir() and not any(v.startswith('.') for v in p.relative_to(tree/'src').parts)]
        source='#include "common.h"\n#include "Lists.h"\n#include "PlayerInfo.h"\n#include <stddef.h>\n#pragma section(".cmly",read)\nextern "C" __declspec(allocate(".cmly")) const unsigned int playerLayout[]={sizeof(CPlayerInfo),alignof(CPlayerInfo),\n';names=[]
        for kind,name,count in player_fields(PLAYER_FIELD_SCHEMA):
            if kind=='directive':source+=name+'\n'
            else:source+=str(len(names))+',offsetof(CPlayerInfo,'+name+'),\n';names.append(name)
        source+='};\n';label=f'{bits}-{phase}';path=probe/(label+'.cpp');path.write_text(source,newline='\n');obj=probe/(label+'.obj');flags='/nologo /c /O2 /std:c++14 /EHsc /Zc:sizedDealloc- /DLIBRW /DRW_D3D9 /DUSE_D3D9 /DAUDIO_OAL /DNOMINMAX '+' '.join('/I"'+str(p)+'"' for p in dirs+deps);driver=probe/(label+'.cmd');driver.write_text('@echo off\ncall "'+str(vcvars)+'" >nul\nif errorlevel 1 exit /b 1\ncl '+flags+' /Fo"'+str(obj)+'" "'+str(path)+'"\nexit /b %errorlevel%\n',newline='\n');run=subprocess.run(['cmd.exe','/c',str(driver)],env=dict(os.environ),capture_output=True,text=True);(probe/(label+'.log')).write_text(run.stdout+run.stderr,newline='\n');assert run.returncode==0,(label,run.stdout[-4000:],run.stderr)
        data=obj.read_bytes();sectionCount=struct.unpack_from('<H',data,2)[0];table=20+struct.unpack_from('<H',data,16)[0];sections=[]
        for i in range(sectionCount):
            # Extract only the compiler-emitted integer array and reject missing or duplicate layout sections
            entry=table+i*40
            if data[entry:entry+8].rstrip(b'\0')==b'.cmly':
                size,start=struct.unpack_from('<II',data,entry+16);sections.append(data[start:start+size])
        assert len(sections)==1 and len(sections[0])%8==0;values=struct.unpack('<'+'I'*(len(sections[0])//4),sections[0]);outputs.append({'size':values[0],'alignment':values[1],'fields':[(names[values[i]],values[i+1]) for i in range(2,len(values),2)]})
    assert outputs[0]==outputs[1],(bits,outputs);rows.append({'bits':bits,'before':outputs[0],'after':outputs[1],'exit':0});print(bits,'size',outputs[0]['size'],'alignment',outputs[0]['alignment'],'fields',len(outputs[0]['fields']),'passed',flush=True)
(out/'native-parent-layouts.json').write_text(json.dumps(rows,indent=2),newline='\n')
