"""Audit scheduler/zone reports and compare original members with actual C."""
from pathlib import Path
import re
import audio_police_suspect_migration as suspect
from audio_controls_migration import function
import audio_police_reports_migration as reports
import audio_police_state_fixture as state
root=Path(__file__).resolve().parents[2];out=root/'build/audio-police-reports-c-tests'
old=(out/'before/src/audio/PolRadio.cpp').read_text();kernel=(root/'src/audio/AudioPoliceReports.c').read_text()
def normalize(s):
    # Compare every actual statement independently of whitespace and comments
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',s,flags=re.S))
for name,target in reports.METHODS.items():
    actual=function(kernel,'AudioPolice_',target)
    assert normalize(reports.body(function(old,'cAudioManager::',name)))==normalize(actual[actual.index('{'):]),name
for file,transform in [('PolRadio.cpp',reports.radio),('AudioManager.h',reports.header),('AudioManager.cpp',reports.calls),('DMAudio.cpp',reports.facade)]:
    assert normalize(transform((out/'before/src/audio'/file).read_text()))==normalize((root/'src/audio'/file).read_text()),file
expected=reports.calls(function((out/'before/src/audio/DMAudio.cpp').read_text(),'DMAudio_','PlaySuspectLastSeen'))
assert normalize(expected)==normalize(function(kernel,'DMAudio_','PlaySuspectLastSeen'))
common=state.common.replace('void InitialisePoliceRadioZones();','void ServicePoliceRadio();\nbool8 SetupCrimeReport();\nvoid PlaySuspectLastSeen(float,float,float);\nvoid SetupSuspectLastSeenReport();\nvoid InitialisePoliceRadioZones();')
common=common.replace('value(m==&AudioManager);value(m->m_bIsInitialised);value(m->m_FrameCounter);','value(m==&AudioManager);value(m->m_bIsInitialised);value(m->m_FrameCounter);value(m->m_bIsPaused);\nfor(unsigned i=0;i<5;++i) {value(m->m_anRandomTable[i]);}')
common+='''
#include "AudioPoliceGame.h"
#define TRUE 1
using int16=int16_t;
extern "C" void AudioPolice_Service(cAudioManager *);
extern "C" uint8_t AudioPolice_SetupCrimeReport(cAudioManager *);
extern "C" void AudioPolice_PlaySuspect(cAudioManager *,float,float,float);
extern "C" void AudioPolice_ServiceChannel(cAudioManager *,uint8_t);
extern "C" void AudioPolice_SuspectReport(cAudioManager *);
#ifdef FIX_BUGS
extern "C" uint32_t AudioPoliceHost_LogicalFramesPassed(void);
#endif
static uint32_t logicalFrames=1;
static uint8_t replayFlag;
static int16_t zoneId;
static unsigned playerCalls,playerPattern;
struct CVector {float x,y,z;CVector(float a,float b,float c):x(a),y(b),z(c) {}};
class CWanted {
public:
    int32 level;
    int32 GetWantedLevel() {
        // Observe wanted reads only after the original player and replay gates
        value(20);snapshot(active);return level;
    }
};
class CPlayerPed {public:CWanted *m_pWanted;};
static CWanted wanted;
static CPlayerPed player;
static CPlayerPed noWanted;
CPlayerPed *FindPlayerPed() {
    // Preserve repeated lookups and controlled third-lookup disappearance
    value(21);value(playerCalls);snapshot(active);unsigned call=playerCalls++;
    if(playerPattern==0)return NULL;
    if(playerPattern==1)return &noWanted;
    if(playerPattern==3 && call%3==2)return NULL;
    return &player;
}
struct CReplay {
    static bool IsPlayingBack() {
        // Observe the replay gate after crime report generation
        value(22);snapshot(active);return replayFlag!=0;
    }
};
struct CTimer {
#ifdef FIX_BUGS
    static uint32 GetLogicalFramesPassed() {
        // Observe the original unsigned scheduling decrement location
        value(23);snapshot(active);return logicalFrames;
    }
#endif
};
struct FakeMusic {uint8 m_nMusicMode;};
static FakeMusic MusicManager;
class CZone {public:char name[8];float minx,miny,maxx,maxy;};
static CZone zone;
struct CTheZones {
    static int16 FindAudioZone(CVector *v) {
        // Observe the same copied coordinates supplied to the game zone lookup
        value(24);real(v->x);real(v->y);real(v->z);snapshot(active);return zoneId;
    }
    static CZone *GetAudioZone(int16 id) {
        // Return stable field storage for both original and pointer-view reports
        value(25);value(id);snapshot(active);return &zone;
    }
};
void cAudioManager::SetupSuspectLastSeenReport() {
    // Observe scheduling before simulating mutable vehicle-report queue work
    value(26);snapshot(this);PoliceRadioQueue_Add(&m_sPoliceRadioQueue,123);
}
void AudioPolice_SuspectReport(cAudioManager *m) {
    // Supply the same controlled report boundary while the actual C report is verified separately
    m->SetupSuspectLastSeenReport();
}
void AudioPolice_ServiceChannel(cAudioManager *m,uint8_t wantedLevel) {
    // Capture the final narrowed wanted level and complete report state
    value(27);value(wantedLevel);snapshot(m);
}
'''
methods='\n'.join(function(old,'cAudioManager::',name) for name in reports.METHODS)
facade=function((out/'before/src/audio/DMAudio.cpp').read_text(),'DMAudio_','PlaySuspectLastSeen')
bridges='''
extern "C" void AudioPolice_Service(cAudioManager *m) {
    // Reach the original persistent scheduler from the shared C comparison caller
    m->ServicePoliceRadio();
}
extern "C" uint8_t AudioPolice_SetupCrimeReport(cAudioManager *m) {
    // Reach the original crime report from C without changing owner identity
    return m->SetupCrimeReport();
}
extern "C" void AudioPolice_PlaySuspect(cAudioManager *m,float x,float y,float z) {
    // Drive the original coordinate report through the same C ABI
    m->PlaySuspectLastSeen(x,y,z);
}
'''
trace=(root/'utils/tests/audio_police_reports.cpp.in').read_text()
for variant in ('before','after'):
    source=common+(methods+facade+bridges if variant=='before' else suspect.remove_bridge(reports.HOST)+"\n#ifdef FIX_BUGS\n"+function(old,"AudioPoliceHost_","LogicalFramesPassed")+"\n#endif\n")+trace
    source=source.replace('@SERVICE@','m->ServicePoliceRadio()' if variant=='before' else 'AudioPolice_Service(m)').replace('@CRIME@','m->SetupCrimeReport()' if variant=='before' else 'AudioPolice_SetupCrimeReport(m)').replace('@COORD@','m->PlaySuspectLastSeen(x,y,z)' if variant=='before' else 'AudioPolice_PlaySuspect(m,x,y,z)')
    (out/(variant+'.cpp')).write_text(source,newline='\n')
