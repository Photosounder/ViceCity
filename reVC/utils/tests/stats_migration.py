"""Relocate the complete statistics owner, packed save format and conditional display rows into C11."""
import re
import object_names_migration as objects
from events_migration import arguments
OBSERVER=None

def scalar(source):
    # Preserve all original widths and remove only native shorthand types
    for old,new in [('int32','int32_t'),('uint32','uint32_t'),('uint8','uint8_t'),('wchar','uint16_t'),('Const','const'),('nil','NULL')]:source=re.sub(r'\b'+old+r'\b',new,source)
    return source

def fields(header):
    # Capture the complete canonical scalar and array declarations without replacing shared radio ownership
    values=re.findall(r'static\s+(int32|float|char|bool)\s+(\w+)(\[[^\]]+\])?\s*;',header)
    assert len(values)==91,len(values)
    return [(scalar(kind),name,array) for kind,name,array in values]

def signatures(source,header):
    # Recover named arguments from every complete operation and both original inline checkpoint operations
    values=re.findall(r'(?m)^(void|int32|wchar \*|float|int)\s*CStats::(\w+)\(([^\n)]*)\)',source)
    values=[(scalar(kind),name,scalar(args) if args else 'void') for kind,name,args in values]
    for name in ['CheckPointReachedSuccessfully','CheckPointReachedUnsuccessfully']:
        assert re.search(r'static void '+name+r'\(\)\s*\{[^}]*\}',header)
        values.append(('void',name,'void'))
    assert len(values)==38 and len({v[1] for v in values})==38,len(values)
    return values

def constants(header):
    # Preserve the complete original four-entry enum and its explicit values
    match=re.search(r'\tenum \{[\s\S]*?\n\t\};',header);assert match
    return re.sub(r'\b(TOTAL_\w+)\b',r'Stats_\1',match[0]).replace('\t','')

def names(source,header,ops):
    # Project only complete original member names and enum constants into their C namespace
    source=source.replace('CStats::','Stats_')
    for name in [v[1] for v in fields(header)]+ops+re.findall(r'\bTOTAL_\w+\b',constants(header).replace('Stats_','')):
        source=re.sub(r'(?<![\w])'+name+r'\b','Stats_'+name,source)
    return source

def header(original,source):
    # Declare every original canonical object and complete operation through a C-compatible public header
    ops=signatures(source,original)
    return '//+ rouz edit (ChatGPT)\n#pragma once\n#include <stdint.h>\n#include <stdbool.h>\n#include "PedType.h"\n#include "audio_enums.h"\n#include "MusicSavedData.h"\n'+constants(original)+'\n#ifdef __cplusplus\nextern "C" {\n#endif\n'+''.join('extern '+kind+' Stats_'+name+re.sub(r'\b(TOTAL_\w+)\b',r'Stats_\1',array)+';\n' for kind,name,array in fields(original))+''.join(kind+' Stats_'+name+'('+args+');\n' for kind,name,args in ops)+'#ifdef __cplusplus\n}\n#endif\n//- rouz edit (ChatGPT)\n'

LEAVES=[
('int32_t','Money','void','return CWorld::Players[CWorld::PlayerInFocus].m_nMoney;'),
('int32_t','VisibleMoney','void','return CWorld::Players[CWorld::PlayerInFocus].m_nVisibleMoney;'),
('bool','HasCheated','void','return CPad::bHasPlayerCheated;'),
('bool','NastyGame','void','return CGame::nastyGame;'),
('CPlayerInfo *','PlayerInfo','void','return &CWorld::Players[CWorld::PlayerInFocus];'),
('int32_t','CollectedPackages','CPlayerInfo *player','return player->m_nCollectedPackages;'),
('int32_t','TotalPackages','CPlayerInfo *player','return player->m_nTotalPackages;'),
('int32_t','Language','void','return FrontEndMenuManager.m_PrefsLanguage;'),
('bool','Japanese','void','#ifdef MORE_LANGUAGES\n    return CFont::IsJapanese();\n#else\n    return false;\n#endif'),
('char *','String2','void','return gString2;'),('uint16_t *','Unicode','void','return gUString;'),('uint16_t *','Unicode2','void','return gUString2;'),
('void','UnicodeStrcpy','uint16_t *target,const uint16_t *source','UnicodeStrcpy(target,source);'),
('void','AsciiToUnicode','const char *source,uint16_t *target','AsciiToUnicode(source,target);'),
('void','Filter','uint16_t *text','CFont::FilterOutTokensFromString(text);'),
('void','AssertFailed','const char *expression,const char *file,unsigned line,const char *function','re3_assert(expression,file,line,function);')]
BACKEND='//+ rouz edit (ChatGPT)\n#pragma once\n#include "Stats.h"\n#ifdef __cplusplus\nclass CPlayerInfo;\nextern "C" {\n#else\ntypedef struct CPlayerInfo CPlayerInfo;\n#endif\n'+''.join(kind+' Stats_Backend'+name+'('+args+');\n' for kind,name,args,code in LEAVES)+'#ifdef __cplusplus\n}\n#endif\n//- rouz edit (ChatGPT)\n'

