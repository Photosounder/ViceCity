"""Prove every real original/C model field offset under both native pointer widths."""
from pathlib import Path
import json,os,subprocess
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/col-model-c-tests';probe=out/'layouts';probe.mkdir(exist_ok=True);rows=[]
deps=[root/'vendor/librw',root/'vendor/openal-soft/include',root/'vendor/libsndfile/include',root/'vendor/mpg123/include',Path('C:/msys/home'),Path('C:/msys/home/github/CITAlloc'),root/'.diagnostics/events-c-tests/native-includes']
for bits in [32,64]:
    # Check every field of both complete record declarations with the actual native compiler ABI
    vcvars=Path('C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Auxiliary/Build')/f'vcvars{bits}.bat';pointer=bits//8;first=52 if bits==32 else 56
    fields=[('boundingSphere',0),('boundingBox',16),('numSpheres',40),('numBoxes',42),('numTriangles',44),('numLines',46),('level',47),('ownsCollisionVolumes',48)]+[(name,first+i*pointer) for i,name in enumerate(['spheres','lines','boxes','vertices','triangles','trianglePlanes'])]
    for profile,defines in [('default',[]),('compressed',['COMPRESSED_COL_VECTORS']),('vu',['VU_COLLISION'])]:
        for phase,language in [('before','cpp'),('after','cpp'),('after','c')]:
            # Compile actual saved and staged public headers without synthetic SDK record definitions
            tree=out/('before' if phase=='before' else 'overlay');dirs=[tree/'src']+[p for p in (tree/'src').rglob('*') if p.is_dir() and not any(v.startswith('.') for v in p.relative_to(tree/'src').parts)];label=f'{bits}-{profile}-{phase}-{language}';path=probe/(label+'.'+language);isC=language=='c';assertion='_Static_assert' if isC else 'static_assert';alignment='_Alignof' if isC else 'alignof';source=('' if isC else '#include "common.h"\n')+'#include "ColModel.h"\n#include <stddef.h>\n'
            source+=''.join(assertion+'(offsetof(CColModel,'+name+')=='+str(offset)+',"'+name+'");\n' for name,offset in fields)
            source+=assertion+'(sizeof(CColModel)=='+str(76 if bits==32 else 104)+',"Model size");\n'+assertion+'('+alignment+'(CColModel)=='+str(pointer)+',"Model alignment");\n'+assertion+'(sizeof(CColTrianglePlane)=='+str(8 if profile=='vu' else 20)+',"Actual plane size");\n'+assertion+'(sizeof(CompressedVector)=='+str(12 if profile=='default' else 6)+',"Actual vertex size");\n';path.write_text(source,newline='\n')
            flags='/nologo /c /O2 /fp:precise /DLIBRW /DRW_D3D9 /DUSE_D3D9 /DAUDIO_OAL /DNOMINMAX '+('/std:c11 /TC ' if isC else '/std:c++14 /EHsc /Zc:sizedDealloc- ')+' '.join('/D'+d for d in defines)+' '+' '.join('/I"'+str(p)+'"' for p in dirs+deps);driver=probe/(label+'.cmd');driver.write_text('@echo off\ncall "'+str(vcvars)+'" >nul\nif errorlevel 1 exit /b 1\ncl '+flags+' /Fo"'+str(probe/(label+'.obj'))+'" "'+str(path)+'"\nexit /b %errorlevel%\n',newline='\n');run=subprocess.run(['cmd.exe','/c',str(driver)],env=dict(os.environ),capture_output=True,text=True);(probe/(label+'.log')).write_text(run.stdout+run.stderr,newline='\n');assert run.returncode==0,(label,run.stdout[-4000:],run.stderr[-1000:]);rows.append({'bits':bits,'profile':profile,'phase':phase,'language':language,'exit':0});print(label,'passed',flush=True)
(out/'native-layouts.json').write_text(json.dumps(rows,indent=2),newline='\n')
