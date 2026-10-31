"""Generate an isolated native project without recreating the user's disposable build folder."""
from pathlib import Path
import subprocess,json,sys,xml.etree.ElementTree as ET
import object_names_migration as objects
root=Path(__file__).resolve().parents[2];stage='shot-info' if '--shot-info' in sys.argv else 'weapon-effects' if '--weapon-effects' in sys.argv else 'sprite2d' if '--sprite2d' in sys.argv else 'sprite' if '--sprite' in sys.argv else 'shadow-cameras' if '--shadow-cameras' in sys.argv else 'rw-helper' if '--rw-helper' in sys.argv else 'postfx' if '--postfx' in sys.argv else 'mblur' if '--mblur' in sys.argv else 'texture-pools' if '--texture-pools' in sys.argv else 'lights' if '--lights' in sys.argv else 'clump-read' if '--clump-read' in sys.argv else 'tex-read' if '--tex-read' in sys.argv else 'txd-store' if '--txd-store' in sys.argv else 'memory-heap' if '--memory-heap' in sys.argv else 'memory-mgr' if '--memory-mgr' in sys.argv else 'col-model' if '--col-model' in sys.argv else 'node-name' if '--node-name' in sys.argv else 'anim-plugin' if '--anim-plugin' in sys.argv else 'frame-update' if '--frame-update' in sys.argv else 'bones' if '--bones' in sys.argv else 'anim-manager' if '--anim-manager' in sys.argv else 'anim-group' if '--anim-group' in sys.argv else 'anim-association' if '--anim-association' in sys.argv else 'anim-hierarchy' if '--anim-hierarchy' in sys.argv else 'anim-sequence' if '--anim-sequence' in sys.argv else 'anim-frame' if '--anim-frame' in sys.argv else 'anim-clump' if '--anim-clump' in sys.argv else 'anim-node' if '--anim-node' in sys.argv else 'keyframes' if '--keyframes' in sys.argv else 'quaternion' if '--quaternion' in sys.argv else 'stored-poly' if '--stored-poly' in sys.argv else 'col-point' if '--col-point' in sys.argv else 'col-sphere' if '--col-sphere' in sys.argv else 'col-box' if '--col-box' in sys.argv else 'col-line' if '--col-line' in sys.argv else 'col-triangle' if '--col-triangle' in sys.argv else 'compressed-vector' if '--compressed-vector' in sys.argv else 'rect' if '--rect' in sys.argv else 'stored-car' if '--stored-car' in sys.argv else 'ped-routes' if '--ped-routes' in sys.argv else 'damage' if '--damage' in sys.argv else 'doors' if '--doors' in sys.argv else 'transmission' if '--transmission' in sys.argv else 'input-state' if '--input-state' in sys.argv else 'streaming-info' if '--streaming-info' in sys.argv else 'lists' if '--lists' in sys.argv else 'profile-power' if '--profile-power' in sys.argv else 'autopilot' if '--autopilot' in sys.argv else 'debug-display' if '--debug-display' in sys.argv else 'roadblocks' if '--roadblocks' in sys.argv else 'stats' if '--stats' in sys.argv else 'darkel' if '--darkel' in sys.argv else 'wanted' if '--wanted' in sys.argv else 'events';out=root/f'.diagnostics/{stage}-c-tests';project=out/'project'
wrapper=out/'project.lua'
wrapper.write_text('''dofile("@ROOT@/premake5.lua")
workspace "reVC"
    location "@PROJECT@"
    toolset "v143"
    filter {}
project "reVC"
    targetdir "@PROJECT@/bin/%{cfg.platform}/%{cfg.buildcfg}"
    objdir "@PROJECT@/obj/%{cfg.platform}/%{cfg.buildcfg}"
    includedirs {"C:/msys/home", "C:/msys/home/github/CITAlloc", "@ROOT@/.diagnostics/events-c-tests/native-includes"}
    links {"SDL2", "winmm", "comdlg32", "ole32", "opengl32"}
    filter "platforms:win-amd64*"
        libdirs {"C:/msys/home/github/quakespasm/Windows/SDL2/lib64"}
    filter "platforms:win-x86*"
        libdirs {"C:/msys/home/github/quakespasm/Windows/SDL2/lib"}
    filter {}
project "librw"
    targetdir "@PROJECT@/lib/%{cfg.platform}/%{cfg.buildcfg}"
    objdir "@PROJECT@/librw-obj/%{cfg.platform}/%{cfg.buildcfg}"
'''.replace('@ROOT@',root.as_posix()).replace('@PROJECT@',project.as_posix()).replace('@OUT@',out.as_posix()),newline='\n')
result=subprocess.run([str(root/'premake5.exe'),'--file='+str(wrapper),'vs2019','--with-librw','--no-git-hash'],cwd=root,capture_output=True,text=True)
(out/'project-generation.log').write_text(result.stdout+result.stderr,newline='\n');assert result.returncode==0,result.stdout+result.stderr
path=project/'reVC.vcxproj';assert path.exists();source=objects.project(path.read_text())
if stage in ['shot-info','weapon-effects','sprite2d','sprite','shadow-cameras','rw-helper','postfx','mblur','texture-pools','lights','anim-manager','bones','frame-update','anim-plugin','node-name','col-model','memory-mgr','memory-heap','txd-store','tex-read','clump-read']:
    # Build the complete staged source tree with coherent staged include directories before production application
    import importlib
    migration=importlib.import_module(stage.replace('-','_')+'_migration')
    source=migration.project(source)
    source=source.replace('..\\..\\..\\src','..\\overlay\\src')
path.write_text(source,newline='\n')
ns={'m':'http://schemas.microsoft.com/developer/msbuild/2003'};tree=ET.parse(path);items=[p.get('Include') for p in tree.findall('.//m:ClCompile',ns) if p.get('Include')]
assert sum('GameplayEvents.c' in p for p in items)==1;assert all((path.parent/p.replace('\\','/')).resolve().exists() for p in items)
print('Generated isolated MSVC project with',len(items),'source files, including GameplayEvents.c')