for name,flags in {'default':'#define AUDIO_OAL\n','vanilla':'#define AUDIO_OAL\n#undef FIX_BUGS\n','no-external':'#undef EXTERNAL_3D_SOUND\n','ps2':'#undef GTA_PC\n#undef AUDIO_REVERB\n#undef EXTERNAL_3D_SOUND\n#define GTA_PS2\n'}.items():
    (out/(name+'.h')).write_text('#include "config.h"\n'+flags,newline='\n')
(out/'caller.c').write_text('''#include "AudioManager.h"
#include "AudioPoliceGame.h"
#include "DMAudio.h"
#include <assert.h>
#include <stddef.h>
_Static_assert(sizeof(AudioPoliceZoneView)==5*sizeof(void *),"Pointer-only zone view");
void TestPoliceReportsC(void) {
    // Reach all actual report operations and the public coordinate facade from C
    AudioManager.m_bIsInitialised=0;
    AudioPolice_Service(&AudioManager);
    AudioPolice_PlaySuspect(&AudioManager,1,2,3);
    DMAudio_PlaySuspectLastSeen(1,2,3);
    assert(AudioPolice_SetupCrimeReport(&AudioManager)==0);
}
''',newline='\n')
print('Three report bodies, public facade, eight game adapters, and all caller/header edits audited')
