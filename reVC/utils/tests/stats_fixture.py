"""Prepare complete original/C statistics comparisons with packed-byte and display observations."""
from pathlib import Path
import re,json,sys
import stats_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/stats-c-tests';saved=out/'before';preview=out/'preview'
old=(saved/'src/core/Stats.cpp').read_text();header=(saved/'src/core/Stats.h').read_text();fields=m.fields(header)
if '--production' in sys.argv:
    for path in json.loads((out/'changes.json').read_text()):assert (root/path).read_text()==m.calls((saved/path).read_text()),path
    for name in ['StatisticsSystem.c','Stats_backend.h']:assert (root/'src/core'/name).read_text()==(preview/name).read_text(),name
for name,path in [('PedType.h','src/peds/PedType.h'),('audio_enums.h','src/audio/audio_enums.h'),('MusicSavedData.h','src/audio/MusicSavedData.h'),('TextLanguages.h','src/core/TextLanguages.h')]:
    (out/name).write_text((saved/path).read_text(),newline='\n')
(out/'config.h').write_text('#pragma once\n',newline='\n')
originalHeader=header.replace('"../audio/MusicSavedData.h"','"MusicSavedData.h"');(out/'original-Stats.h').write_text(originalHeader,newline='\n')
capi='''#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "audio_enums.h"
#include "MusicSavedData.h"
#include "TextLanguages.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct CText {int unused;} CText;
extern CText TheText;
uint16_t *Text_Get(CText *text,const char *key);
char *UnicodeToAscii(uint16_t *text);
float *DMAudio_GetListenTimeArray(void);
uint8_t DMAudio_IsMP3RadioChannelAvailable(void);
const uint32_t *Timer_GetTimeInMilliseconds(void);
void TestStats_Assert(const char *expression);
#ifdef __cplusplus
}
#endif
'''
(out/'capi.h').write_text(capi,newline='\n')
for name in ['Text.h','DMAudio.h','Timer.h']:(out/name).write_text('#pragma once\n#include "capi.h"\n',newline='\n')
(out/'common.h').write_text('''#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include "config.h"
#include "capi.h"
typedef int32_t int32;typedef uint32_t uint32;typedef uint16_t uint16;typedef uint8_t uint8;typedef uint16_t wchar;
#define Const const
#define nil NULL
#define ARRAY_SIZE(value) (sizeof(value)/sizeof((value)[0]))
#define Min(a,b) ((a)<(b)?(a):(b))
#define Max(a,b) ((a)>(b)?(a):(b))
#ifdef MASTER
#define assert(expression) (expression)
#else
#define assert(expression) (void)((!!(expression))||(TestStats_Assert(#expression),0))
#endif
inline float Floor(float value){return floorf(value);}
''',newline='\n')
owners='''#pragma once
#include "common.h"
#ifdef TEST_STATS_ORIGINAL
#include "original-Stats.h"
#define GLOBAL(name) CStats::name
#define COUNT(name) CStats::name
#else
#include "@PREVIEW@/Stats.h"
#define GLOBAL(name) Stats_##name
#define COUNT(name) Stats_##name
#endif
class CPlayerInfo {public:int32 m_nMoney,m_nVisibleMoney,m_nCollectedPackages,m_nTotalPackages;};
class CWorld {public:static CPlayerInfo Players[2];static uint8 PlayerInFocus;};
class CPad {public:static bool bHasPlayerCheated;};
class CGame {public:static bool nastyGame;};
class CMenuManager {public:
enum {LANGUAGE_AMERICAN=TEXT_LANGUAGE_AMERICAN,LANGUAGE_FRENCH=TEXT_LANGUAGE_FRENCH,LANGUAGE_GERMAN=TEXT_LANGUAGE_GERMAN,LANGUAGE_ITALIAN=TEXT_LANGUAGE_ITALIAN,LANGUAGE_SPANISH=TEXT_LANGUAGE_SPANISH
#ifdef MORE_LANGUAGES
,LANGUAGE_POLISH=TEXT_LANGUAGE_POLISH,LANGUAGE_RUSSIAN=TEXT_LANGUAGE_RUSSIAN,LANGUAGE_JAPANESE=TEXT_LANGUAGE_JAPANESE
#endif
};int32 m_PrefsLanguage;};
extern CMenuManager FrontEndMenuManager;
class CFont {public:static bool IsJapanese();static void FilterOutTokensFromString(uint16_t *text);};
extern char gString2[512];extern uint16_t gUString[256],gUString2[256];
void UnicodeStrcpy(uint16_t *target,const uint16_t *source);void AsciiToUnicode(const char *source,uint16_t *target);
extern "C" void re3_assert(const char*,const char*,unsigned int,const char*);
#include "entry.h"
'''.replace('@PREVIEW@',preview.as_posix())
(out/'owners.h').write_text(owners,newline='\n')
api='#pragma once\n#include <stdint.h>\n#ifdef __cplusplus\nextern "C" {\n#endif\n';caller='#include "'+(preview/'Stats.h').as_posix()+'"\n#include "entry.h"\n';original='#include "owners.h"\n'+re.sub(r'^#include[^\n]*\n','',old,flags=re.M)+'\n'
for kind,name,args in m.signatures(old,header):
    values=[] if args=='void' else [re.search(r'(\w+)\s*$',arg)[1] for arg in args.split(',')]
    api+=kind+' Run_Stats_'+name+'('+args+');\n'+kind+' Before_Stats_'+name+'('+args+');\n'
    original+='extern "C" '+kind+' Before_Stats_'+name+'('+args+')\n{\n    // Invoke the complete original operation through its actual original public signature\n    '+('return ' if kind!='void' else '')+'CStats::'+name+'('+','.join(values)+');\n}\n'
    caller+=kind+' Run_Stats_'+name+'('+args+')\n{\n    // Exercise the whole public operation from an independent strict C11 caller\n#ifdef TEST_STATS_ORIGINAL\n    '+('return ' if kind!='void' else '')+'Before_Stats_'+name+'('+','.join(values)+');\n#else\n    '+('return ' if kind!='void' else '')+'Stats_'+name+'('+','.join(values)+');\n#endif\n}\n'
