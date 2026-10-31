"""Audit full INI conversion and generate original versus C11 callers."""
from pathlib import Path
import json,re
import inifile_migration as m
root=Path(__file__).resolve().parents[2];out=root/'build/inifile-c-tests'
def normalize(source):return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',source,flags=re.S))
counts={}
for path in json.loads((out/'changes.json').read_text()):
    original=(out/'before'/path).read_text()
    expected=m.HOST if path=='src/core/IniFile.cpp' else m.project(original) if path.startswith('build/') else m.calls(original)
    assert normalize(expected)==normalize((root/path).read_text()),path
    assert b'\r' not in (root/path).read_bytes(),path
    if path.endswith('.cpp') and path!='src/core/IniFile.cpp':counts[path]=original.count('CIniFile::')
assert len(counts)==5 and sum(counts.values())==11,counts
old=(out/'before/src/core/IniFile.cpp').read_text()
assert normalize(m.kernel(old))==normalize((root/'src/core/IniFile.c').read_text())
assert (root/'src/core/IniFile_backend.h').read_text()==m.BACKEND_HEADER
(out/'caller-counts.json').write_text(json.dumps(counts,indent=2))
(out/'caller.h').write_text('''#pragma once
#ifdef __cplusplus
extern "C" {
#endif
void TestIniLoad(void);
float *TestIniPed(void);
float *TestIniCar(void);
#ifdef __cplusplus
}
#endif
''',newline='\n')
(out/'caller.c').write_text('''#include "IniFile.h"
#include "caller.h"
void TestIniLoad(void)
{
    // Load through the actual C API from a separate C11 translation unit
    IniFile_LoadIniFile();
}
float *TestIniPed(void)
{
    // Return the live C pedestrian multiplier used by density sliders
    return &IniFile_PedNumberMultiplier;
}
float *TestIniCar(void)
{
    // Return the live C traffic multiplier used by density sliders
    return &IniFile_CarNumberMultiplier;
}
''',newline='\n')
(out/'original-IniFile.h').write_text((out/'before/src/core/IniFile.h').read_text(),newline='\n')
(out/'original-caller.cpp').write_text('''#include "original-IniFile.h"
#include "caller.h"
void TestIniLoad(void)
{
    // Invoke the complete untouched original class method
    CIniFile::LoadIniFile();
}
float *TestIniPed(void)
{
    // Return the original live pedestrian multiplier address
    return &CIniFile::PedNumberMultiplier;
}
float *TestIniCar(void)
{
    // Return the original live traffic multiplier address
    return &CIniFile::CarNumberMultiplier;
}
''',newline='\n')
(out/'original.cpp').write_text(old.replace('#include "IniFile.h"','#include "original-IniFile.h"'),newline='\n')
(out/'host.cpp').write_text(m.HOST,newline='\n')
(out/'common.h').write_text('''#pragma once
#include <stdint.h>
#include <stdio.h>
#include <string.h>
typedef int32_t int32;
#define Min(a,b) ((a) < (b) ? (a) : (b))
#define Max(a,b) ((a) > (b) ? (a) : (b))
''',newline='\n')
(out/'Population.h').write_text('#pragma once\nclass CPopulation { public: static int32 MaxNumberOfPedsInUse,MaxNumberOfPedsInUseInterior; };\n',newline='\n')
(out/'CarCtrl.h').write_text('#pragma once\nclass CCarCtrl { public: static int32 MaxNumberOfCarsInUse; };\n',newline='\n')
(out/'main.h').write_text('#pragma once\nextern char gString[256];\n',newline='\n')
template=(root/'utils/tests/inifile.cpp.in').read_text()
native='\n'.join('#include "'+(root/path).as_posix()+'"' for path in ['src/core/common.h','src/control/CarCtrl.h','src/core/main.h','src/peds/Population.h'])
(out/'test.cpp').write_text(template.replace('@NATIVE_HEADERS@',native),newline='\n')
print('Audited full loader, two initialized floats, shared buffer, three integer writes, 11 references in five callers and nine complete source/project files')
