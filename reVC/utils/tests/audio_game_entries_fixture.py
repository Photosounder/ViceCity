"""Compare crime entry and player vehicle lookup against saved C++ members."""
from pathlib import Path
import re
from audio_controls_migration import function
import audio_game_entries_migration as entries
import audio_police_state_fixture as state
root=Path(__file__).resolve().parents[2];out=root/'build/audio-game-entries-c-tests'
oldlogic=(out/'before/src/audio/AudioLogic.cpp').read_text();oldradio=(out/'before/src/audio/PolRadio.cpp').read_text()
kernel=(root/'src/audio/AudioGameEntries.c').read_text()
def normalize(s):
    # Compare complete statements independently of comments and whitespace
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',s,flags=re.S))
vehicle=function(oldlogic,'cAudioManager::','FindVehicleOfPlayer');crime=function(oldradio,'cAudioManager::','ReportCrime')
for expected,prefix,name in [(entries.vehicle_body(vehicle),'AudioGame_','FindVehicle'),(entries.crime_body(crime),'AudioPolice_','ReportCrime')]:
    actual=function(kernel,prefix,name);assert normalize(expected)==normalize(actual[actual.index('{'):])
for file,transform in [('AudioLogic.cpp',entries.logic),('PolRadio.cpp',entries.radio),('AudioManager.h',entries.header),('AudioPoliceGame.h',entries.game_header),('AudioPoliceSuspect.c',entries.suspect),('DMAudio.cpp',entries.facade),('MusicManager.cpp',entries.calls)]:
    assert normalize(transform((out/'before/src/audio'/file).read_text()))==normalize((root/'src/audio'/file).read_text()),file
facade=function((out/'before/src/audio/DMAudio.cpp').read_text(),'DMAudio_','ReportCrime')
expected=facade.replace('eCrimeType crime','enum eCrimeType crime').replace('pos','position').replace('AudioManager.ReportCrime(crime, *position);','AudioPolice_ReportCrime(&AudioManager, crime, position);')
assert normalize(expected)==normalize(function(kernel,'DMAudio_','ReportCrime'))
common=state.common.replace('void InitialisePoliceRadioZones();','CVehicle *FindVehicleOfPlayer();\nvoid ReportCrime(eCrimeType,const CVector &);\nvoid InitialisePoliceRadioZones();')
common=common.replace('value(m==&AudioManager);value(m->m_bIsInitialised);value(m->m_FrameCounter);','value(m==&AudioManager);value(m->m_bIsInitialised);value(m->m_FrameCounter);')
common+='''
#include "AudioPoliceGame.h"
#define nil NULL
class CVector {public:float x,y,z;CVector(float a,float b,float c):x(a),y(b),z(c) {}};
extern "C" void AudioPolice_ReportCrime(cAudioManager *,enum eCrimeType,const CVector *);
static CVector position(1,2,3);
static uint8_t mutateWanted;
class CWanted {
public:
    int32 level;
    int32 GetWantedLevel() {
        // Observe eligibility before cooldown and mutate coordinates to expose early copying
        value(20);snapshot(active);
        if(mutateWanted) {position.x+=1;position.y-=1;position.z+=0.5f;}
        return level;
    }
};
class CEntity {
public:
    bool vehicle;
    bool IsVehicle() {
        // Observe the original type test before the base-to-derived cast
        value(21);snapshot(active);return vehicle;
    }
};
class PaddingBase {public:virtual ~PaddingBase() {} int pad=7;};
class CVehicle:public PaddingBase,public CEntity {};
class CPlayerPed {public:CWanted *m_pWanted;CEntity *m_attachedTo;};
static CWanted wanted;
static CPlayerPed player;
static CVehicle directVehicle,attachedVehicle;
static CEntity object;
static uint8_t hasPlayer,hasDirect,attachment;
static unsigned lookupCalls;
CVehicle *FindPlayerVehicle() {
    // Observe the first lookup even when the later player result is absent
    value(22);snapshot(active);++lookupCalls;return hasDirect?&directVehicle:NULL;
}
CPlayerPed *FindPlayerPed() {
    // Observe the separate player lookup for both entry operations
    value(23);snapshot(active);++lookupCalls;return hasPlayer?&player:NULL;
}
struct FakeMusic {uint8 m_nMusicMode;};
static FakeMusic MusicManager;
extern "C" uint8_t AudioPoliceHost_MusicMode(void) {
    // Read the same music byte at the original crime gate
    return MusicManager.m_nMusicMode;
}
extern "C" CPlayerPed *AudioPoliceHost_FindPlayer(void) {
    // Drive the same controlled game player lookup from C
    return FindPlayerPed();
}
extern "C" int32_t AudioPoliceHost_WantedLevel(const CPlayerPed *p) {
    // Preserve the original unchecked wanted dereference for defined inputs
    return p->m_pWanted->GetWantedLevel();
}
'''
bridges='''
extern "C" CVehicle *AudioGame_FindVehicle(void) {
    // Drive the original owner-independent lookup through the shared C caller
    return active->FindVehicleOfPlayer();
}
extern "C" void AudioPolice_ReportCrime(cAudioManager *m,enum eCrimeType type,const CVector *p) {
    // Drive the original reference-taking entry through the same opaque pointer ABI
    m->ReportCrime(type,*p);
}
'''
trace=(root/'utils/tests/audio_game_entries.cpp.in').read_text()
for variant in ('before','after'):
    source=common+(vehicle+crime+facade+bridges if variant=='before' else entries.HOST)+trace
    source=source.replace('@LOOKUP@','m->FindVehicleOfPlayer()' if variant=='before' else 'AudioGame_FindVehicle()').replace('@CRIME@','m->ReportCrime(type,position)' if variant=='before' else 'AudioPolice_ReportCrime(m,type,&position)')
    (out/(variant+'.cpp')).write_text(source,newline='\n')
for name,flags in {'default':'#define AUDIO_OAL\n','vanilla':'#define AUDIO_OAL\n#undef FIX_BUGS\n','no-external':'#undef EXTERNAL_3D_SOUND\n','ps2':'#undef GTA_PC\n#undef AUDIO_REVERB\n#undef EXTERNAL_3D_SOUND\n#define GTA_PS2\n'}.items():
    (out/(name+'.h')).write_text('#include "config.h"\n'+flags,newline='\n')
(out/'caller.c').write_text('''#include "AudioManager.h"
#include "DMAudio.h"
#include "AudioPoliceGame.h"
#include <assert.h>
CVehicle *TestGameEntriesC(const CVector *position) {
    // Reach both entries and the public crime facade from actual C
    CVehicle *vehicle=AudioGame_FindVehicle();
    AudioPolice_ReportCrime(&AudioManager,CRIME_HIT_PED,position);
    DMAudio_ReportCrime(CRIME_HIT_COP,position);
    return vehicle;
}
''',newline='\n')
print('Both complete entry bodies, public facade, five adapters, seven full caller/header edits audited')
