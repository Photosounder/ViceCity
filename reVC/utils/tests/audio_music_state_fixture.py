import weather_migration as _weather_conversion
"""Audit complete independent music operations and generate original/C runtime comparisons."""
from pathlib import Path
import re
import audio_music_state_migration as m
root=Path(__file__).resolve().parents[2];out=root/'build/audio-music-state-c-tests'
def normalize(source):
    # Apply the explicit verified weather rename before comparing every original statement
    source=_weather_conversion.calls(source)
    # Compare complete statements independently of comments and formatting
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',source,flags=re.S))
old=(out/'before/src/audio/MusicManager.cpp').read_text();oldh=(out/'before/src/audio/MusicManager.h').read_text();current=(root/'src/audio/MusicManager.h').read_text();kernel=(root/'src/audio/MusicState.c').read_text()
for name,transform in [('src/audio/MusicManager.cpp',m.logic),('src/audio/AudioManager.cpp',m.logic),('src/audio/DMAudio.cpp',m.logic),('src/audio/oal/AudioSampleHost.cpp',m.logic),('src/audio/MusicManager.h',m.header),('build/reVC.vcxproj',m.project),('build/reVC.vcxproj.filters',m.project)]:assert normalize(transform((out/'before'/name).read_text()))==normalize((root/name).read_text()),name
assert normalize(m.fields(oldh))==normalize(m.fields(current))
for name in ['cMusicManager']+m.METHODS:
    original=m.function(old,'cMusicManager::',name);actual=m.function(kernel,'MusicState_','InitState' if name=='cMusicManager' else name)
    assert normalize(m.body(original))==normalize(actual[actual.index('{'):]),name
for name,(ret,field) in m.GETTERS.items():
    original=re.search(r'\b'+name+r'\(\)\s*\{([^}]+)\}',oldh).group(1);actual=m.function(kernel,'MusicState_',name)
    assert normalize('{'+original.replace(field,'manager->'+field)+'}')==normalize(actual[actual.index('{'):]),name
