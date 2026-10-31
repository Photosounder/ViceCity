"""Generate the production Visual Studio solution, including distinct C/C++ object names."""
from pathlib import Path
import subprocess
import sys

root = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(root / 'utils/tests'))
from object_names_migration import project

build = root / 'build'
build.mkdir(exist_ok=True)
sdl_include = Path('C:/msys/home/github/quakespasm/Windows/SDL2/include')
shim = build / 'include/SDL2'
shim.mkdir(parents=True, exist_ok=True)
for name in ['SDL.h', 'SDL_audio.h', 'SDL_opengl.h', 'SDL_syswm.h', 'SDL_vulkan.h']:
    # Expose the installed flat SDK through the SDL2 include layout used by rouziclib
    header = sdl_include / name
    assert header.is_file(), header
    (shim / name).write_text('#include "' + header.as_posix() + '"\n', newline='\n')
wrapper = build / 'generate.lua'
wrapper.write_text('''dofile("@ROOT@/premake5.lua")
workspace "reVC"
    location "@BUILD@"
    toolset "v143"
    filter {}
project "reVC"
    includedirs {"C:/msys/home", "C:/msys/home/github/CITAlloc", "@BUILD@/include"}
    links {"SDL2", "winmm", "comdlg32", "ole32", "opengl32"}
    filter "platforms:win-amd64*"
        libdirs {"C:/msys/home/github/quakespasm/Windows/SDL2/lib64"}
    filter "platforms:win-x86*"
        libdirs {"C:/msys/home/github/quakespasm/Windows/SDL2/lib"}
    filter {}
'''.replace('@ROOT@', root.as_posix()).replace('@BUILD@', build.as_posix()), newline='\n')
subprocess.run([str(root / 'premake5.exe'), '--file=' + str(wrapper), 'vs2019',
                '--with-librw', '--no-git-hash'], cwd=root, check=True)
path = build / 'reVC.vcxproj'
path.write_text(project(path.read_text()), newline='\n')
print('Open ' + str(build / 'reVC.sln'))
