"""Generate the production Visual Studio solution, including distinct C/C++ object names."""
from pathlib import Path
import subprocess
import sys

root = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(root / 'utils/tests'))
from object_names_migration import project

build = root / 'build'
build.mkdir(exist_ok=True)
sdl_root = Path('D:/VC_libs')
for required in [sdl_root / 'include/SDL2/SDL.h', sdl_root / 'lib/SDL2.lib']:
    if not required.is_file():
        raise FileNotFoundError(required)
wrapper = build / 'generate.lua'
wrapper.write_text('''dofile("@ROOT@/premake5.lua")
workspace "reVC"
    location "@BUILD@"
    toolset "v143"
    filter {}
project "reVC"
    includedirs {"C:/msys/home", "C:/msys/home/github/CITAlloc", "D:/VC_libs/include"}
    links {"SDL2", "winmm", "comdlg32", "ole32", "opengl32"}
    filter "platforms:win-amd64*"
        libdirs {"D:/VC_libs/lib"}
    filter {}
'''.replace('@ROOT@', root.as_posix()).replace('@BUILD@', build.as_posix()), newline='\n')
subprocess.run([str(root / 'premake5.exe'), '--file=' + str(wrapper), 'vs2019',
                '--with-librw', '--no-git-hash'], cwd=root, check=True)
path = build / 'reVC.vcxproj'
path.write_text(project(path.read_text()), newline='\n')
print('Open ' + str(build / 'reVC.sln'))
