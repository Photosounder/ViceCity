"""Compare actual original class and new C record layouts with real Direct3D declarations."""
from pathlib import Path
import ctypes,json,os,re,subprocess
import texture_pools_migration as m

root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/texture-pools-c-tests';probe=out/'layouts';probe.mkdir(exist_ok=True);env=dict(os.environ);key=next(k for k in env if k.lower()=='path');env[key]='C:/msys/ucrt64/bin;C:/msys/usr/bin;'+env[key];ctypes.windll.kernel32.SetErrorMode(0x8003);results=[]
original=(out/'before/src/rw/TexturePools.h').read_text().replace('class CPaletteList\n{','class CPaletteList\n{\npublic:')
(probe/'original.h').write_text(original,newline='\n')
for compiler,bits in [('clang',32),('clang',64),('gcc',64)]:
    # Use actual Windows COM declarations on both ABIs and compare every original data member
    target=['--target=i686-w64-windows-gnu','--sysroot=C:/msys/mingw32'] if bits==32 else []
    observations=[]
    for phase in ['original','new-cpp','new-c']:
        # Open only original private palette access for the layout observer without altering any field or method
        cpp=phase!='new-c';source='#include <stddef.h>\n#include <stdint.h>\n#include <stdio.h>\n'
        source+='#include <d3d8.h>\ntypedef uint32_t uint32;typedef int32_t int32;\n#include "original.h"\n' if phase=='original' else '#include "TexturePools.h"\n'
        statements=[]
        for record,fields in [('CTexturePool',m.POOL_FIELDS),('CPaletteList',m.PALETTE_FIELDS)]:
            # Include size, alignment and all offsets rather than inferring ABI equivalence from runtime traces
            statements.append('printf("'+record+' %zu %zu",sizeof('+record+'),(size_t)__alignof('+record+'));')
            statements+=['printf(" '+field+'=%zu",offsetof('+record+','+field+'));' for field in fields];statements.append('puts("");')
        source+='int main(void)\n{\n    // Report exact native layouts without allocating or invoking any pool policy\n    '+'\n    '.join(statements)+'\n    return 0;\n}\n';label=f'{compiler}-{bits}-{phase}';path=probe/(label+('.cpp' if cpp else '.c'));path.write_text(source,newline='\n');exe=probe/(label+'.exe');tool=Path('C:/msys/ucrt64/bin')/('clang++.exe' if compiler=='clang' and cpp else 'g++.exe' if cpp else compiler+'.exe')
        command=[str(tool),*target,'-std=c++14' if cpp else '-std=c11','-static','-static-libgcc',*(['-static-libstdc++'] if cpp else []),'-I'+str(out/'overlay/src/rw'),'-I'+str(out/'overlay/src/core'),str(path),'-o',str(exe)];run=subprocess.run(command,env=env,capture_output=True,text=True);(probe/(label+'.log')).write_text(run.stdout+run.stderr,newline='\n');assert run.returncode==0,(label,run.stderr)
        runEnv=dict(env);runEnv[key]='C:/Windows/System32;C:/Windows';run=subprocess.run([str(exe)],env=runEnv,capture_output=True,text=True,timeout=10);assert run.returncode==0,(label,run.stderr);observations.append(run.stdout)
    assert len(set(observations))==1,(compiler,bits,observations);results.append({'compiler':compiler,'bits':bits,'consumers':['original-cpp','new-cpp','new-c'],'layout':observations[0],'exit':0});print(compiler,bits,observations[0].strip(),flush=True)
(out/'native-layouts.json').write_text(json.dumps(results,indent=2),newline='\n')