def host():
    # Retain only native field, string and font accesses outside the complete C statistics policies
    return '//+ rouz edit (ChatGPT)\n#include "common.h"\n#include "Stats_backend.h"\n#include "World.h"\n#include "PlayerInfo.h"\n#include "Pad.h"\n#include "Game.h"\n#include "main.h"\n#include "Font.h"\n#include "Frontend.h"\n'+ '\n'.join(kind+' Stats_Backend'+name+'('+args+')\n{\n    // Access the original typed native primitive at its original C policy access point\n    '+code+'\n}\n' for kind,name,args,code in LEAVES)+'//- rouz edit (ChatGPT)\n'

def operation(source,name):
    # Match the entire exact operation including signatures with an opening brace on the same line
    match=re.search(r'(?m)^(?:void|int32|wchar \*|float|int)\s*CStats::'+name+r'\(',source);assert match,name
    end,args=arguments(source,source.index('(',match.start()));opening=source.index('{',end)
    return source[opening:source.index('\n}',opening)+2]

def kernel(source,oldheader):
    # Preserve every complete original statement, save-buffer copy and display-row macro
    ops=signatures(source,oldheader);chunks=[]
    for kind,name,args in ops:
        if name.startswith('CheckPointReached'):
            block='{\n'+re.search(r'static void '+name+r'\(\)\s*\{([^}]*)\}',oldheader)[1].strip()+'\n}'
        else:block=operation(source,name)
        text=kind+' Stats_'+name+'('+args+')\n'+block
        text=names(text,oldheader,[v[1] for v in ops])
        text=text.replace('CWorld::Players[CWorld::PlayerInFocus].m_nVisibleMoney','Stats_BackendVisibleMoney()').replace('CWorld::Players[CWorld::PlayerInFocus].m_nMoney','Stats_BackendMoney()')
        text=text.replace('CPad::bHasPlayerCheated','Stats_BackendHasCheated()').replace('CGame::nastyGame','Stats_BackendNastyGame()')
        text=text.replace('CPlayerInfo &player = CWorld::Players[CWorld::PlayerInFocus];','CPlayerInfo *player = Stats_BackendPlayerInfo();').replace('player.m_nCollectedPackages','Stats_BackendCollectedPackages(player)').replace('player.m_nTotalPackages','Stats_BackendTotalPackages(player)')
        text=text.replace('FrontEndMenuManager.m_PrefsLanguage','Stats_BackendLanguage()').replace('CMenuManager::LANGUAGE_','TEXT_LANGUAGE_')
        text=text.replace('CFont::IsJapanese()','Stats_BackendJapanese()').replace('CFont::FilterOutTokensFromString(','Stats_BackendFilter(').replace('UnicodeStrcpy(','Stats_BackendUnicodeStrcpy(').replace('AsciiToUnicode(','Stats_BackendAsciiToUnicode(').replace('Floor(','floorf(')
        for old,new in [('gString2','Stats_BackendString2()'),('gUString2','Stats_BackendUnicode2()'),('gUString','Stats_BackendUnicode()')]:text=re.sub(r'\b'+old+r'\b',new,text)
        text=scalar(text).replace('\n{','\n{\n    // Preserve the complete original '+name+' operation and every ordered external access',1)
        chunks.append(text)
    # Keep the original conversion constants and every original global object's storage type
    conversion=source[source.index('#ifdef USE_PRECISE_MEASUREMENT_CONVERTION'):source.index('\nint32 CStats::SeagullsKilled;')]
    definitions=[]
    for kind,name,array in fields(oldheader):definitions.append(kind+' Stats_'+name+re.sub(r'\b(TOTAL_\w+)\b',r'Stats_\1',array)+';')
    return '''//+ rouz edit (ChatGPT)
#include "Stats_backend.h"
#include "Text.h"
#include "TextLanguages.h"
#include "DMAudio.h"
#include "Timer.h"
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#define ARRAY_SIZE(value) (sizeof(value)/sizeof((value)[0]))
#define Min(a,b) ((a)<(b)?(a):(b))
#define Max(a,b) ((a)>(b)?(a):(b))
#ifdef MASTER
#define assert(expression) (expression)
#else
#define assert(expression) (void)((!!(expression))||(Stats_BackendAssertFailed(#expression,__FILE__,__LINE__,__func__),0))
#endif
'''+conversion+'\n'+'\n'.join(definitions)+'\n\n'+'\n\n'.join(chunks)+'\n//- rouz edit (ChatGPT)\n'

