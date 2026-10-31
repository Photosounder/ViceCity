"""Compare complete original/C pool owners through real Direct3D COM vtables without loading D3D."""
from pathlib import Path
import ctypes,json,os,re,subprocess

root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/texture-pools-c-tests';probe=out/'runtime';results=[];env=dict(os.environ);key=next(k for k in env if k.lower()=='path');env[key]='C:/msys/ucrt64/bin;C:/msys/usr/bin;'+env[key];ctypes.windll.kernel32.SetErrorMode(0x8003)
for compiler,bits in [('clang',32),('clang',64),('gcc',64)]:
    # Use the installed real x86 sysroot while keeping compiler and LLVM tools in the requested MSYS paths
    target=['--target=i686-w64-windows-gnu','--sysroot=C:/msys/mingw32'] if bits==32 else []
    for optimization in ['O0','O2']:
        for profile in ['fixed','classic']:
            # Preserve independent original and C producer objects under both actual Windows COM ABIs
            observations=[]
            for phase in ['before','after']:
                tree=out/('before' if phase=='before' else 'overlay');label=f'{compiler}-{bits}-{optimization}-{profile}-{phase}';profileHeader=probe/(label+'-profile.h');profileText='#include "'+(tree/'src/core/config.h').as_posix()+'"\n'
                if profile=='classic':profileText+='#undef FIX_BUGS\n'
                profileHeader.write_text(profileText,newline='\n');objects=[];units=[probe/'fixture.cpp',probe/'original.cpp',probe/'before-caller.cpp'] if phase=='before' else [probe/'fixture.cpp',tree/'src/rw/TexturePools.c',probe/'after-caller.c']
                for path in units:
                    # Keep every owner algorithm complete and instrument only the external allocator boundary
                    cpp=path.suffix=='.cpp';tool=Path('C:/msys/ucrt64/bin')/('clang++.exe' if compiler=='clang' and cpp else 'g++.exe' if cpp else compiler+'.exe');obj=probe/(label+'-'+path.stem+'.o');objects.append(obj)
                    command=[str(tool),*target,'-std=c++14' if cpp else '-std=c11','-'+optimization,'-ffp-contract=off','-include',str(profileHeader),'-I'+str(probe),'-I'+str(tree/'src/rw'),'-I'+str(tree/'src/core'),'-isystem','C:/msys/home/github/CITAlloc']
                    if phase=='before':command+=['-DORIGINAL']
                    command+=['-c',str(path),'-o',str(obj)];run=subprocess.run(command,env=env,capture_output=True,text=True);(probe/(label+'-'+path.stem+'.log')).write_text(run.stdout+run.stderr,newline='\n');assert run.returncode==0,(label,path.name,run.stderr[-5000:])
                    if path.name in ['original.cpp','TexturePools.c']:
                        # Prove the actual whole producer calls CITAlloc rather than an accidentally substituted CRT allocator
                        symbols=subprocess.run(['C:/msys/ucrt64/bin/objdump.exe','-t',str(obj)],env=env,capture_output=True,text=True,check=True).stdout;assert re.search(r'\b_?cita_win_malloc\b',symbols) and re.search(r'\b_?cita_win_free\b',symbols),(label,symbols)
                cxx=Path('C:/msys/ucrt64/bin')/('clang++.exe' if compiler=='clang' else 'g++.exe');exe=probe/(label+'.exe');run=subprocess.run([str(cxx),*target,'-static','-static-libgcc','-static-libstdc++',*[str(p) for p in objects],'-o',str(exe)],env=env,capture_output=True,text=True);(probe/(label+'-link.log')).write_text(run.stdout+run.stderr,newline='\n');assert run.returncode==0,(label,run.stderr[-5000:])
                imports=subprocess.run(['C:/msys/ucrt64/bin/objdump.exe','-p',str(exe)],env=env,capture_output=True,text=True,check=True);names=re.findall(r'DLL Name: (\S+)',imports.stdout);assert names and not any(re.search(r'libgcc|libstdc|libwinpthread|msys-|d3d8',name,re.I) for name in names),names
                runEnv=dict(env);runEnv[key]='C:/Windows/System32;C:/Windows';run=subprocess.run([str(exe)],env=runEnv,capture_output=True,text=True,timeout=30);(probe/(label+'-run.log')).write_text(run.stdout+run.stderr,newline='\n');assert run.returncode==0,(label,run.returncode,run.stderr);observations.append(run.stdout.strip())
            assert observations[0]==observations[1],(compiler,bits,optimization,profile,observations);results.append({'compiler':compiler,'bits':bits,'optimization':optimization,'profile':profile,'before':observations[0],'after':observations[1],'exit':0});print(compiler,bits,optimization,profile,observations[0],flush=True)
assert len(results)==12
(out/'native-runs.json').write_text(json.dumps(results,indent=2),newline='\n')
