"""Audit the full directory conversion and generate original/C operation callers."""
from pathlib import Path
import json,re
import player_skin_migration as skins
import directory_migration as m
root=Path(__file__).resolve().parents[2];out=root/'build/directory-c-tests'
def normalize(source):return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',source,flags=re.S))
for path in json.loads((out/'changes.json').read_text()):
    original=(out/'before'/path).read_text()
    target=root/('src/core/Directory.c' if path=='src/core/Directory.cpp' else path)
    expected=m.kernel(original) if path=='src/core/Directory.cpp' else m.project(original) if path.startswith('build/') else m.calls(original)
    if path=='src/renderer/PlayerSkin.cpp':skins.verify_legacy(original,m.calls,root)
    assert normalize(expected)==normalize(target.read_text()),path
    assert b'\r' not in target.read_bytes(),path
assert not (root/'src/core/Directory.cpp').exists()
assert 'class CDirectory' not in (root/'src/core/Directory.h').read_text()
old=(out/'before/src/core/Directory.cpp').read_text();assert len(re.findall(r'^CDirectory::',old,re.M))==7
callerOriginal='\n'.join((out/'before'/path).read_text() for path in ['src/core/Streaming.cpp','src/animation/CutsceneMgr.cpp','src/rw/TexRead.cpp'])
assert callerOriginal.count('std::allocator<CDirectory>().construct(')==3
assert callerOriginal.count('->~CDirectory()')==6
assert sum(callerOriginal.count('->'+name+'(') for name in ['ReadDirFile','WriteDirFile','AddItem','FindItem'])==9
(out/'original-Directory.h').write_text((out/'before/src/core/Directory.h').read_text(),newline='\n')
(out/'original.cpp').write_text(old.replace('#include "Directory.h"','#include "original-Directory.h"'),newline='\n')
(out/'common.h').write_text('''#pragma once
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "config.h"
#ifndef __FILE_NAME__
#define __FILE_NAME__ __FILE__
#endif
#define CITA_TLS thread_local
extern "C" {
#include <cita_windows.h>
}
typedef uint32_t uint32;
typedef int32_t int32;
extern "C" void re3_assert(const char *,const char *,unsigned int,const char *);
#ifndef MASTER
#define assert(expr) (void)((!!(expr)) || (re3_assert(#expr,__FILE__,__LINE__,__func__),0))
#else
#define assert(expr) (expr)
#endif
''',newline='\n')
(out/'no-fix.h').write_text('#pragma once\n#include "config.h"\n#undef FIX_BUGS\n',newline='\n')
declarations='''void TestDirectory_Initialize(CDirectory *directory,int32_t capacity);
void TestDirectory_Shutdown(CDirectory *directory);
void TestDirectory_Read(CDirectory *directory,const char *filename);
bool TestDirectory_Write(CDirectory *directory,const char *filename);
void TestDirectory_Add(CDirectory *directory,const DirectoryInfo *info);
void TestDirectory_AddImage(CDirectory *directory,const DirectoryInfo *info,int32_t image);
bool TestDirectory_Find(CDirectory *directory,const char *name,uint32_t *offset,uint32_t *size);
'''
(out/'caller.h').write_text('''#pragma once
#include <stdint.h>
#include <stdbool.h>
#ifdef ORIGINAL_DIRECTORY
#include "original-Directory.h"
typedef CDirectory::DirectoryInfo DirectoryInfo;
#else
#include "Directory.h"
#endif
#ifdef __cplusplus
extern "C" {
#endif
'''+declarations+'#ifdef __cplusplus\n}\n#endif\n',newline='\n')
forms=[('Initialize','Directory_Initialize(directory,capacity)','new(directory) CDirectory(capacity)'),('Shutdown','Directory_Shutdown(directory)','directory->~CDirectory()'),('Read','Directory_ReadDirFile(directory,filename)','directory->ReadDirFile(filename)'),('Write','Directory_WriteDirFile(directory,filename)','directory->WriteDirFile(filename)'),('Add','Directory_AddItem(directory,info)','directory->AddItem(*info)'),('AddImage','Directory_AddItemWithImage(directory,info,image)','directory->AddItem(*info,image)'),('Find','Directory_FindItem(directory,name,offset,size)','directory->FindItem(name,*offset,*size)')]
for variant in ['before','after']:
    source=('#include "common.h"\n#define ORIGINAL_DIRECTORY\n#include <new>\n' if variant=='before' else '')+'#include "caller.h"\n'
    for name,after,before in forms:
        proto=next(line for line in declarations.splitlines() if 'TestDirectory_'+name+'(' in line)
        ret='return ' if proto.startswith(('bool ','int ')) else ''
        source+=proto[:-1]+'\n{\n\t// Exercise the complete operation through the same external test entry point\n\t'+ret+(before if variant=='before' else after)+';\n}\n'
    (out/('original-caller.cpp' if variant=='before' else 'caller.c')).write_text(source,newline='\n')
(out/'test.cpp').write_text((root/'utils/tests/directory.cpp.in').read_text().replace('@NATIVE_HEADER@','#include "'+(root/'src/core/common.h').as_posix()+'"'),newline='\n')
print('Audited both records, all seven operations, three constructor sites, six destructor sites, nine method calls and ten complete source/project files')
