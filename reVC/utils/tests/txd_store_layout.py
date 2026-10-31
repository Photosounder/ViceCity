"""Compare every original TXD record field against C and C++ consumers on both native ABIs."""
from pathlib import Path
import ctypes
import json
import os
import subprocess

root = Path(__file__).resolve().parents[2]
out = root / '.diagnostics/txd-store-c-tests'
probe = out / 'runtime'
records = {'TxdDef': 'texDict refCount name'}
env = dict(os.environ)
path_key = next(k for k in env if k.lower() == 'path')
ctypes.windll.kernel32.SetErrorMode(0x8003)
results = []
for bits in [32, 64]:
    # Use real Windows pointer widths and the complete immutable original declarations
    observations = []
    for phase in ['before', 'after-cpp', 'after-c']:
        # Check both language consumers without suppressing or changing the custom-width assertion
        tree = out / ('before' if phase == 'before' else 'overlay')
        label = f'layout-{bits}-{phase}'
        suffix = '.c' if phase == 'after-c' else '.cpp'
        source = probe / (label + suffix)
        prefix = '#include "TxdStore.h"\n#include <stddef.h>\n#include <stdio.h>\n' if suffix == '.c' else '#include <rwcore.h>\n#include "TxdStore.h"\n'
        statements = []
        for record, fields in records.items():
            # Include total size, alignment and every field offset in the native observation
            statements.append(f'printf("{record} %zu %zu",sizeof({record}),(size_t)__alignof({record}));')
            statements.extend(f'printf(" {field}=%zu",offsetof({record},{field}));' for field in fields.split())
            statements.append('puts("");')
        source.write_text(prefix + 'int main(void)\n{\n    // Report complete native record layouts without constructing or allocating TXD storage\n    ' + '\n    '.join(statements) + '\n    return 0;\n}\n', newline='\n')
        vcvars = Path('C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Auxiliary/Build') / f'vcvars{bits}.bat'
        exe = probe / (label + '.exe')
        flags = '/std:c11 /TC' if suffix == '.c' else '/std:c++14 /EHsc'
        flags += ' /DLIBRW /DRW_D3D9 /DUSE_D3D9 /DAUDIO_OAL /DNOMINMAX /Zc:sizedDealloc-'
        includes = [tree/'src']+[p for p in (tree/'src').rglob('*') if p.is_dir()]+[root/'vendor/librw',root/'vendor/openal-soft/include',root/'vendor/libsndfile/include',root/'vendor/mpg123/include',Path('C:/msys/home'),Path('C:/msys/home/github/CITAlloc'),root/'.diagnostics/events-c-tests/native-includes']
        command = 'cl /nologo /O2 /Gy /Gw ' + flags + ' ' + ' '.join('/I"' + str(p) + '"' for p in includes)
        command += ' /Fo"' + str(probe / (label + '.obj')) + '" /Fe"' + str(exe) + '" "' + str(source) + '"'
        command += ' /link /OPT:REF /LIBPATH:"C:/msys/home/github/quakespasm/Windows/SDL2/'+('lib64' if bits==64 else 'lib')+'"'
        driver = probe / (label + '.cmd')
        driver.write_text('@echo off\ncall "' + str(vcvars) + '" >nul\nif errorlevel 1 exit /b 1\n' + command + '\nexit /b %errorlevel%\n', newline='\n')
        build = subprocess.run(['cmd.exe', '/c', str(driver)], env=env, capture_output=True, text=True)
        (probe / (label + '.log')).write_text(build.stdout + build.stderr, newline='\n')
        assert build.returncode == 0, (label, build.stdout, build.stderr)
        run_env = dict(env)
        run_env[path_key] = 'C:/Windows/System32;C:/Windows'
        run = subprocess.run([str(exe)], env=run_env, capture_output=True, text=True, timeout=10)
        assert run.returncode == 0, (label, run.stderr)
        observations.append(run.stdout)
    assert len(set(observations)) == 1, (bits, observations)
    results.append({'bits': bits, 'consumers': ['original-cpp', 'new-cpp', 'new-c'], 'layout': observations[0], 'exit': 0})
    print(bits, observations[0].strip(), flush=True)
(out / 'native-layouts.json').write_text(json.dumps(results, indent=2), newline='\n')