track=oldh[oldh.index('class tStreamedSample'):oldh.index('class CVehicle;')].replace('class tStreamedSample\n{\npublic:','typedef struct tStreamedSample {').replace('};','} tStreamedSample;')
actual=current[current.index('typedef struct tStreamedSample'):current.index('#ifdef __cplusplus')]
assert normalize(m.types(track))==normalize(actual)
record=m.fields(oldh)
getter_lines='\n'.join(re.search(r'[^\n]*\b'+name+r'\(\)\s*\{[^}]+\}',oldh).group(0) for name in m.GETTERS)
methods=m.function(old,'cMusicManager::','cMusicManager')+'\n'+'\n'.join(m.function(old,'cMusicManager::',name) for name in m.METHODS)
common='''#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include <math.h>
#include "config.h"
#include "audio_enums.h"
#include "Validate.h"
using uint32=uint32_t;using uint8=uint8_t;using int32=int32_t;using int8=int8_t;using bool8=uint8_t;
#define TRUE 1
#define FALSE 0
@RECORD@
@METHODS@
cMusicManager MusicManager;
extern "C" uint64_t TestAircraftHash(uint64_t,const void*,uint32_t);
extern "C" uint64_t TestMusicC(cMusicManager*,unsigned,uint32_t,uint32_t);
static uint64_t digest=1469598103934665603ULL;
static void value(uint32_t v) {
    // Observe scalar returns without including owner pointer identities
    digest=TestAircraftHash(digest,&v,sizeof(v));
}
static uint64_t operate(cMusicManager *manager,unsigned op,uint32_t a,uint32_t b) {
    // Enter complete original methods or the actual C caller with identical narrowing
    @OPERATE@
}
int main() {
    // Observe the real original or C-backed constructor before any explicit mutations
    digest=TestAircraftHash(digest,&MusicManager,sizeof(MusicManager));
    cMusicManager owners[2];
    unsigned groups=0;
    static const uint32_t tracks[]={0,1,STREAMED_SOUND_RADIO_MP3_PLAYER,STREAMED_SOUND_RADIO_POLICE,TOTAL_STREAMED_SOUNDS-1,TOTAL_STREAMED_SOUNDS,UINT32_MAX,STREAMED_SOUND_MISSION_COMPLETED,STREAMED_SOUND_CUTSCENE_FINALE};
    static const uint32_t positions[]={UINT32_MAX,0,1,0x80000000U,0x7fffffffU,UINT32_MAX-1,256,65536};
    for(unsigned seed=0;seed<1024;seed++) {
        // Seed every live field and retained byte, then establish valid modulo divisors
        for(unsigned owner=0;owner<2;owner++) {
            memset((void*)&owners[owner],(seed+owner)&255,sizeof(owners[owner]));
            for(unsigned i=0;i<TOTAL_STREAMED_SOUNDS;i++)owners[owner].m_aTracks[i].m_nLength=1U+((seed*1103515245U+i*12345U+owner)%UINT32_MAX);
            for(unsigned i=0;i<NUM_RADIOS;i++)owners[owner].aListenTimeArray[i]=((int)((seed+i+owner)%13)-6)*1.25f;
            if(seed%11==0)owners[owner].aListenTimeArray[seed%NUM_RADIOS]=NAN;
            if(seed%17==0)owners[owner].aListenTimeArray[(seed+1)%NUM_RADIOS]=INFINITY;
        }
        for(unsigned pass=0;pass<3;pass++) {
            // Exercise both owners across live gates, boundary tracks, flags and signed arguments
            for(unsigned op=0;op<12;op++) {
                cMusicManager *manager=&owners[(seed+op+pass)%2];
                manager->m_bIsInitialised=(uint8_t)((seed+op+pass)%4==0?0:(seed%2?1:255));
                manager->m_bDisabled=(uint8_t)((seed+pass)%3==0?0:1);
                manager->m_bAnnouncementInProgress=(uint8_t)((seed+op)%3==0?0:255);
                manager->m_nMusicMode=(uint8_t)((seed+pass)%8);
                manager->m_nUpcomingMusicMode=(uint8_t)((seed+op)%8);
                manager->m_nPlayingTrack=tracks[(seed+op+pass)%9];
                uint32_t a=op==1?(seed%17==0?UINT32_MAX:seed%14):tracks[(seed+pass)%9];
                uint32_t b=positions[(seed+op+pass)%8];
                uint64_t result=operate(manager,op,a,b);value((uint32_t)result);
                if(op==10) {
                    // Preserve the mutable alias returned by the listen-time accessor
                    assert((float*)(uintptr_t)result==manager->aListenTimeArray);
                    ((float*)(uintptr_t)result)[seed%NUM_RADIOS]=(float)((int)(seed%23)-11);
                    value(10);
                }
                digest=TestAircraftHash(digest,&owners[0],sizeof(owners[0]));
                digest=TestAircraftHash(digest,&owners[1],sizeof(owners[1]));groups++;
            }
        }
    }
    printf("%u %llu\\n",groups,(unsigned long long)digest);return 0;
}
'''
# Normalize only the pointer-return operation before hashing its scalar representation
common=common.replace('value((uint32_t)result);','if(op!=10)value((uint32_t)result);')
cases='''switch(op) {
case 0: @INIT@;break;
case 1: @SCRIPT@;break;
case 2: @FRONT@;break;
case 3: @STOP@;break;
case 4: @ANNOUNCE@;break;
case 5: @RESET@;break;
case 6: return @INITIALIZED@;
case 7: return @MODE@;
case 8: return @TRACK@;
case 9: return @FAVOURITE@;
case 10: return (uint64_t)(uintptr_t)@LISTEN@;
case 11: return @INTERRUPT@;
}return 0;'''
expressions={'INIT':('MusicState_InitState(manager)','InitOriginal(manager)'), 'SCRIPT':('MusicState_SetRadioChannelByScript(manager,a,(int32_t)b)','manager->SetRadioChannelByScript(a,(int32_t)b)'), 'FRONT':('MusicState_PlayFrontEndTrack(manager,a,(uint8_t)b)','manager->PlayFrontEndTrack(a,(uint8_t)b)'), 'STOP':('MusicState_StopFrontEndTrack(manager)','manager->StopFrontEndTrack()'), 'ANNOUNCE':('MusicState_PlayAnnouncement(manager,a)','manager->PlayAnnouncement(a)'), 'RESET':('MusicState_ResetTimers(manager,a)','manager->ResetTimers(a)'), 'INITIALIZED':('MusicState_IsInitialised(manager)','manager->IsInitialised()'), 'MODE':('MusicState_GetMusicMode(manager)','manager->GetMusicMode()'), 'TRACK':('MusicState_GetCurrentTrack(manager)','manager->GetCurrentTrack()'), 'FAVOURITE':('MusicState_GetFavouriteRadioStation(manager)','manager->GetFavouriteRadioStation()'), 'LISTEN':('MusicState_GetListenTimeArray(manager)','manager->GetListenTimeArray()'), 'INTERRUPT':('MusicState_CheckForMusicInterruptions(manager)','manager->CheckForMusicInterruptions()')}
for variant in ['before','after']:
    selected=cases
    for key,(c,cpp) in expressions.items():selected=selected.replace('@'+key+'@',cpp if variant=='before' else c)
    if variant=='before':
        decl=m.types(track)+'\nstruct cMusicManager {\n'+record+'\ncMusicManager();\n'+getter_lines+'\n'+ '\n'.join(m.function(old,'cMusicManager::',name).split('{')[0].replace('cMusicManager::','').strip()+';' for name in m.METHODS)+'\n};'
        constructor_body=m.body(m.function(old,'cMusicManager::','cMusicManager'))
        impl=methods+'\nstatic void InitOriginal(cMusicManager *manager)\n'+constructor_body
        # Keep the reference initializer independent of the new C initializer
    else:decl='#include "MusicManager.h"';impl=m.CTOR
    source=common.replace('@RECORD@',decl).replace('@METHODS@',impl).replace('@OPERATE@',selected if variant=='before' else 'return TestMusicC(manager,op,a,b);')
    if variant=='before':source=source.replace('MusicState_IsInitialised(manager)','manager->IsInitialised()')
    (out/(variant+'.cpp')).write_text(source,newline='\n')
