"""Audit complete migration and generate mixed-language callers for real file tests."""
from pathlib import Path
import json,re
import player_skin_migration as skins
import filemgr_migration as m
import tex_list_migration as textures
import ped_data_migration as pedestrian
import weapon_info_migration as weapons
import text_migration as text
import timecycle_migration as cycle
root=Path(__file__).resolve().parents[2];out=root/'build/filemgr-c-tests'
def normalize(source):
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',source,flags=re.S))
paths=json.loads((out/'changes.json').read_text());counts={}
for path in paths:
    original=(out/'before'/path).read_text()
    expected=m.HOST if path=='src/core/FileMgr.cpp' else m.project(original) if path.startswith('build/') else m.calls(original)
    target=root/textures.target('src/core/Directory.c' if path=='src/core/Directory.cpp' else path)
    if path=='src/renderer/PlayerSkin.cpp':skins.verify_legacy(original,m.calls,root)
    if path=='src/peds/PedType.cpp':pedestrian.verify_legacy(original,m.calls,root)
    if path=='src/weapons/WeaponInfo.cpp':weapons.verify_legacy(original,m.calls,root)
    if path=='src/text/Text.cpp':text.verify_legacy(original,m.calls,root)
    if path=='src/renderer/Timecycle.cpp':cycle.verify_legacy(original,m.calls,root)
    assert normalize(expected)==normalize(target.read_text()),path
    assert b'\r' not in target.read_bytes(),path
    if path.endswith('.cpp') and path!='src/core/FileMgr.cpp':counts[path]=len(re.findall(r'\bCFileMgr::',m.masked(original)))
old=(out/'before/src/core/FileMgr.cpp').read_text()
assert normalize(m.kernel(old))==normalize((root/'src/core/FileMgr.c').read_text())
assert (root/'src/core/FileMgr_backend.h').read_text()==m.BACKEND_HEADER
assert len(counts)==46 and all(counts.values())
assert 'CFileMgr' not in (root/'src/core/FileMgr.h').read_text()
(out/'caller-counts.json').write_text(json.dumps(counts,indent=2))
header=m.HEADER
protos=re.findall(r'^(?:void|FileMgrSize|int|size_t|bool|char \*)\s*FileMgr_\w+\([^\n]+;',header,re.M)
assert len(protos)==15
(out/'caller.h').write_text('#pragma once\n#include "FileMgr.h"\n#ifdef __cplusplus\nextern "C" {\n#endif\n'+'\n'.join(s.replace('FileMgr_','TestFileMgr_') for s in protos)+'\n#ifdef __cplusplus\n}\n#endif\n',newline='\n')
body=[];baseline=[]
for proto in protos:
    match=re.match(r'(.*?)\s*(FileMgr_\w+)\((.*)\);',proto);typ,name,args=match.groups()
    params=[] if args=='void' else [re.search(r'(\w+)\s*$',a)[1] for a in args.split(',')]
    ret='' if typ=='void' else 'return '
    signature=proto[:-1].replace('FileMgr_','TestFileMgr_')
    body.append(signature+'\n{\n\t// Call the complete C file operation through a separate C11 translation unit\n\t'+ret+name+'('+', '.join(params)+');\n}\n')
    member=name.replace('FileMgr_','').replace('OpenFileDefault','OpenFile')
    baseline.append(signature+'\n{\n\t// Invoke the original class operation with the same scalar arguments\n\t'+ret+'CFileMgr::'+member+'('+', '.join(params)+');\n}\n')
(out/'caller.c').write_text('#include "caller.h"\n'+''.join(body),newline='\n')
(out/'original-caller.cpp').write_text('#include "common.h"\n#include "original-FileMgr.h"\n#include "caller.h"\n#include <type_traits>\nstatic_assert(std::is_same<FileMgrSize,ssize_t>::value,"Exact original signed size type");\n'+''.join(baseline),newline='\n')
(out/'original-FileMgr.h').write_text((out/'before/src/core/FileMgr.h').read_text(),newline='\n')
(out/'original.cpp').write_text(old.replace('#include "FileMgr.h"','#include "original-FileMgr.h"'),newline='\n')
(out/'host.cpp').write_text((root/'src/core/FileMgr.cpp').read_text(),newline='\n')
common='''#pragma once
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#ifndef _WIN32
#ifndef __FILE_NAME__
#define __FILE_NAME__ __FILE__
#endif
#define CITA_TLS thread_local
extern "C" {
#include <cita_windows.h>
}
#endif
#if defined(_MSC_VER)
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#endif
typedef uint8_t uint8;
#define nil NULL
extern "C" void re3_assert(const char *, const char *,unsigned int,const char *);
#ifndef MASTER
#define assert(expr) (void)((!!(expr)) || (re3_assert(#expr,__FILE__,__LINE__,__func__),0))
#else
#define assert(expr) (expr)
#endif
'''
(out/'common.h').write_text(common,newline='\n')
(out/'crossplatform.h').write_text('''#pragma once
#ifdef _WIN32
#define fcaseopen fopen
#else
char *casepath(const char *,bool);
FILE *_fcaseopen(const char *,const char *);
#define fcaseopen _fcaseopen
#endif
''',newline='\n')
# Preinclude system headers while targeting Windows, then compile the unchanged POSIX branches
(out/'posix.h').write_text('''#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <direct.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#undef _WIN32
#define TEST_POSIX
''',newline='\n')
(out/'no-fix.h').write_text('#pragma once\n#include "config.h"\n#undef FIX_BUGS\n',newline='\n')
(out/'test.cpp').write_text((root/'utils/tests/filemgr.cpp.in').read_text(),newline='\n')
print('Audited complete file-manager C logic, both former inline methods, 46 callers with '+str(sum(counts.values()))+' calls and all 50 source/project snapshots')