def stats_calls(source):
    # Remove the entire static owner and project all direct data and function consumers
    if 'class CStats\n' in source:
        from pathlib import Path
        root=Path(__file__).resolve().parents[2];folder=root/'.diagnostics/stats-c-tests/before'
        original=(folder/'src/core/Stats.cpp').read_text() if (folder/'src/core/Stats.cpp').exists() else (root/'src/core/Stats.cpp').read_text()
        return header(source,original)
    if 'int32 CStats::SeagullsKilled;' in source:
        if OBSERVER:OBSERVER(source)
        return host()
    if 'CStats::' not in source:return source
    result=source.replace('CStats::','Stats_')
    if source.lstrip().startswith('//+ rouz edit (ChatGPT)') and source.rstrip().endswith('//- rouz edit (ChatGPT)'):return result
    return '//+ rouz edit (ChatGPT)\n'+result.rstrip('\n')+'\n//- rouz edit (ChatGPT)\n'

def stats_project(source):
    # Register a unique C object and its native-access header without introducing paired-object collisions
    for tag,old,new in [('ClCompile','Stats.cpp','StatisticsSystem.c'),('ClInclude','Stats.h','Stats_backend.h')]:
        entry='<'+tag+' Include="..\\src\\core\\'+old+'" />';addition='<'+tag+' Include="..\\src\\core\\'+new+'"'+('>\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>' if tag=='ClCompile' else ' />')
        if '..\\src\\core\\'+new+'"' not in source:
            source=source.replace(entry,entry+'\n    '+addition)
            pattern=r'(<'+tag+r' Include="\.\.\\src\\core\\'+re.escape(old)+r'">\n\s*<Filter>core</Filter>\n\s*</'+tag+'>)';source=re.sub(pattern,lambda match:match[0]+'\n    '+match[0].replace(old,new),source)
    return objects.project(source)

def verify_legacy(source,transform,root):
    # Prove complete relocation of the original statistics producer and its canonical storage
    global OBSERVER
    captured=[];saved=OBSERVER;OBSERVER=captured.append
    try:expected=transform(source)
    finally:OBSERVER=saved
    assert len(captured)==1
    normalize=lambda value:re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*[\s\S]*?\*/','',value))
    assert normalize(expected)==normalize((root/'src/core/Stats.cpp').read_text())
    original=(root/'.diagnostics/stats-c-tests/before/src/core/Stats.h').read_text()
    assert normalize(kernel(captured[0],original))==normalize((root/'src/core/StatisticsSystem.c').read_text())

def verify_dependency(saved,current):
    # Permit only the explicit complete statistics projection through historical dependency guards
    if saved.read_bytes()==current.read_bytes():return
    original=saved.read_text();expected=calls(original);assert expected!=original and expected==current.read_text(),str(current)


def calls(source):
    # Preserve the complete statistics projection before removing both roadblock classes
    import roadblocks_migration as roadblocks
    return roadblocks.calls(stats_calls(source))

def project(source):
    # Preserve previous registrations before adding the uniquely named roadblock C object
    import roadblocks_migration as roadblocks
    return roadblocks.project(stats_project(source))