selected=cases
for key,(c,cpp) in expressions.items():selected=selected.replace('@'+key+'@',c)
(out/'caller.c').write_text('#include "MusicManager.h"\nuint64_t TestMusicC(cMusicManager *manager,unsigned op,uint32_t a,uint32_t b)\n{\n    // Exercise all actual C declarations and the mutable returned array alias\n'+selected+'\n}\n',newline='\n')
(out/'fast_hash.c').write_bytes((root/'utils/tests/audio_aircraft_observer.c').read_bytes())
for shape,flags in {'default':'','vanilla':'#undef FIX_BUGS\n','no-external':'#undef EXTERNAL_3D_SOUND\n','ps2':'#undef GTA_PC\n#undef EXTERNAL_3D_SOUND\n#define GTA_PS2\n'}.items():(out/(shape+'.h')).write_text('#include "config.h"\n'+flags,newline='\n')
# Emit every original/current/C field offset, size and alignment, including the nested track record
entries=[];cent=[]
for typ,name,arrays in re.findall(r'^\s*(\w+)\s+(\w+)(\[[^;]+)?;',record,re.M):
    entries.append('offsetof(cMusicManager,'+name+'),sizeof(((cMusicManager*)0)->'+name+'),alignof(decltype(((cMusicManager*)0)->'+name+')),')
    cent.append('offsetof(cMusicManager,'+name+'),sizeof(((cMusicManager*)0)->'+name+'),_Alignof('+typ+'),')
for variant in ['before','after','c']:
    cpp=variant!='c';selected=out/'before/src/audio/MusicManager.h' if variant=='before' else root/'src/audio/MusicManager.h'
    aliases='using uint32=uint32_t;using uint8=uint8_t;using int32=int32_t;using int8=int8_t;using bool8=uint8_t;\n' if cpp else ''
    source='#include <stdint.h>\n#include <stddef.h>\n#include "config.h"\n#include "Validate.h"\n'+aliases+'#include "'+selected.as_posix()+'"\n'+('extern "C" ' if cpp else '')+'const size_t AudioManagerAbi[]={sizeof(cMusicManager),'+('alignof' if cpp else '_Alignof')+'(cMusicManager),\n'+'\n'.join(entries if cpp else cent)+'\n};\n'+('extern "C" ' if cpp else '')+'const size_t AudioManagerAbiCount=sizeof(AudioManagerAbi)/sizeof(AudioManagerAbi[0]);\n'
    (out/(variant+'-abi.'+('cpp' if cpp else 'c'))).write_text(source,newline='\n')
print('Twelve complete music operations, all stored fields and track layout, original constructor registration, callers and project edits audited')