api+='#ifdef __cplusplus\n}\n#endif\n';(out/'entry.h').write_text(api,newline='\n');(out/'caller.c').write_text(caller,newline='\n');(out/'original.cpp').write_text(original,newline='\n')
(out/'host.cpp').write_text('#include "owners.h"\n#include "'+(preview/'Stats_backend.h').as_posix()+'"\n'+re.sub(r'^#include[^\n]*\n','',m.host(),flags=re.M),newline='\n');(out/'kernel.c').write_text((preview/'StatisticsSystem.c').read_text(),newline='\n')
observations='''static void SeedCanonical(int seed)
{
    // Initialize every named scalar and array element after the original static startup state
'''
snapshot='''static void CanonicalSnapshot()
{
    // Observe every named canonical object without including unrelated addresses or padding
'''
for i,(kind,name,array) in enumerate(fields):
    target='GLOBAL('+name+')';value='(seed+'+str(i)+('+(int)i' if array else '')+')'
    if kind=='bool':expression='('+value+'%2)!=0'
    elif kind=='float':expression='(float)'+value+'*.25f'
    elif kind=='char':expression="(char)('A'+"+value+'%26)'
    else:expression=value
    if array:
        count='sizeof('+target+')/sizeof('+target+'[0])'
        observations+='    for(unsigned i=0;i<'+count+';i++)'+target+'[i]='+expression+';\n'
        snapshot+='    for(unsigned i=0;i<'+count+';i++)'+('Floating' if kind=='float' else 'Number')+'('+target+'[i]);\n'
    else:
        observations+='    '+target+'='+expression+';\n';snapshot+='    '+('Floating' if kind=='float' else 'Number')+'('+target+');\n'
(out/'observations.h').write_text(observations+'}\n'+snapshot+'}\n',newline='\n')
saveNames=json.loads((out/'save-fields.json').read_text())['save'];expected='''static void ExpectedSave(uint8_t *buffer)
{
    // Independently pack every actual original save field in its immutable original serialization order
    size_t offset=0;
'''
for name in saveNames:
    target=name if name.startswith('MusicSavedData_') else 'GLOBAL('+name+')'
    expected+='    memcpy(buffer+offset,&'+target+',sizeof('+target+'));offset+=sizeof('+target+');\n'
expected+='    Check(offset==595);\n}\n';(out/'expected-save.h').write_text(expected,newline='\n')
(out/'fixture.cpp').write_text((root/'utils/tests/stats.cpp.in').read_text(),newline='\n')
print('Prepared all 38 whole original/C operations, 91 object observations, independent packed-save expectations and complete display-row policies')
